"""Finite independent math/byte/trace oracles, not a target stack or hardware test."""
from pathlib import Path
from fractions import Fraction
import json, math, sys, copy
from common import parse

ROOT=Path(__file__).resolve().parents[1]
MASK=(1<<32)-1

def crc(data):
    # Polynomial long division, different from the target's incremental recurrence.
    rem=0xffff
    for byte in data:
        rem ^= byte<<8
        for _ in range(8):
            rem=rem<<1
            if rem & 0x10000: rem ^= 0x11021
    return rem

def escaped(data):
    return bytes([126])+b''.join(bytes([125,b^32]) if b in (125,126) else bytes([b]) for b in data)+bytes([126])

def checksum(data):
    if len(data)%2: data+=b'\0'
    total=sum(int.from_bytes(data[n:n+2],'big') for n in range(0,len(data),2))
    while total>>16: total=(total&65535)+(total>>16)
    return total^65535

def be(n,size): return n.to_bytes(size,'big')

def presentation(record):
    fields=[(1,bytes([record['verb']])),(2,be(record['request_id'],2))]
    if 'value' in record: fields.append((3,(record['value']&65535).to_bytes(2,'big')))
    if 'unit' in record: fields.append((4,bytes([record['unit']])))
    if 'label' in record: fields.append((5,record['label'].encode('utf-8')))
    return bytes([1])+b''.join(bytes([tag,len(value)])+value for tag,value in fields)

def tlv_result(raw):
    if not raw or raw[0]!=1: return 'schema'
    if len(raw)>80: return 'capacity'
    pos=1;last=0;fields={};sizes={1:1,2:2,3:2,4:1}
    while pos<len(raw):
        if pos+2>len(raw): return 'truncated'
        tag,n=raw[pos:pos+2];pos+=2
        if tag<=last: return 'order'
        if tag not in range(1,6): return 'unknown_tag'
        if pos+n>len(raw): return 'truncated'
        if tag in sizes and n!=sizes[tag]: return 'length'
        value=raw[pos:pos+n];pos+=n;last=tag;fields[tag]=value
        if tag==5:
            if n>16:return 'label_capacity'
            try:value.decode('utf-8','strict')
            except UnicodeDecodeError:return 'utf8'
    if 1 not in fields or 2 not in fields:return 'required'
    if fields[1][0] not in range(1,5):return 'verb'
    if fields[1][0] in (2,3) and (3 not in fields or 4 not in fields):return 'required'
    if 4 in fields and fields[4]!=bytes([1]):return 'unit'
    return 'ok'

def link_result(wire,local,granted):
    if len(wire)<2 or wire[0]!=126 or wire[-1]!=126:return 'delimiter'
    body=bytearray();pending=False
    for b in wire[1:-1]:
        if pending:body.append(b^32);pending=False
        elif b==125:pending=True
        elif b==126:return 'delimiter'
        else:body.append(b)
    if pending:return 'escape'
    if len(body)>47:return 'capacity'
    if len(body)<7:return 'length'
    if crc(body[:-2])!=int.from_bytes(body[-2:],'big'):return 'crc'
    version,kind,src,dst,n=body[:5]
    if version!=1:return 'version'
    if kind not in range(3):return 'kind'
    if n>40 or len(body)!=n+7 or (kind in (1,2) and n):return 'length'
    if dst!=local or src==local:return 'address'
    if kind==0 and not granted:return 'grant'
    return 'ok'

def arp_packet(i,reply=False):
    smac=bytes.fromhex(i['local_mac'] if reply else i['peer_mac'])
    dmac=bytes.fromhex(i['peer_mac']) if reply else bytes(6)
    sip=bytes(i['local_ip'] if reply else i['peer_ip'])
    dip=bytes(i['peer_ip'] if reply else i['local_ip'])
    return be(1,2)+be(0x800,2)+bytes([6,4])+be(2 if reply else 1,2)+smac+sip+dmac+dip

