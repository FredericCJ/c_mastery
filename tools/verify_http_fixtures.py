"""Independent finite HTTP fixture calculations; no learner C/server execution."""
from datetime import datetime, timezone
from copy import deepcopy
from email.utils import format_datetime
from urllib.parse import urlsplit
import re
import shutil
from pathlib import Path
from tempfile import TemporaryDirectory
from common import ROOT, read_json, dump, parse

HTTP_STEMS=['contract','01_lifecycle','02_grammar','03_framing','04_chunks',
            '05_semantics','06_output','07_deadlines','08_security','09_wire']

def assert_fixture_integrity(root):
    folder=root/'fixtures/network_http'
    expected={name+'.json' for name in HTTP_STEMS}
    actual={p.relative_to(folder).as_posix():p for p in folder.rglob('*.json')}
    assert set(actual)==expected,('missing or orphan HTTP fixture',set(actual)^expected)
    for name,path in actual.items():
        assert path.resolve().is_relative_to(folder.resolve()),('escaping fixture',name)
        assert read_json(path)['id']=='FX-HTTP-'+path.stem,('HTTP fixture ID',name)
    module,_=parse(root/'curriculum/modules/T42.md')
    ids=['PR-T42-'+str(n).zfill(2) for n in range(1,9)]
    actual_projects={p.relative_to(root/'curriculum/projects').as_posix() for p in (root/'curriculum/projects').rglob('PR-T42-*.md')}
    assert actual_projects=={key+'.md' for key in ids}, 'HTTP project file set'
    assert module['id']=='T42' and module['projects']==ids, 'HTTP module/project order'
    for n,key in enumerate(ids,1):
        project,_=parse(root/f'curriculum/projects/{key}.md')
        paths=['fixtures/network_http/contract.json','fixtures/network_http/'+HTTP_STEMS[n]+'.json']
        if n in [5,6,8]: paths.append('fixtures/network_http/09_wire.json')
        assert project['id']==key and project['module']=='T42',('HTTP project identity',key)
        assert project['fixtures']==paths,('HTTP project/fixture binding',key)
    adapter,_=parse(root/'curriculum/projects/PR-T41-08.md')
    assert adapter['fixtures'].count('fixtures/network_http/09_wire.json')==1, 'shared wire adapter binding'

def integrity_controls():
    controls=0
    for kind in ['id','binding','module','missing','flat_orphan','nested_orphan','extra_project']:
        with TemporaryDirectory(prefix='c-mastery-http-integrity-') as name:
            root=Path(name)
            shutil.copytree(ROOT/'fixtures/network_http',root/'fixtures/network_http')
            for path in [ROOT/'curriculum/modules/T42.md',ROOT/'curriculum/projects/PR-T41-08.md']+list((ROOT/'curriculum/projects').glob('PR-T42-*.md')):
                target=root/path.relative_to(ROOT); target.parent.mkdir(parents=True,exist_ok=True)
                shutil.copyfile(path,target)
            if kind=='id':
                target=root/'fixtures/network_http/06_output.json'; value=read_json(target)
                value['id']='UNRELATED'; dump(target,value)
            elif kind=='binding':
                target=root/'curriculum/projects/PR-T42-06.md'
                target.write_text(target.read_text(encoding='utf-8').replace('06_output.json','unlisted.json'),encoding='utf-8')
            elif kind=='module':
                target=root/'curriculum/modules/T42.md'
                target.write_text(target.read_text(encoding='utf-8').replace('"PR-T42-06"','"PR-T42-05"'),encoding='utf-8')
            elif kind=='missing': (root/'fixtures/network_http/06_output.json').unlink()
            elif kind=='extra_project':
                shutil.copyfile(root/'curriculum/projects/PR-T42-06.md',root/'curriculum/projects/PR-T42-09.md')
            else:
                target=root/('fixtures/network_http/nested/unlisted.json' if kind=='nested_orphan' else 'fixtures/network_http/unlisted.json')
                dump(target,read_json(root/'fixtures/network_http/06_output.json'))
            try: assert_fixture_integrity(root)
            except AssertionError: controls+=1
            else: raise AssertionError('Invalid HTTP fixture integrity accepted: '+kind)
    return controls

BOOL_FIELDS={'accepted','upgrade','ambiguous_framing','following_request','connection_close','physical_layers_owned_by_server',
             'idle_expired','request_total_expired','output_expired','drain_expired',
             'output_success_claim','tls_owned'}

