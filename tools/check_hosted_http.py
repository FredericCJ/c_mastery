"""Read-only current-closure guard plus ordinary learner export checks."""
import argparse,copy,hashlib,json,re,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'reviews/v5/expansion/legacy_hosted_http';LAB=ROOT/'labs/hosted_followthrough/http';REF=ROOT/'assessor/hosted_followthrough/http';sys.path[:0]=[str(ROOT),str(LAB),str(ROOT/'tools')]
from run_hosted_http import sources
import parser_cases
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def need(x,message):
 if not x:raise ValueError(message)
def read(path):
 p=(ROOT/path).resolve();need(p.is_relative_to(ROOT.resolve()),'Escaped path');return p.read_bytes()
def validate_campaign(v):
 need(v.get('passed') is True and v.get('error') is None,'Campaign not passed')
 need(v['inspected_paths']=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'Stale current source closure')
 artifacts=v['artifact_sha256'];need(artifacts and all(hashlib.sha256(read(p)).hexdigest()==h for p,h in artifacts.items()),'Missing/stale artifact')
 rows={r['label']:r for r in v['commands']};need(len(rows)==len(v['commands'])==57,'Missing/duplicate command')
 def text(name):
  r=rows[name];need(r['output'] in artifacts,'Unbound command output');return read(r['output']).decode()
 def result(name,code,marker=''):
  r=rows[name];need(r['exit_code']==r['expected_exit_code']==code,'Wrong outcome '+name);need(marker in text(name),'Missing mechanism '+name)
 expected,wire=parser_cases.batch();need(len(expected)==8029 and len(parser_cases.cases())==144,'Unexpected corpus identity')
 corpus_paths=[p for p in artifacts if p.endswith('/parser-input.txt')];need(len(corpus_paths)==1 and read(corpus_paths[0]).decode()==wire,'Wrong exact input partitions')
 for cc in ('gcc','clang'):
  result(cc+'-version',0,'15.2' if cc=='gcc' else '21.1.8')
  for profile,flags in [('debug',['-O0','-g']),('release',['-O3','-DNDEBUG'])]:
   prefix=cc+'-'+profile
   for kind in ('core','probe'):
    build=rows[prefix+'-'+kind+'-build']['command'];need(build[0]==cc and all(f in build for f in ['-std=c17','-Werror','-pedantic',*flags]) and str(REF/'flow.c') in build and str(LAB/(('tests' if kind=='core' else 'probe')+'.c')) in build,'Wrong compile selection')
    result(prefix+'-'+kind+'-build',0);result(prefix+'-'+kind,0)
   core=text(prefix+'-core')
   for marker in ('lifecycle:','2-pins-quiescent','admission:','flags-failure-close','io:offsets4,4,6,6,9','response:HEAD10,19','fifo1:','fifo2:','prefix-fifo:','closing:','deadlines:','eof-events:','HF_PASS:all'):need(marker in core,'Missing distinct core mechanism '+marker)
   actual=[json.loads(s) for s in text(prefix+'-probe').splitlines()];groups=parser_cases.verify(expected,actual)
   for e,a in zip(expected,actual):
    if e['name']=='extension-129':need(a['used']==bytes.fromhex(e['wire']).index(b'\r\n\r\n')+4+1+129+2,'Wrong first extension rejection boundary')
   observation=next(r for r in v['parser_observations'] if r['profile']==prefix);need(observation=={'profile':prefix,'cases':144,'partitions':8029,'groups':groups},'Wrong parser observations')
   result(prefix+'-ledger',0);ledger=json.loads(text(prefix+'-ledger'));need(ledger=={'struct http_request':2224,'struct http_parser':10640,'struct hf_output':1632,'struct hf_flow':13960,'struct hf_pool':80,'four_flows':55840,'line':8193,'body':1024,'target':1025,'output_bytes':3264},'Wrong actual ABI ledger')
  for kind in ('linux','server','custom'):result(cc+'-'+kind+'-build',0)
  need('-DHF_SERVER_SOURCE="'+str(REF/'server.c')+'"' in rows[cc+'-linux-build']['command'],'Linux harness did not select explicit assessor source')
  result(cc+'-linux',0,'LINUX_PASS:5-initializer-failures:unchanged-publication');need('accept-EAGAIN:flags:buffered5-before-EOF' in text(cc+'-linux'),'Missing readiness/EOF observation')
  result(cc+'-custom',0,'CUSTOM_HTTP_PASS:borrow-call-only,response-retained-until-ACK,retransmit-identical,no-Date,existing-com_net')
  for cap,mode in [(2,1),(1,2),(2,0)]:
   name=f'{cc}-socket-{cap}-{mode}';result(name,0);command=rows[name]['command'];need(command[1]==str(LAB/'socket_client.py') and command[command.index('--capacity')+1]==str(cap) and command[command.index('--date-mode')+1]==str(mode),'Wrong socket adapter configuration')
   path=Path(command[command.index('--report')+1]).relative_to(ROOT).as_posix();need(path in artifacts,'Unbound socket report');s=json.loads(read(path));need(s['passed'] and s['owned_child_reaped'] and s['exit_code']==0 and len(s['records'])==17,'Incomplete socket observations');need('ledger=56176' in s['ready'],'Wrong server-owned record ledger')
   records={r['case']:r for r in s['records']};need(len(records)==17 and all(r['passed'] is True for r in records.values()),'Missing socket case')
   for label,statuses in [('head-pipeline',[200,404,405]),('ordered-100-binary',[200,100,200]),('half-close-False',[400]),('half-close-True',[200]),('te-cl-reject-and-close',[400]),('upgrade-reject-and-close',[501]),('connect-reject-and-close',[501]),('duplicate-reject-and-close',[400]),('close-second-drops-third',[200,200]),('maximum-four',[200]*4),('chunk-byte-writes',[200])]:
    r=records[label];raw=bytes.fromhex(r['response_wire_hex']);need(r['expected']==statuses and [int(x) for x in re.findall(rb'HTTP/1.1 (\d{3}) ',raw)]==statuses,'Wrong raw response association '+label)
    if mode==0:need(b'Date:' not in raw,'Unavailable clock fabricatedDate')
    elif mode==2:need(b'Date: Sun, 01 Jan 2023 00:00:00 GMT' in raw,'Wrong injectedDate')
   need(bytes.fromhex(records['ordered-100-binary']['response_wire_hex']).endswith(b'\0\x80\xff\r\n'),'Binary output altered')
   for kind in ('get','head','binary','expect'):
    r=records['curl-'+kind];need(r['command'][0]=='curl' and r['exit_code']==0 and b'HTTP/1.1 200 OK' in bytes.fromhex(r['response_wire_hex']),'Missing independent curl case')
   drain=records['actual-drain-deadline'];need(drain['descriptor_after']<drain['descriptor_before'] and drain['elapsed_seconds']>=1,'No actual drain reclamation')
   err=Path(s['stderr_path']).relative_to(ROOT).as_posix();need(err in artifacts,'Unbound actual flag log');flags=re.findall(r'ACCEPT fd=\d+ generation=\d+ flags=(\d+),(\d+)',read(err).decode());need(flags and all(int(a)&2048 and int(b)&1 for a,b in flags),'Accepted flags not observed')
  result(cc+'-blocked-output',0,'offset0:abort-not-drain');m=re.search(r'prefill=(\d+):EAGAIN=(\d+):output-timeout-ms=(\d+)',text(cc+'-blocked-output'));need(m and 0<int(m[1])<2097152 and int(m[2])>0 and int(m[3])>=15000,'No bounded real output timeout')
  result(cc+'-object',0);need('-U_FORTIFY_SOURCE' in rows[cc+'-object']['command'],'Wrong freestanding dependency profile');result(cc+'-undefined',0);symbols=set(text(cc+'-undefined').split())-{'U'};need(symbols=={'http_drain','http_eof','http_expired','http_feed','http_init','memcpy','memset','strcmp','strlen'},'Undeclared core dependency')
  result(cc+'-server-stack',0);need(any(p.endswith(cc+'-server.su') for p in artifacts) and any(p.endswith(cc+'-flow.su') for p in artifacts),'Missing stack estimates')
 for label in ('avr-version','avr-emulator-version','avr-build','avr-symbols','avr-undefined','avr-size','avr-execution','independent-models','stub-build','curl-version'):result(label,0)
 need('14.3.0' in text('avr-version') and '10.2.1' in text('avr-emulator-version'),'Wrong AVR tools');need(not text('avr-undefined').strip(),'Undefined AVR runtime');need('.data         0' in text('avr-size'),'Uninitialized AVR data');need('__udivmodsi4' in text('avr-symbols'),'Compiler helper audit missing')
 avr=json.loads(text('avr-execution').splitlines()[0]);need(avr['serial_hex']=='4156523a504153530a' and avr['size_t_bits']==16 and avr['uint32_t_bits']==32 and avr['command'][2]=='arduino-uno' and avr['controlled_child_exit']==0 and 'terminating on signal 15' in avr['stderr'],'Wrong target observation')
 model=json.loads(text('independent-models').splitlines()[0]);need(model['disagreement']['length_boundary']+1==model['disagreement']['chunk_boundary'] and model['disagreement']['repaired']==[400,'close',0] and model['optimistic']['repaired_effects']==0 and model['wire_order']==[1,2],'Wrong changed-premise model')
 result('stub',1,'HF_CHECK:');need(len(v['checks'])==29 and all(r['passed'] is True for r in v['checks']),'Failed/missing source checks');need(len(v['open_gates'])==2 and 'unobserved' in v['open_gates'][0] and 'physical' in v['open_gates'][1],'External evidence invented')
 return True