def echo_packet(i,reply):
    data=bytes.fromhex(i['payload_hex'])
    icmp=bytes([0 if reply else 8,0,0,0])+be(i['id'],2)+be(i['sequence'],2)+data
    icmp=icmp[:2]+be(checksum(icmp),2)+icmp[4:]
    src=bytes(i['local_ip'] if reply else i['peer_ip']);dst=bytes(i['peer_ip'] if reply else i['local_ip'])
    ip=bytes([0x45,0])+be(20+len(icmp),2)+bytes([0,0,0x40,0,64,1,0,0])+src+dst
    ip=ip[:10]+be(checksum(ip),2)+ip[12:]
    return ip+icmp

def seq_delta(a,b):
    n=(a-b)&MASK
    return n-(1<<32) if n>=(1<<31) else n

def window_update_allowed(update,wl1,wl2):
    # Serial-number comparisons are valid only in the declared live half-range.
    return seq_delta(update['seq'],wl1)>0 or (update['seq']==wl1 and seq_delta(update['ack'],wl2)>=0)

def derive(i):
    k=i['case_kind']
    if k=='edu_budget':
        session=i['presentation_max']+i['session_header'];q,r=divmod(session,i['fragment_payload'])
        fragments=[i['fragment_payload']]*q+([r] if r else [])
        l4=max(fragments)+i['transport_header'];l3=l4+i['network_header']
        canonical=i['schema_bytes']+len(i['schema1_value_max'])*i['tlv_header']+sum(i['schema1_value_max'])
        canonical_session=canonical+i['session_header'];cq,cr=divmod(canonical_session,i['fragment_payload'])
        return dict(session_bytes=session,fragment_lengths=fragments,canonical_max=canonical,canonical_session_bytes=canonical_session,canonical_fragment_lengths=[i['fragment_payload']]*cq+([cr] if cr else []),transport_max=l4,network_max=l3,generated_link_body=l3+i['link_header']+i['crc_bytes'],generic_link_body=i['link_payload_max']+i['link_header']+i['crc_bytes'],generic_wire_max=2*(i['link_payload_max']+i['link_header']+i['crc_bytes'])+2)
    if k=='grant_timing':
        bits=1+i['data_bits']+i['stop_bits'];duration=math.ceil(i['frame_octets']*bits*1000000/i['baud'])
        return dict(bits_per_octet=bits,frame_us_ceiling=duration,early_release_ok=i['early_release_us']>=i['tc_us'],legal_release_ok=i['release_us']>=i['tc_us'],peer_start_ok=i['peer_us']>=i['release_us']+i['guard_us'],completion_within_limit=duration<=i['reply_completion_us'],termination_ohms=Fraction(i['termination_ohms'],2).numerator,physical_gate=i['evidence_scope']=='measured')
    if k=='link_bytes':
        pre=bytes.fromhex(i['body_before_crc_hex']);body=pre+be(crc(pre),2)
        return dict(crc_hex=f'{crc(pre):04x}',body_hex=body.hex(),wire_hex=escaped(body).hex(),check_vector_hex=f'{crc(bytes.fromhex(i["check_hex"])):04x}',results=[link_result(bytes.fromhex(x['wire_hex']),x['local'],x['granted']) for x in i['cases']])
    if k=='route':
        out=[]
        for p in i['packets']:
            if p['source']==i['router']:out.append({'result':'own_origin'})
            elif p['protocol']!=4 or p['declared_length']!=len(bytes.fromhex(p['payload_hex'])):out.append({'result':'format'})
            elif p['hops']<=1:out.append({'result':'hop'})
            elif str(p['destination']) not in i['routes']:out.append({'result':'route'})
            else:
                r=i['routes'][str(p['destination'])]
                out.append(dict(result='forward',source=p['source'],destination=p['destination'],hops=p['hops']-1,segment=r['segment'],link_source=r['src'],link_destination=r['dst'],payload_hex=p['payload_hex']))
        return dict(decisions=out,borrow_retained=False,copy_survives_poison=bytes.fromhex(i['payload_hex'])!=bytes.fromhex(i['poison_hex']))
    if k=='message_trace':
        payload=bytes.fromhex(i['payload_hex']);parts=[payload[n:n+24] for n in range(0,len(payload),24)]
        assembled=bytearray();next_index=0;acks=[];deliveries=0;last=None
        for index in i['received_indices']:
            if index==next_index:
                assembled.extend(parts[index]);acks.append(index);last=index;next_index+=1
                if next_index==len(parts):deliveries+=1
            elif index==last:acks.append(index)
        retained=sum(len(parts[n]) for n in i['failure_received_indices']);published=retained==len(payload)
        before=retained
        if i['failure_action']=='abort':retained=0
        return dict(fragment_lengths=[len(p) for p in parts],ack_indices=acks,delivered_hex=assembled.hex(),deliveries=deliveries,retransmit_ms=[i['timeout_ms']*n for n in range(1,i['retry_limit']+1)],failed_ms=i['timeout_ms']*(i['retry_limit']+1),probe_ms=i['probe_origin_ms']+i['timeout_ms'],blocked_ms=i['blocked_origin_ms']+i['blocked_limit_ms'],failure_publishes_partial=published,retained_after_failure=retained,failure_storage_trace=[before,retained])
    if k=='session_trace':
        holder=i['initial_holder'];pending=False;last_cp=0;last_payload=None;effects=0;events=[]
        for e in i['events']:
            if e['type']=='data':
                if e['sender']!=holder or pending:events.append('blocked')
                elif e['checkpoint']==last_cp:events.append('duplicate' if e['payload_hex']==last_payload else 'error_changed_duplicate')
                elif e['checkpoint']!=last_cp+1:events.append('error_checkpoint')
                else:last_cp=e['checkpoint'];last_payload=e['payload_hex'];effects+=1;events.append('commit')
            elif e['type']=='token_send':pending=True;events.append('token_pending')
            elif e['type']=='token_complete':holder=e['new_holder'];pending=False;events.append('token_committed')
            elif e['type']=='resume':events.append('resumed' if e['retained'] and e['epoch']==i['epoch'] else 'resume_refused')
        return dict(events=events,effects=effects,last_checkpoint=last_cp,holder=holder,token_pending=pending,header_bytes=1+2+2+2,durable=False)
    if k=='presentation':
        raw=presentation(i['record'])
        return dict(wire_hex=raw.hex(),wire_bytes=len(raw),signed_value=int.from_bytes(bytes.fromhex(i['signed_hex']),'big',signed=True),results=[tlv_result(bytes.fromhex(x)) for x in i['cases_hex']],publication_on_failure=False)
    if k=='application':
        plant=i['plant'];state=i['setpoint'];seen={};effects=0;responses=[]
        for r in i['requests']:
            identity=(r['verb'],r.get('value'),r['request_id'])
            if r['checkpoint'] in seen:responses.append(dict(seen[r['checkpoint']]));continue
            if r['verb']=='GET':result=dict(verb='REPLY',request_id=r['request_id'],value=plant)
            elif -4000<=r['value']<=12500:
                state=r['value'];effects+=1;result=dict(verb='REPLY',request_id=r['request_id'],value=state)
            else:result=dict(verb='ERROR',request_id=r['request_id'],value=state)
            seen[r['checkpoint']]=dict(result);responses.append(result)
        required=set(i['required_physical']);measured={e['name'] for e in i['evidence'] if e['scope']=='measured'}
        return dict(responses=responses,setpoint=state,plant=plant,effects=effects,missing_physical=sorted(required-measured),family_complete=required<=measured)
    if k=='ether_budget':
        ip=i['ip_header']+i['tcp_header']+i['payload'];raw=i['ether_header']+ip;pad=max(0,i['minimum_without_fcs']-raw)
        return dict(ip_bytes=ip,frame_without_pad=raw,pad_bytes=pad,frame_with_fcs=raw+pad+i['fcs'],dma_bytes=i['buffers']*i['buffer_bytes']+i['descriptors']*i['descriptor_bytes'],socket_owns_tcp=False)
    if k=='phy':
        modes={1:'10half',5:'10full',2:'100half',6:'100full'};code=(i['special']>>2)&7;mode=modes.get(code,'unsupported')
        current=bool(i['status_second']&4);done=bool(i['status_second']&32) and bool(i['special']&4096)
        identity=i['id1']==7 and i['id2']&0xfff0==0xc130
        reset_timeout=i['reset_elapsed_ms']>=i['reset_deadline_ms'];mdio_timeout=i['mdio_elapsed_ms']>=i['mdio_deadline_ms']
        negative_ready=[]
        for c in i['negative_cases']:
            negative_ready.append(identity and bool(c['second']&4) and bool(c['second']&32) and bool(c['special']&4096) and modes.get((c['special']>>2)&7,'unsupported').endswith('full') and c['reset_elapsed_ms']<i['reset_deadline_ms'])
        return dict(identity_match=identity,historical_down=not bool(i['status_first']&4),current_link=current,mode=mode,negotiated=done,ready=identity and current and done and mode.endswith('full') and not reset_timeout and not mdio_timeout,mdc_hz_numerator=i['hclk_hz'],mdc_hz_denominator=i['divider'],reset_timeout=reset_timeout,mdio_timeout=mdio_timeout,negative_case_ready=negative_ready)
    if k=='dma':
        views=[]
        for text in i['rx_words_hex']:
            word=int(text,16);length=(word>>16)&0x3fff
            if word>>31:views.append(dict(result='dma_owned',view_bytes=None))
            elif word&((1<<15)|(1<<14)):views.append(dict(result='error',view_bytes=None))
            elif word&0x300!=0x300:views.append(dict(result='partial',view_bytes=None))
            elif not 64<=length<=1518:views.append(dict(result='length',view_bytes=None))
            else:views.append(dict(result='ok',view_bytes=length-4))
        mask=sum(1<<n for n in i['last_tx_bits'])
        control=mask&~(1<<31)
        return dict(rx_views=views,ring_bytes=i['buffers']*i['buffer_bytes']+i['descriptors']*16,last_tx_control_hex=f'{mask:08x}',reuse_needs_rebuild=(int(i['writeback_hex'],16)&control)!=control,normal_descriptor_bytes=4*4,physical_proven=i['evidence_scope']=='measured',reuse_checks=[(int(w,16)&control)!=control for w in i['reuse_words_hex']])
    if k=='arp':
        request=arp_packet(i);reply=arp_packet(i,True)
        return dict(request_hex=request.hex(),reply_hex=reply.hex(),arp_bytes=len(request),learn_sender_match=bytes.fromhex(i['ether_sa'])==bytes.fromhex(i['arp_sha']),cache_valid=[t<i['cache_ms'] for t in i['query_ms']],request_ms=[i['retry_ms']*n for n in range(i['requests'])],failure_ms=i['retry_ms']*i['requests'],pending_after_failure=0,authenticated=False)
    if k=='echo':
        req=echo_packet(i,False);reply=echo_packet(i,True);frame=14+len(reply)
        data=bytes.fromhex(i['odd_hex'])
        return dict(request_hex=req.hex(),reply_hex=reply.hex(),ip_total=len(reply),icmp_checksum_hex=reply[22:24].hex(),ip_checksum_hex=reply[10:12].hex(),pad_bytes=max(0,60-frame),frame_with_fcs=max(60,frame)+4,odd_checksum_hex=f'{checksum(data):04x}')
    if k=='tcp_trace':
        nxt=(i['peer_isn']+1)&MASK;acks=[nxt];accepted=bytearray();state='ESTABLISHED';invalid=0
        for e in i['events']:
            if e['kind']=='ack_unsent':invalid+=int(seq_delta(e['ack'],i['local_isn']+1)>0);continue
            if e['kind']=='fin':
                if e['seq']==nxt:nxt=(nxt+1)&MASK;state='CLOSE-WAIT';acks.append(nxt)
                continue
            data=bytes.fromhex(e['payload_hex']);delta=seq_delta(e['seq'],nxt)
            if delta<=0:
                trim=min(len(data),-delta);tail=data[trim:]
                if e['seq']+trim & MASK==nxt:accepted.extend(tail);nxt=(nxt+len(tail))&MASK
            acks.append(nxt)
        passive=state;fin_end=(i['local_isn']+2)&MASK
        for e in i['local_close']:
            if e['kind']=='fin' and passive=='CLOSE-WAIT':passive='LAST-ACK'
            elif e['kind']=='ack' and passive=='LAST-ACK' and e['ack']==fin_end:passive='CLOSED'
        close=[]
        for e in i['close_cases']:
            if e['state']=='FIN-WAIT-1' and e['peer_fin']:close.append('TIME-WAIT' if e['our_fin_acked'] else 'CLOSING')
            elif e['state']=='FIN-WAIT-2' and e['peer_fin']:close.append('TIME-WAIT')
            elif e['state']=='CLOSING' and e['our_fin_acked']:close.append('TIME-WAIT')
            else:close.append(e['state'])
        wrapped=i['wrapped_window']
        return dict(accepted_hex=accepted.hex(),ack_numbers=acks,rcv_nxt=nxt,after_peer_fin=state,invalid_unsent_ack=invalid,local_fin_end=fin_end,passive_final_state=passive,time_wait_ms=2*i['msl_ms'],reset_results=['abort' if s==nxt else 'challenge' if 0<seq_delta(s,nxt)<i['rx_window'] else 'drop' for s in i['reset_sequences']],window_updates=[window_update_allowed(e,i['wl1'],i['wl2']) for e in i['window_updates']],wrapped_window_updates=[window_update_allowed(e,wrapped['wl1'],wrapped['wl2']) for e in wrapped['updates']],close_states=close,repeated_fin_deadline_ms=i['repeated_fin_ms']+2*i['msl_ms'])
    if k=='rto':
        srtt=None;var=None;snap=[]
        for e in i['samples']:
            if not e['retransmitted']:
                r=Fraction(e['ms'])
                if srtt is None:srtt=r;var=r/2
                else:var=Fraction(3,4)*var+Fraction(1,4)*abs(srtt-r);srtt=Fraction(7,8)*srtt+Fraction(1,8)*r
            rto=max(1000,min(60000,math.ceil(srtt+max(i['granularity_ms'],4*var))))
            snap.append(dict(srtt_ms=int(srtt) if srtt.denominator==1 else float(srtt),variance_ms=int(var) if var.denominator==1 else float(var),rto_ms=rto))
        times=[];now=0;rto=i['initial_rto_ms']
        for _ in range(i['retry_limit']):now+=rto;times.append(now);rto=min(60000,2*rto)
        abort=now+rto
        usable=[];new=[]
        for e in i['flow']:
            flight=seq_delta(e['nxt'],e['una'])
            if flight<0:raise ValueError('Flight is outside the declared live half-range')
            available=max(0,e['window']-flight);usable.append(available)
            new.append(0 if flight else min(available,i['mss'],e['pending']))
        return dict(estimates=snap,retransmit_ms=times,abort_ms=abort,cap_ms=min(60000,i['cap_input_ms']),post_syn_loss_min_ms=max(3000,i['initial_rto_ms']),usable_windows=usable,new_data_bytes=new,probe_ms=times,ambiguous_samples=sum(e['retransmitted'] for e in i['samples']))
    if k=='http_adapter':
        shared=json.loads((ROOT/i['shared_wire_fixture']).read_text(encoding='utf-8'))
        messages=shared['expected']['messages'];lengths=[];segments=[];joined=[]
        for msg in messages:
            raw=msg['header'].encode('ascii')+bytes.fromhex(msg['body_hex']);lengths.append(len(raw))
            parts=[raw[n:n+i['mss']] for n in range(0,len(raw),i['mss'])]
            segments.append([len(p) for p in parts]);joined.append(b''.join(parts).hex())
        required=set(i['required_physical']);measured={e['name'] for e in i['evidence'] if e['scope']=='measured'}
        return dict(response_lengths=lengths,segment_lengths=segments,rejoined_hex=joined,head_body_bytes=len(bytes.fromhex(messages[1]['body_hex'])),missing_physical=sorted(required-measured),family_complete=required<=measured,parser_hours_counted_here=0)
    raise ValueError(k)

