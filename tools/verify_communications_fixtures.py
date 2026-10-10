"""Independent arithmetic/trace oracles for T38/T39 supplied fixtures.

These small models verify declared fixture properties, not learner C, electrical
compliance, peripheral silicon or a complete protocol implementation. No target
driver or complete solved C implementation is supplied.
"""
from __future__ import annotations
import argparse
from fractions import Fraction
import json
from pathlib import Path
import re
import zlib

ROOT=Path(__file__).resolve().parents[1]
OWNED={'T38','T39'}
META=re.compile(r'\A```json\s*\n(.*?)\n```',re.S)

def require(ok,message):
    if not ok: raise ValueError(message)

def rational(value):
    f=Fraction(value)
    return {'numerator':f.numerator,'denominator':f.denominator}

BOOLEAN_KEYS={'over8','txe','tc','busy','bsy','transmit','lbcl','te','re','cancelled','dma_stopped',
 'address_write_ack','data_ack','arbitration_lost','address_read_ack','scl_releasable','sda_stuck',
 'same_reference','connected','swapped','remote','extended','controller_fd','consumer_holds',
 'phy_link','descriptor_available','frame_valid','next_hop','transport','http'}

def validate_input(kind,value,key='',depth=0):
    require(depth<=8,'input nesting bound')
    if key in {'fdf','brs','esi'}:
        # The FD validator deliberately receives malformed raw flag values.
        require(type(value) in {bool,int},'raw FD flag representation')
        if type(value) is int:require(-255<=value<=255,'raw FD flag bound')
        return
    if key in BOOLEAN_KEYS:
        require(type(value) is bool,'boolean input required: '+key);return
    if isinstance(value,dict):
        require(len(value)<=32 and all(type(k) is str for k in value),'input object bound')
        for k,v in value.items():validate_input(kind,v,k,depth+1)
    elif isinstance(value,list):
        require(len(value)<=96,'input list bound')
        for v in value:validate_input(kind,v,key,depth+1)
    elif value is None:require(key=='release_after','unexpected null input')
    elif type(value) is int:
        require(-(1<<63)<value<(1<<63),'bounded integer input')
        if key in {'baud','clock_hz','hclk_hz','line_rate','cap_pf','iol_ma'}:
            require(value>0,'positive input required: '+key)
        if key in {'address','register','dlc','length','bytes','capacity','rx_capacity','fifo_capacity',
                   'data_words','bytes_after_header','received','sent','rise_ns','stop_bits'}:
            require(value>=0,'nonnegative input required: '+key)
        if key in {'cpol','cpha'}:require(value in [0,1],'sampling-mode bit')
        if key=='status_words':require(0<=value<=0xffffffff,'descriptor status width')
    elif type(value) is str:require(len(value)<=4000,'input string bound')
    else:raise ValueError('invalid input type: '+key)

def strict_equal(left,right):
    if type(left) is not type(right):return False
    if isinstance(left,dict):
        return left.keys()==right.keys() and all(strict_equal(left[k],right[k]) for k in left)
    if isinstance(left,list):
        return len(left)==len(right) and all(strict_equal(a,b) for a,b in zip(left,right))
    return left==right

def crc32_reference(data):
    # Bit recurrence independently checked against the standard-library zlib.
    remainder=0xffffffff
    for octet in data:
        remainder^=octet
        for _ in range(8):
            remainder=(remainder>>1)^(0xedb88320 if remainder&1 else 0)
    value=remainder^0xffffffff
    require(value==zlib.crc32(data),'CRC recurrence disagrees with zlib')
    return value

def arbitration_bits(frame):
    identifier=frame['identifier']; extended=frame['extended']; remote=frame['remote']
    require(0<=identifier<2**(29 if extended else 11),'CAN identifier range')
    # Comparison prefix deliberately includes standard IDE0 from the control
    # field, so standard-remote versus extended is decided by an actual bit.
    if not extended: return f'{identifier:011b}'+str(int(remote))+'0'
    return f'{identifier>>18:011b}'+'11'+f'{identifier&((1<<18)-1):018b}'+str(int(remote))