def assert_fixture_types(value, path=()):
    """JSON boolean and integer are distinct even though Python equality permits True==1."""
    is_boolean=(bool(path) and path[-1] in BOOL_FIELDS) or (len(path)>1 and path[-2]=='close') or (
        len(path)>2 and path[-3]=='outcomes' and path[-1]==2)
    if is_boolean:
        assert type(value) is bool,('expected boolean',path,value)
        return
    assert type(value) is not bool and type(value) is not float,('unexpected numeric type',path,value)
    if isinstance(value,dict):
        for key,item in value.items(): assert_fixture_types(item,path+(key,))
    elif isinstance(value,list):
        for index,item in enumerate(value): assert_fixture_types(item,path+(index,))

def main():
    assert_fixture_integrity(ROOT)
    cases=0
    def get(name):
        nonlocal cases
        item=read_json(ROOT/f'fixtures/network_http/{name}.json')
        assert_fixture_types(item)
        for field in ['id','description','input','expected','verification_method']:
            assert field in item,(name,field)
        cases+=1
        return item['input'],item['expected']
    contract,expected=get('contract')
    assert bytes.fromhex(expected['health_representation_hex'])==b'ok\n'
    assert expected['health_octets']==len(b'ok\n')==3
    assert expected['routes']==['/health','/echo'] and not expected['physical_layers_owned_by_server']
    assert contract['limits']=={'connections':4,'header_bytes':8192,'header_fields':32,'body_bytes':1024,
        'request_target_bytes':1024,'chunk_extension_bytes_per_chunk':128,'trailer_bytes':1024,'trailer_fields':8,
        'queued_complete_requests':2,'requests_per_connection':4,'idle_ms':5000,
        'request_total_ms':15000,'output_total_ms':15000,'close_drain_ms':1000}

    x,e=get('01_lifecycle'); live=[]; rejected=[]
    for fd in x['accepted_fds']:
        (live if len(live)<x['capacity'] else rejected).append(fd)
    assert live==e['admitted'] and rejected==e['rejected']
    live.remove(x['close_fd']); live.append(x['replacement_fd'])
    assert live==e['after_release_and_replacement'] and len(set(live))==x['capacity']
    assert e['required_flags']==['nonblocking','close_on_exec'] and e['race_leaks']==0

    x,e=get('02_grammar'); accepted=[]; rejected=[]; ends=[]
    token_chars="!#$%&'*+-.^_|~"+chr(96)
    token=re.compile(r"[A-Za-z0-9"+re.escape(token_chars)+r"]+")
    for index,case in enumerate(x['cases']):
        raw=case['wire'].encode('latin1'); end=raw.find(b'\r\n\r\n')
        valid=end>=0 and b'\x00' not in raw
        authority=None
        if valid:
            lines=raw[:end].decode('latin1').split('\r\n')
            parts=lines[0].split(' ')
            valid=len(parts)==3 and parts[2]=='HTTP/1.1' and bool(token.fullmatch(parts[0]))
            fields=[]
            for line in lines[1:]:
                if ':' not in line or line.startswith((' ','\t')):
                    valid=False; break
                name,value=line.split(':',1)
                if not token.fullmatch(name) or any(ord(c)<32 and c!='\t' for c in value):
                    valid=False; break
                fields.append((name.lower(),value.strip(' \t')))
            hosts=[v for k,v in fields if k=='host']
            valid=valid and len(hosts)==1 and bool(hosts[0]) and not any(c.isspace() for c in hosts[0])
            if valid and parts[1].startswith('http://'):
                authority=urlsplit(parts[1]).netloc
                valid=authority=='localhost:8080'
            elif valid:
                authority=hosts[0]; valid=parts[1]=='/health'
        assert valid==case['accepted'],(index,valid)
        if valid:
            accepted.append(index); ends.append(end+4)
            for split in range(len(raw)+1):
                assert raw[:split]+raw[split:]==raw
            if index==1: assert authority==e['absolute_target_authority']
        else: rejected.append(index)
    assert accepted==e['accepted_case_indices'] and rejected==e['rejected_case_indices']
    assert ends==e['valid_header_end_octets'],ends

    x,e=get('03_framing'); outcomes=[]
    for case in x['cases']:
        pairs=[(k.lower(),v.strip(' \t')) for k,v in case['headers']]
        lengths=[v for k,v in pairs if k=='content-length']
        codings=[v for k,v in pairs if k=='transfer-encoding']
        if len(lengths)>1 or (lengths and codings):
            result=['ERROR',400,True]
        elif codings:
            names=[part.strip(' \t').lower() for value in codings for part in value.split(',')]
            if len(codings)!=1 or any(not token.fullmatch(name) for name in names) or names.count('chunked')!=1 or names[-1]!='chunked':
                result=['ERROR',400,True]
            else:
                result=['CHUNKED',0,False] if names==['chunked'] else ['ERROR',501,True]
        elif lengths:
            value=lengths[0]
            if not re.fullmatch('[0-9]+',value) or int(value)>x['length_integer_max']:
                result=['ERROR',400,True]
            elif int(value)>x['body_limit']: result=['ERROR',413,True]
            else: result=['LENGTH',int(value),False]
        else: result=['NONE',0,False]
        outcomes.append(result)
    assert outcomes==e['outcomes']

    x,e=get('04_chunks')
    # This is a finite reconstruction of this fixture's wire; not a reusable HTTP receiver.
    raw=x['valid_wire'].encode('ascii')
    assert raw.startswith(b'3;x=y\r\n')
    first_start=len(b'3;x=y\r\n'); first=raw[first_start:first_start+3]
    assert raw[first_start+3:].startswith(b'\r\n2\r\n')
    second_start=first_start+3+len(b'\r\n2\r\n'); second=raw[second_start:second_start+2]
    assert raw[second_start+2:]==b'\r\n0\r\nX-Debug: ignored\r\n\r\n'
    assert (first+second).hex()==e['body_hex'] and len(first+second)==e['body_octets']
    assert e['trailers_discarded']==[['X-Debug','ignored']]
    assert not x['incomplete_wire'].endswith('\r\n\r\n') and e['incomplete_publishes']==0
    invalid=x['invalid_wire'].encode('ascii'); assert invalid[6:8]!=b'\r\n' and e['invalid_status']==400
    assert x['terminal_only']=='0\r\n\r\n' and bytes.fromhex(e['terminal_body_hex'])==b''

    x,e=get('05_semantics')
    assert format_datetime(datetime.fromtimestamp(x['utc_epoch_seconds'],timezone.utc),usegmt=True)==e['date']
    assert len(x['cases'])==len(e['responses'])
    for request,response in zip(x['cases'],e['responses']):
        method,target=request['method'],request['target']; echoed=bytes.fromhex(request['body_hex'])
        if method not in ['GET','HEAD','POST']: status,body=501,b'not implemented\n'
        elif target not in ['/health','/echo']: status,body=404,b'not found\n'
        elif target=='/health' and method not in ['GET','HEAD']: status,body=405,b'method not allowed\n'
        elif target=='/echo' and method!='POST': status,body=405,b'method not allowed\n'
        else: status,body=200,b'ok\n' if target=='/health' else echoed
        assert status==response['status'] and len(body)==response['length']
        assert (b'' if method=='HEAD' else body).hex()==response['body_hex']
        if status==405: assert response['allow']==('GET, HEAD' if target=='/health' else 'POST')
    assert e['expectation_first_status']==[100,417,400]

    x,e=get('06_output'); raw=bytes.fromhex(x['output_hex']); position=0; offsets=[]; emitted=b''
    for result in x['send_results']:
        if isinstance(result,int):
            assert 0<result<=len(raw)-position
            emitted+=raw[position:position+result]; position+=result
        else: assert result in ['EINTR','EAGAIN']
        offsets.append(position)
    assert offsets==e['offsets_after_each_result'] and emitted.hex()==e['emitted_hex']
    assert x['request_ids'][:x['queue_capacity']]==e['initial_queue']
    assert x['request_ids'][x['queue_capacity']]==e['paused_request']
    close_at=x['request_ids'].index(x['close_request_id'])
    assert x['request_ids'][:close_at+1]==e['response_order']
    assert x['request_ids'][close_at+1:]==e['cancelled_after_close']
    wire=read_json(ROOT/'fixtures/network_http/09_wire.json')
    messages={m['method']:m['header'].encode('ascii')+bytes.fromhex(m['body_hex']) for m in wire['expected']['messages']}
    ordered=[messages['GET'].replace(b'Connection: close',b'Connection: keep-alive'),
             wire['input']['interim_continue'].encode('ascii'),messages['POST']]
    pipeline=x['continue_pipeline']; certificates=e['continue_pipeline']
    assert pipeline['response_records']==[[1,'final'],[2,'interim100'],[2,'final']]
    # Request2 headers arrive after a prefix of response1. Its decision is ready,
    # but no response2 byte may overtake the retained final response1 bytes.
    assert 0<pipeline['request2_headers_after_octets']<len(ordered[0])
    combined=b''.join(ordered); position=0; offsets=[]; completed=[]
    boundaries=[len(ordered[0]),len(ordered[0])+len(ordered[1]),len(combined)]
    for result in pipeline['send_results']:
        if type(result) is int:
            assert 0<result<=len(combined)-position
            position+=result
        else: assert result in ['EINTR','EAGAIN']
        offsets.append(position)
        completed.append(sum(position>=boundary for boundary in boundaries))
    assert pipeline['send_results'][0]==pipeline['request2_headers_after_octets']
    assert position==len(combined) and offsets==certificates['offsets_after_each_result']
    assert boundaries==certificates['message_end_octets'] and completed==certificates['completed_messages_after_each_result']
    assert combined.hex()==certificates['emitted_hex']

    x,e=get('07_deadlines')
    assert (x['now']-x['progress'][-1]>=x['idle_ms'])==e['idle_expired']
    assert (x['now']-x['request_start']>=x['request_total_ms'])==e['request_total_expired']
    assert (x['output_now']-x['output_start']>=x['output_total_ms'])==e['output_expired']
    assert (x['drain_now']-x['drain_start']>=x['close_drain_ms'])==e['drain_expired']
    assert e['request_status']==408 and not e['output_success_claim']

    x,e=get('08_security'); statuses=[]
    for request in x['requests']:
        statuses.append(400 if request.get('ambiguous_framing') else 501 if request.get('upgrade') or request.get('method')=='CONNECT' else 200)
    assert statuses==e['first_status'] and e['close']==[True]*4
    assert e['following_requests_dispatched']==[0]*4 and not e['tls_owned'] and e['lower_layers_owned']=='OS'
    x,e=get('09_wire')
    assert x['interim_continue']=='HTTP/1.1 100 Continue\r\n\r\n'
    for message in e['messages']:
        header=message['header'].encode('ascii')
        assert header.endswith(b'\r\n\r\n') and not b'\r\n\r\n' in header[:-4]
        lines=header[:-4].split(b'\r\n')
        assert lines[0]==b'HTTP/1.1 200 OK'
        names=[line.split(b':',1)[0].decode('ascii') for line in lines[1:]]
        assert ['status-line']+names==x['header_order']
        fields=dict(line.split(b': ',1) for line in lines[1:])
        assert fields[b'Date']==b'Sun, 01 Jan 2023 00:00:00 GMT' and fields[b'Connection']==b'close'
        body=bytes.fromhex(message['body_hex'])
        if message['method']=='HEAD': assert not body and fields[b'Content-Length']==b'3'
        else: assert len(body)==int(fields[b'Content-Length'])
        if message['method']=='POST':
            assert body.hex()==x['echo_body_hex'] and fields[b'Content-Type']==b'application/octet-stream'
        elif message['method']=='GET': assert body==b'ok\n'
    controls=[('01_lifecycle',lambda f:f['expected'].update(race_leaks=False)),
              ('01_lifecycle',lambda f:f['input'].update(capacity=True)),
              ('03_framing',lambda f:f['expected']['outcomes'][2].__setitem__(1,True)),
              ('08_security',lambda f:f['expected']['close'].__setitem__(0,1))]
    for name,mutate in controls:
        broken=deepcopy(read_json(ROOT/f'fixtures/network_http/{name}.json')); mutate(broken)
        try: assert_fixture_types(broken)
        except AssertionError: pass
        else: raise AssertionError('Type-confused fixture was accepted: '+name)
    report={'passed':True,'fixtures':cases,'type_confusion_controls_rejected':len(controls),
            'integrity_controls_rejected':integrity_controls(),
            'scope':'Independent static byte/count/capacity/deadline and finite policy oracles only; original learner C, sockets and client interoperability not executed.'}
    dump(ROOT/'reviews/http_fixture_checks.json',report)
    print(report)

if __name__=='__main__':
    main()