def count(value):
    if isinstance(value,dict):return sum(count(v) for v in value.values())
    if isinstance(value,list):return sum(count(v) for v in value)
    return 1

def validate_input(value,key=''):
    booleans={'granted','retained','retransmitted','peer_fin','our_fin_acked'}
    if isinstance(value,dict):
        for k,v in value.items():validate_input(v,k)
    elif isinstance(value,list):
        for v in value:validate_input(v,key)
    elif key in booleans:
        if type(value) is not bool:raise ValueError('Boolean input required: '+key)
    elif type(value) is bool:raise ValueError('Boolean masquerading as integer/string input: '+key)
    elif type(value) is float:raise ValueError('Integer input required: '+key)
    elif type(value) is int:
        if key not in {'value','setpoint','plant'} and value<0:raise ValueError('Negative unsigned/count input: '+key)
        if key in {'seq','peer_isn','local_isn','ack','wl1','wl2','reset_sequences','una','nxt'} and not 0<=value<=MASK:raise ValueError('Sequence outside u32: '+key)
        if key in {'window','rx_window'} and not 0<=value<=65535:raise ValueError('Unscaled window outside u16: '+key)
        if key in {'local_ip','peer_ip'} and not 0<=value<=255:raise ValueError('IPv4 octet outside u8: '+key)
    elif type(value) is not str:raise ValueError('Unsupported input field type: '+key)