def main(report):
 from research.v5.expansion.legacy_hosted_http.author_support import attach,SUPPORT,PROJECTS,public_paths
 from cmastery import assessments
 campaign=BASE/'campaign.json';v=json.loads(campaign.read_text());validate_campaign(v);checks=[{'name':'current-source-bound-mechanisms','passed':True}]
 def check(name,value):checks.append({'name':name,'passed':bool(value)})
 controls=[('missing-command',lambda x:x['commands'].pop()),('changed-source',lambda x:x['inspected_paths'].update({'labs/hosted_followthrough/http/flow.h':'0'*64})),('changed-output',lambda x:x['artifact_sha256'].update({next(iter(x['artifact_sha256'])):'0'*64})),('stub-as-pass',lambda x:next(r for r in x['commands'] if r['label']=='stub').update(exit_code=0)),('wrong-profile',lambda x:next(r for r in x['commands'] if r['label']=='clang-release-core-build')['command'].remove('-DNDEBUG')),('missing-partitions',lambda x:x['parser_observations'][0].update(partitions=1)),('wrong-socket-policy',lambda x:next(r for r in x['commands'] if r['label']=='gcc-socket-1-2')['command'].__setitem__(6,'2')),('invented-independent',lambda x:x.update(open_gates=[]))]
 for name,mutate in controls:
  d=copy.deepcopy(v);mutate(d)
  try:validate_campaign(d);rejected=False
  except (ValueError,KeyError,IndexError,AssertionError):rejected=True
  check(name,rejected)
 for pid in PROJECTS:
  path=ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json');r=json.loads(path.read_text());old=json.loads(subprocess.check_output(['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:'+path.relative_to(ROOT).as_posix()],cwd=ROOT,text=True));criteria=copy.deepcopy(r['criteria'])
  for c in criteria:c.pop('successor_support',None)
  check(pid+'-original-criteria',criteria==old['criteria']);check(pid+'-historical-contracts',all(r[k]==old[k] for k in ('baseline_binding','execution_support','status')));check(pid+'-idempotent',attach(copy.deepcopy(r))==r)
  with tempfile.TemporaryDirectory(prefix='hosted-http-export-') as d:
   work=Path(d);assessments.export(ROOT,pid,work);check(pid+'-public-export',all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in public_paths()) and not any('assessor' in p.relative_to(work).parts for p in work.rglob('*')))
   if pid in ('PR-T42-01','PR-T42-08'):
    for ref in (False,True):
     kind='core' if pid=='PR-T42-01' else 'linux';marker='HF' if kind=='core' else 'LINUX';exe=work/('check-'+str(ref));cmd=[sys.executable,work/'labs/hosted_followthrough/http/build.py','--kind',kind,'--output',exe]
     if ref:
      parser=next(ROOT/p for p in v['artifact_sha256'] if p.endswith('/parser.c'));cmd+=['--flow-source',REF/'flow.c','--parser-source',parser,'--depth-source',ROOT/'assessor/http_depth/http_depth.c','--server-source',REF/'server.c']
     b=subprocess.run(list(map(str,cmd)),capture_output=True,text=True,timeout=60);okay=False
     if b.returncode==0:
      e=subprocess.run([exe],capture_output=True,text=True,timeout=10);okay=e.returncode==(0 if ref else 1) and marker+('_PASS:' if ref else '_CHECK:') in e.stdout+e.stderr
     check(pid+('-explicit-reference' if ref else '-incomplete-stub'),okay)
 support=json.loads((ROOT/SUPPORT).read_text());original=[c for pid in PROJECTS for c in json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments'/(pid+'.json')).read_text())['criteria']];expected={c['id'] for c in original}|{variant['id'] for c in original for variant in c['variants']}
 check('48-exact-identities',len(support['updates'])==48 and {r['id'] for r in support['updates']}==expected);check('support-pins',all(hashlib.sha256(read(p)).hexdigest()==h for p,h in support['source_sha256'].items()));check('external-authority-separate',all(r['dimensions']['external_review']['state']=='not-run' and r['dimensions']['independent_learner']['state']=='not-assessed' and r['dimensions']['physical_observation']['state'].startswith('not-run') for r in support['updates']))
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':all(c['passed'] for c in checks),'checks':checks,'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in (Path(__file__).resolve(),campaign,ROOT/SUPPORT)},'scope':'Source-bound author evidence and learner exports;delayed/external/physical dimensions remain separate.'};dest=(ROOT/report).resolve();need(dest.is_relative_to(BASE.resolve()),'Guard report escapes');dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'checks':len(checks),'failed':[c['name'] for c in checks if not c['passed']]}));return 0 if result['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