def recompute(kind,x):
    validate_input(kind,x)
    if kind=='uart':
        clocks=[]
        for c in x['baud_cases']:
            clock=c['clock_hz'];baud=c['baud'];over8=c['over8']
            require(clock>0 and baud>0,'positive UART clock/baud')
            divisor=(2*clock+baud)//(2*baud)
            scale=8 if over8 else 16
            if not scale<=divisor<=scale*4096-1:
                clocks.append({'status':'range'});continue
            brr=(divisor//scale)*16+divisor%scale
            clocks.append({'status':'ok','divisor':divisor,'brr':brr,
                           'baud':rational(Fraction(clock,divisor)),
                           'error_ppm':rational(Fraction((clock-baud*divisor)*1000000,baud*divisor))})
        frames=[]
        for f in x['frames']:
            width=f['data_bits'];value=f['value'];parity=f['parity']
            require(width in [7,8] and 0<=value<2**width and parity in ['none','even','odd'],'UART frame subset')
            data=[(value>>i)&1 for i in range(width)]
            bits=[0]+data
            if parity!='none': bits.append((sum(data)%2)^(parity=='odd'))
            bits.append(1)
            observed=bits.copy()
            for index in f.get('flip_indices',[]): observed[index]^=1
            decoded=sum(bit<<i for i,bit in enumerate(observed[1:1+width]))
            errors=[]
            if observed[0]!=0: errors.append('start')
            if parity!='none' and sum(observed[1:1+width+1])%2!=(parity=='odd'): errors.append('parity')
            if observed[-1]!=1: errors.append('framing')
            frames.append({'bits':bits,'observed':observed,'decoded':decoded,'errors':errors})
        return {'baud_results':clocks,'frame_results':frames,
                'release_safe':[c['tc'] and not c['busy'] for c in x['completion_cases']]}
    if kind=='usart_sync':
        cases=[]
        for c in x['cases']:
            bits=[(c['value']>>i)&1 for i in range(8)]
            clocks=8 if c['lbcl'] else 7
            edges=list(range(1,2*clocks+1)) if c['transmit'] else []
            cases.append({'tx_data_bits':bits,'clock_edges':len(edges),
                          'sample_edges':[e for e in edges if e%2==(0 if c['cpha'] else 1)],
                          'clock_idle':c['cpol'],'receive_clock_available':c['transmit'],
                          'configuration_change_allowed':not c['te'] and not c['re'],
                          'last_bit_clock_present':bool(c['transmit'] and c['lbcl'])})
        return {'cases':cases,'start_stop_clock_edges':0}
    if kind=='i2c_electrical':
        pulls=[]
        for c in x['pullups']:
            # 0.8473 * ohm * pF / 1000 = ns; use integer/rational arithmetic.
            minimum=Fraction(c['vdd_mv']-c['vol_mv'],c['iol_ma'])
            maximum=Fraction(c['rise_ns']*1000*10000,8473*c['cap_pf'])
            low=(minimum.numerator+minimum.denominator-1)//minimum.denominator
            high=maximum.numerator//maximum.denominator
            pulls.append({'minimum_ohm':low,'maximum_ohm':high,'feasible':low<=high,
                          'selected_valid':low<=c['selected_ohm']<=high})
        arbitration=[]
        for c in x['arbitration']:
            streams=c['streams'];require(len({len(s) for s in streams})==1,'equal I2C trace length')
            active=set(range(len(streams)));bus=[];lost=[]
            for bit in range(len(streams[0])):
                state=min(int(streams[i][bit]) for i in active);bus.append(state)
                removed=sorted(i for i in active if int(streams[i][bit])>state)
                lost.append(removed);active-=set(removed)
            arbitration.append({'bus':bus,'lost_by_bit':lost,'survivors':sorted(active)})
        return {'pullups':pulls,'arbitration':arbitration,
                'common_low_ticks':max(x['low_ticks']),'common_high_ticks':min(x['high_ticks'])}
    if kind=='i2c_transaction':
        transactions=[]
        for c in x['transactions']:
            address=c['address']; require(0<=address<128,'7-bit I2C address')
            status='ok';last='terminal_read_nack';bytes_out=[address<<1,c['register'],(address<<1)|1]
            if not c['address_write_ack']:status='address_nack';last='stop'
            elif not c['data_ack']:status='data_nack';last='stop'
            elif c['arbitration_lost']:status='arbitration_lost';last='release_without_stop'
            elif c['stretch_ticks']>c['deadline_ticks']:status='timeout';last='release_and_diagnose'
            elif not c['address_read_ack']:status='address_nack';last='stop'
            transactions.append({'address_and_register_bytes':bytes_out,'status':status,'closing_event':last,
                                 'published_read':c['read_byte'] if status=='ok' else None})
        clears=[]
        for c in x['bus_clear']:
            if not c['scl_releasable']: clears.append({'action':'reset_required','pulses':0})
            elif not c['sda_stuck']: clears.append({'action':'idle','pulses':0})
            elif c['release_after'] is not None and c['release_after']<=9:
                clears.append({'action':'stop_then_recheck','pulses':c['release_after']})
            else: clears.append({'action':'reset_required','pulses':9})
        return {'transactions':transactions,'bus_clear':clears}
    if kind=='spi':
        modes=[]
        for c in x['modes']:
            capture=list(range(2 if c['cpha'] else 1,17,2))
            modes.append({'capture_edges':capture,'clock_levels':[(1-c['cpol']) if e%2 else c['cpol'] for e in range(1,17)],
                          'mosi_bits':[(c['tx']>>i)&1 for i in range(7,-1,-1)],
                          'miso_bits':[(c['rx']>>i)&1 for i in range(7,-1,-1)]})
        endings=[]
        for c in x['endings']:
            status='ok'
            if c['cancelled'] and not c['dma_stopped']:status='cancel_pending'
            elif c['received']<c['sent']:status='undrained_receive'
            elif not c['txe'] or c['bsy']:status='in_flight'
            endings.append({'status':status,'release_cs':status=='ok','reuse_buffer':status=='ok'})
        return {'modes':modes,'endings':endings}
    if kind=='rs232':
        return {'driver_volts':[x['negative_volts'] if bit else x['positive_volts'] for bit in x['logic_bits']],
                'receiver_logic':[1 if v<=-3 else 0 if v>=3 else None for v in x['line_volts']],
                'connections':[{'source':c['source'],'sink':c['sink'],
                                'valid':c['source_direction']=='out' and c['sink_direction']=='in' and c['same_reference']} for c in x['connections']]}
    if kind=='rs422':
        bits=x['bits'];receivers=[]
        for r in x['receivers']:
            if not r['connected']: values=[None]*len(bits)
            else: values=[b^(r['swapped']) for b in bits]
            receivers.append({'name':r['name'],'observed_bits':values})
        return {'receiver_results':receivers,'topologies_valid':[c['drivers']==1 and 1<=c['receivers']<=10 for c in x['topologies']],
                'threshold_results':[1 if v>=200 else 0 if v<=-200 else None for v in x['differential_mv']]}
    if kind=='rs485':
        char_ns=Fraction((1+x['data_bits']+x['stop_bits'])*1000000000,x['baud'])
        end=Fraction(x['assert_ns']+x['enable_ns'])+x['bytes']*char_ns
        disabled=end+x['disable_ns'];turnaround=disabled+x['guard_ns']
        intervals=x['drive_intervals_ns']
        overlaps=[]
        for i,a in enumerate(intervals):
            for j,b in enumerate(intervals[i+1:],i+1):
                if max(a[0],b[0])<min(a[1],b[1]):overlaps.append([i,j])
        load=sum(Fraction(n,d) for n,d in x['unit_loads'])
        accepted=x['incoming_bytes'][:x['rx_capacity']]
        return {'character_ns':rational(char_ns),'final_tc_ns':rational(end),
                'driver_disabled_ns':rational(disabled),'earliest_peer_start_ns':rational(turnaround),
                'release_requests_safe':[Fraction(t)>=end for t in x['release_requests_ns']],
                'overlap_pairs':overlaps,'unit_load':rational(load),'load_valid':load<=32,
                'accepted_bytes':accepted,'overflow_count':len(x['incoming_bytes'])-len(accepted)}
    if kind=='can_classic':
        bits=[arbitration_bits(f) for f in x['frames']]
        # Prefix arbitration comparisons include format bits; examples use equal
        # length per format and standard/extended shared-prefix comparisons.
        winning_bits=min(bits)
        winners=[i for i,b in enumerate(bits) if b==winning_bits]
        c=x['timing'];tq=1+c['bs1']+c['bs2']; prescaler=c['prescaler']
        legal=1<=prescaler<=1024 and 1<=c['bs1']<=16 and 1<=c['bs2']<=8 and 1<=c['sjw']<=min(4,c['bs2'])
        filters=[(identifier&x['mask'])==(x['filter_id']&x['mask']) for identifier in x['filter_inputs']]
        fifo=[];lost=0
        for value in x['fifo_values']:
            if len(fifo)<x['fifo_capacity']:fifo.append(value)
            else:lost+=1
        locked=fifo.copy()
        unlocked=fifo.copy()
        if lost:unlocked[-1]=x['fifo_values'][-1]
        stream=[int(bit) for bit in x['crc_input_bits']]
        dividend=0
        for bit in stream:dividend=(dividend<<1)|bit
        dividend<<=15;polynomial=0xc599
        while dividend.bit_length()>=polynomial.bit_length():
            dividend^=polynomial<<(dividend.bit_length()-polynomial.bit_length())
        return {'arbitration_bits':bits,'winner_indices':winners,'timing_legal':legal,
                'btr_fields':{'brp':prescaler-1,'ts1':c['bs1']-1,'ts2':c['bs2']-1,'sjw':c['sjw']-1},
                'bitrate':rational(Fraction(c['clock_hz'],prescaler*tq)),
                'sample_point':rational(Fraction(1+c['bs1'],tq)),
                'filter_matches':filters,'locked_fifo':locked,'unlocked_fifo':unlocked,'overflow_count':lost,
                'crc15':dividend}
    if kind=='can_fd':
        mapping=list(range(9))+[12,16,20,24,32,48,64]
        cases=[]
        for c in x['cases']:
            status='ok'
            if any(type(c.get(flag)) is not bool for flag in ['fdf','brs','esi']):status='invalid_flag'
            elif not c['fdf']:status='not_fd'
            elif c['remote']:status='unsupported_remote'
            elif not 0<=c['dlc']<=15:status='dlc_range'
            elif c['length']!=mapping[c['dlc']]:status='length_mismatch'
            elif not c['controller_fd']:status='controller_lacks_fd'
            cases.append({'status':status,'crc_width':(17 if c['length']<=16 else 21) if status=='ok' else None,
                          'bitrate_policy':('switched_data' if c['brs'] else 'nominal_only') if status=='ok' else None,
                          'transmitter_error_state':('passive' if c['esi'] else 'active') if status=='ok' else None})
        return {'dlc_lengths':mapping,'cases':cases,
                'power_valid':[6<=c['vsup_mv']/1000<=24 and 3135<=c['vio_mv']<=5250 for c in x['power']],
                'spi_transaction_bytes':[4+4*c['data_words'] for c in x['spi']],
                'spi_word_valid':[c['bytes_after_header']%4==0 for c in x['spi']]}
    if kind=='layers':
        sizes=[]
        n=x['application_bytes']
        for layer in x['wrappers']:
            n+=layer['header_bytes'];sizes.append({'name':layer['name'],'pdu_bytes':n})
        return {'pdu_sizes':sizes,
                'peer_equal':[p['left_protocol']==p['right_protocol'] for p in x['peers']],
                'service_compatible':[s['provided']==s['required'] for s in x['services']]}
    if kind=='ethernet_frame':
        cases=[]
        for c in x['cases']:
            payload=bytes.fromhex(c['payload_hex']); field=c['type_or_length']
            status='ok'
            if len(payload)>1500:status='payload_too_long'
            elif 1501<=field<=1535:status='undefined_gap'
            elif field==0x8100:status='vlan_excluded'
            elif field<=1500 and field!=len(payload):status='length_mismatch'
            if status!='ok':cases.append({'status':status});continue
            padding=bytes(max(0,46-len(payload)))
            header=bytes.fromhex(c['destination']+c['source'])+field.to_bytes(2,'big')
            frame=header+payload+padding;crc=crc32_reference(frame)
            cases.append({'status':'ok','pad_bytes':len(padding),'frame_without_fcs_hex':frame.hex(),
                          'fcs_hex':crc.to_bytes(4,'little').hex(),'frame_with_fcs_bytes':len(frame)+4,
                          'wire_slot_bytes':8+len(frame)+4+12})
        return {'cases':cases,'check_string_crc32':crc32_reference(bytes.fromhex(x['check_hex']))}
    if kind=='mac_bridge':
        addresses=[]
        for h in x['addresses']:
            b=bytes.fromhex(h);require(len(b)==6,'MAC address length')
            addresses.append({'broadcast':b==b'\xff'*6,'group':bool(b[0]&1),'local':bool(b[0]&2)})
        table={};traces=[];ports=set(x['ports'])
        for f in x['frames']:
            now=f['time'];table={a:row for a,row in table.items() if now-row[1]<x['ttl']}
            source=f['source'];destination=f['destination'];ingress=f['ingress']
            require(ingress in ports,'bridge ingress')
            require(not int(source[:2],16)&1,'group source excluded')
            table[source]=(ingress,now)
            if int(destination[:2],16)&1 or destination not in table:
                output=sorted(ports-{ingress});reason='flood'
            elif table[destination][0]==ingress:output=[];reason='filter_same_port'
            else:output=[table[destination][0]];reason='known_unicast'
            traces.append({'output_ports':output,'reason':reason,'entries':len(table)})
        return {'address_classes':addresses,'bridge_trace':traces}
    if kind=='phy':
        modes={1:[10,'half'],5:[10,'full'],2:[100,'half'],6:[100,'full']}
        cases=[]
        for c in x['cases']:
            identity=c['id1']==7 and c['id2']&0xfff0==0xc130
            historical=bool(c['bsr_first']&4);current=bool(c['bsr_second']&4)
            done=bool(c['bsr_second']&32) and bool(c['special']&4096)
            mode=modes.get((c['special']>>2)&7)
            status='ready'
            if not identity:status='wrong_phy'
            elif not current:status='link_down'
            elif not done:status='negotiating'
            elif mode is None:status='unknown_mode'
            elif mode[1]!='full':status='half_duplex_excluded'
            cases.append({'identity_ok':identity,'historical_link':historical,'current_link':current,
                          'mode':mode,'status':status,
                          'mac_dm':status=='ready','mac_fes':status=='ready' and mode[0]==100})
        mdio=[]
        for c in x['mdio']:
            if c['busy']:mdio.append({'status':'busy_no_write'});continue
            settings=[(20000000,35000000,2,16),(35000000,60000000,3,26),
                      (60000000,100000000,0,42),(100000000,150000000,1,62),
                      (150000000,180000001,4,102)]
            selected=next(((cr,divisor) for low,high,cr,divisor in settings if low<=c['hclk_hz']<high),None)
            if selected is None:mdio.append({'status':'clock_range'});continue
            require(0<=c['phy_address']<32 and 0<=c['register']<32,'MDIO address range')
            cr,divisor=selected
            mdio.append({'status':'ok','cr':cr,'mdc_hz':rational(Fraction(c['hclk_hz'],divisor)),
                         'read_start_word':(c['phy_address']<<11)|(c['register']<<6)|(cr<<2)|1})
        return {'cases':cases,'mdio':mdio,'rmii_clock_hz':50000000,'reset_bit_mask':1<<15}
    if kind=='dma_initialization':
        buffers=[]
        for c in x['regions']:
            end=c['address']+c['bytes']
            reachable=any(a<=c['address'] and end<=b for a,b in x['dma_ranges'])
            buffers.append({'reachable':reachable,'aligned':c['address']%4==0,
                            'valid':reachable and c['address']%4==0 and c['bytes']>0})
        orders=[]
        for order in x['orders']:
            positions={event:i for i,event in enumerate(order)}
            violations=[[a,b] for a,b in x['dependencies'] if a not in positions or b not in positions or positions[a]>=positions[b]]
            orders.append({'valid':not violations,'violations':violations})
        return {'buffers':buffers,'orders':orders}
    if kind=='ethernet_rx':
        cases=[]
        for c in x['cases']:
            words=c['status_words'];state='ok';length=None
            if any(w&(1<<31) for w in words):state='dma_owned'
            elif not words[0]&(1<<9) or not words[-1]&(1<<8):state='incomplete'
            elif any(w&((1<<15)|(1<<14)) for w in words):state='receive_error'
            else:
                length=(words[-1]>>16)&0x3fff
                if length<64 or length>min(c['capacity'],1518):state='length_range'
            published=length-4 if state=='ok' else None
            recycle=state!='dma_owned' and not (state=='ok' and c['consumer_holds'])
            cases.append({'status':state,'reported_with_fcs':length,'published_without_fcs':published,'recycle_allowed':recycle})
        return {'cases':cases}
    if kind=='ethernet_tx':
        cases=[]
        for c in x['cases']:
            events=c['events'];state='cpu';error=None;first_owned=False;published=[]
            for event in events:
                if error:break
                if event=='prepare_tail':published.append('tail')
                elif event=='prepare_head':published.append('head')
                elif event=='own_tail':
                    if 'tail' not in published:error='unprepared_tail'
                    else:published.append('tail_owned')
                elif event=='own_head':
                    if 'head' not in published or 'tail_owned' not in published:error='partial_publication'
                    else:state='dma';first_owned=True
                elif event=='reuse':
                    if state=='dma':error='premature_reuse'
                elif event=='complete_ok':
                    if not first_owned:error='completion_without_publish'
                    else:state='complete'
                elif event=='complete_error':
                    if not first_owned:error='completion_without_publish'
                    else:state='failed'
                elif event=='reset_confirmed_stopped':state='aborted'
                else:raise ValueError('unknown TX event')
            cases.append({'status':error or state,'reuse_allowed':not error and state in ['cpu','complete','failed','aborted']})
        return {'cases':cases,'pad_bytes':[max(0,46-n) for n in x['payload_lengths']]}
    if kind=='cross_layer':
        budget=[]
        for c in x['budgets']:
            ip=20+20+c['application_bytes'];frame=14+max(46,ip)+4;slot=8+frame+12
            budget.append({'ip_bytes':ip,'frame_bytes':frame,'wire_slot_bytes':slot,
                           'serialization_ns':rational(Fraction(slot*8*1000000000,c['line_rate']))})
        failed=[]
        for c in x['fault_cases']:
            failed.append(next((name for name in x['contract_order'] if not c[name]),None))
        offsets=[]
        for c in x['captures']:
            converted=[t+c['offset_us'] for t in c['local_us']]
            offsets.append({'common_us':converted,'monotonic':all(a<=b for a,b in zip(converted,converted[1:]))})
        return {'budgets':budget,'first_failed_contract':failed,'captures':offsets}
    raise ValueError('unknown oracle kind: '+kind)

def metadata(path):
    m=META.match(path.read_text(encoding='utf-8-sig'))
    require(m is not None,'missing metadata: '+str(path))
    return json.loads(m[1])

def verify(fixture_root):
    projects={}
    for path in (ROOT/'curriculum/projects').glob('PR-T*.md'):
        m=metadata(path)
        if m['module'] in OWNED:
            require(m['id'] not in projects,'duplicate project ID')
            projects[m['id']]=m
    expected_ids={f'PR-T38-{i:02d}' for i in range(1,11)}|{f'PR-T39-{i:02d}' for i in range(1,9)}
    require(set(projects)==expected_ids,'project set differs from 18 owned milestones')
    for module in OWNED:
        m=metadata(ROOT/'curriculum/modules'/f'{module}.md')
        pids={key for key,p in projects.items() if p['module']==module}
        require(m['id']==module and set(m['projects'])==pids and len(m['projects'])==len(pids),'module/project correspondence')
        require(m['workload']['implementation']+m['workload']['verification']==sum(projects[key]['hours'] for key in pids),'module project-hour budget')
    require(sum(projects[p]['hours'] for p in projects if p.startswith('PR-T38'))==80,'T38 project-hour sum')
    require(sum(projects[p]['hours'] for p in projects if p.startswith('PR-T39'))==66,'T39 project-hour sum')
    listed={}
    for key,m in projects.items():
        require(m['fixtures']==['fixtures/communications/'+key+'.json'],'canonical fixture/project binding')
        listed[key+'.json']=key
    actual={p.relative_to(fixture_root).as_posix():p for p in fixture_root.rglob('*.json')}
    require(set(actual)==set(listed),'fixture file set differs; missing or orphan/nested fixture')
    require(all(p.resolve().is_relative_to(fixture_root.resolve()) for p in actual.values()),'fixture resolves outside selected root')
    passed=0;failures=[]
    for name,key in sorted(listed.items()):
        try:
            f=json.loads(actual[name].read_text(encoding='utf-8-sig'))
            require(f['id']==key,'fixture ID/project mismatch')
            require(strict_equal(f['expected'],recompute(f['kind'],f['input'])),'recomputed expected result or type differs')
            passed+=1
        except (ValueError,KeyError,TypeError,AssertionError) as error:
            failures.append({'fixture':name,'error':str(error)})
    print(json.dumps({'passed':passed,'total':len(listed),'failures':failures},indent=2))
    return not failures

if __name__=='__main__':
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixture-root',type=Path,default=ROOT/'fixtures/communications')
    args=parser.parse_args()
    try:
        okay=verify(args.fixture_root.resolve())
    except (ValueError,KeyError,TypeError) as error:
        print(json.dumps({'passed':0,'failure':str(error)}));okay=False
    raise SystemExit(0 if okay else 1)