def strict_equal(actual,expected,path='expected'):
    if type(actual) is not type(expected):raise ValueError(path+': type mismatch '+type(actual).__name__+'/'+type(expected).__name__)
    if isinstance(actual,dict):
        if actual.keys()!=expected.keys():raise ValueError(path+': key mismatch')
        for k in actual:strict_equal(actual[k],expected[k],path+'.'+k)
    elif isinstance(actual,list):
        if len(actual)!=len(expected):raise ValueError(path+': length mismatch')
        for n,(a,e) in enumerate(zip(actual,expected)):strict_equal(a,e,path+f'[{n}]')
    elif actual!=expected:raise ValueError(path+': value mismatch')

def check_doc(doc):
    validate_input(doc['input']);actual=derive(doc['input']);strict_equal(actual,doc['expected']);return actual

def validate_catalog(modules,projects,documents,actual_files):
    expected={f'PR-T{domain}-{n:02}' for domain in (40,41) for n in range(1,9)}
    fixture_files={f'fixtures/networking/{pid}.json' for pid in expected}
    if set(actual_files)!=fixture_files or len(actual_files)!=len(fixture_files):
        raise ValueError('Recursive networking fixture file set differs from the 16 owned milestones')
    expected_projects={f'curriculum/projects/{pid}.md' for pid in expected}
    if set(projects)!=expected_projects:raise ValueError('Canonical networking project file set differs from owned milestones')
    if set(documents)!=fixture_files:raise ValueError('Fixture document catalog differs from owned milestones')
    if set(modules)!={'T40','T41'}:raise ValueError('Networking module catalog mismatch')
    for domain,meta in modules.items():
        ids=[f'PR-{domain}-{n:02}' for n in range(1,9)]
        if meta.get('id')!=domain or meta.get('projects')!=ids:raise ValueError('Module/project correspondence mismatch: '+domain)
    for pid in sorted(expected):
        domain=pid[3:6];number=pid[-2:];fixture=f'fixtures/networking/{pid}.json'
        meta=projects[f'curriculum/projects/{pid}.md']
        fixtures=[fixture]+(['fixtures/network_http/09_wire.json'] if pid=='PR-T41-08' else [])
        if meta.get('id')!=pid or meta.get('module')!=domain or meta.get('topics')!=[f'{domain}.{number}'] or meta.get('fixtures')!=fixtures:
            raise ValueError('Project/fixture correspondence mismatch: '+pid)
        if documents[fixture].get('id')!=f'FX-NET-{domain}-{number}':raise ValueError('File/fixture ID correspondence mismatch: '+pid)

def catalog_controls(modules,projects,documents,actual_files):
    controls=[]
    changed=copy.deepcopy(documents);changed['fixtures/networking/PR-T40-01.json']['id']='FX-NET-WRONG'
    controls.append((modules,projects,changed,actual_files))
    changed=copy.deepcopy(projects);changed['curriculum/projects/PR-T41-06.md']['fixtures']=['fixtures/networking/PR-T41-05.json']
    controls.append((modules,changed,documents,actual_files))
    controls.append((modules,projects,documents,list(actual_files)+['fixtures/networking/nested/orphan.json']))
    rejected=0
    for args in controls:
        try:validate_catalog(*args)
        except ValueError:rejected+=1
    if rejected!=len(controls):raise ValueError('Catalog corruption controls were incorrectly accepted')
    return rejected

def type_controls(docs):
    controls=[]
    base=copy.deepcopy(docs[1]);base['expected']['early_release_ok']=0;controls.append(base)
    base=copy.deepcopy(docs[1]);base['expected']['bits_per_octet']=True;controls.append(base)
    base=copy.deepcopy(docs[1]);base['input']['data_bits']=True;controls.append(base)
    base=copy.deepcopy(docs[2]);base['input']['cases'][0]['granted']=1;controls.append(base)
    rejected=0
    for doc in controls:
        try:check_doc(doc)
        except (ValueError,TypeError):rejected+=1
    if rejected!=len(controls):raise ValueError('Type-confusion controls were incorrectly accepted')
    return rejected

def main():
    files=sorted((ROOT/'fixtures/networking').rglob('*.json'));failures=[];fields=0;docs=[];documents={};modules={};projects={}
    for path in files:
        try:
            doc=json.loads(path.read_text(encoding='utf-8'));docs.append(doc);documents[path.relative_to(ROOT).as_posix()]=doc;fields+=count(doc['expected']);check_doc(doc)
        except (ValueError,TypeError,KeyError,OverflowError) as error:failures.append(dict(file=path.name,error=str(error)))
    actual_files=[p.relative_to(ROOT).as_posix() for p in (ROOT/'fixtures/networking').rglob('*') if p.is_file()]
    catalogs=0;controls=0
    try:
        for domain in ('T40','T41'):modules[domain]=parse(ROOT/f'curriculum/modules/{domain}.md')[0]
        for path in sorted((ROOT/'curriculum/projects').glob('PR-T4[01]-*.md')):projects[path.relative_to(ROOT).as_posix()]=parse(path)[0]
        validate_catalog(modules,projects,documents,actual_files)
        catalogs=catalog_controls(modules,projects,documents,actual_files)
        if len(docs)==16:controls=type_controls(docs)
    except (ValueError,TypeError,KeyError) as error:failures.append(dict(error=str(error)))
    print(json.dumps(dict(passed=not failures,fixtures=len(files),fields_checked=fields,type_confusion_controls_rejected=controls,catalog_corruption_controls_rejected=catalogs,failures=failures,scope='Actual canonical catalog and finite supplied expectations only; no learner C or physical execution'),ensure_ascii=False))
    if failures:raise SystemExit(1)

if __name__=='__main__':main()
