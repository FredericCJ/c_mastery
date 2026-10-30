"""Source-bound T42 execution producer. Every attempt is an immutable new path."""
import argparse,hashlib,json,os,subprocess,sys,time
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/hosted_followthrough/http';REF=ROOT/'assessor/hosted_followthrough/http';BASE=ROOT/'reviews/v5/expansion/legacy_hosted_http'
sys.path[:0]=[str(LAB),str(ROOT/'tools')]
from adapt_hosted_http import compose
import parser_cases
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():
 files=[*LAB.rglob('*'),*REF.rglob('*'),*(ROOT/'fixtures/network_http').glob('*.json'),Path(__file__),ROOT/'tools/adapt_hosted_http.py',ROOT/'tools/adapt_http_depth_baseline.py',ROOT/'assessor/communications/http.c',ROOT/'assessor/communications/net.c',ROOT/'assessor/http_depth/http_depth.c',ROOT/'labs/communications/include/com_http.h',ROOT/'labs/communications/include/com_net.h',ROOT/'labs/http_depth/include/http_depth.h',ROOT/'labs/communications/tests/net.c']
 return sorted({p for p in files if p.is_file() and '__pycache__' not in p.parts})
def main(report):
 dest=(ROOT/report).resolve();assert dest.is_relative_to(BASE.resolve()) and not dest.exists();out=dest.with_suffix('');out.mkdir(parents=True,exist_ok=False)
 rows=[];checks=[];artifacts=set();before={p.relative_to(ROOT).as_posix():sha(p) for p in sources()}
 def run(label,cmd,code=0,stdin=None,timeout=60,env=None):
  command=list(map(str,cmd));start=time.monotonic();r=subprocess.run(command,cwd=ROOT,input=stdin,capture_output=True,text=True,timeout=timeout,env=env)
  log=out/(label+'.txt');log.write_text(r.stdout+r.stderr,encoding='utf-8',newline='\n');artifacts.add(log);rows.append({'label':label,'command':command,'exit_code':r.returncode,'expected_exit_code':code,'elapsed_seconds':time.monotonic()-start,'output':log.relative_to(ROOT).as_posix()});assert r.returncode==code,(label,r.returncode,r.stdout,r.stderr);return r.stdout+r.stderr
 def check(name,condition):checks.append({'name':name,'passed':bool(condition)});assert condition,name
 try:
  for tool in ('gcc','clang','curl'):run(tool+'-version',[tool,'--version'])
  parser=out/'parser.c';composition=compose(parser);artifacts.add(parser)
  custom=out/'net-no-date.c';s=(ROOT/'assessor/communications/net.c').read_text();anchor='"Sat, 10 Oct 2026 00:00:00 GMT"';assert s.count(anchor)==1;custom.write_text(s.replace(anchor,'0'));artifacts.add(custom)
  expected,wire=parser_cases.batch();inputs=out/'parser-input.txt';inputs.write_text(wire);artifacts.add(inputs)
  manifest=out/'parser-expected.json';manifest.write_text(json.dumps(expected,indent=2)+'\n');artifacts.add(manifest)
  observations=[]
  for cc in ('gcc','clang'):
   for profile,flags in [('debug',['-O0','-g']),('release',['-O3','-DNDEBUG'])]:
    prefix=cc+'-'+profile;common=[cc,'-std=c17','-Wall','-Wextra','-Werror','-pedantic',*flags,'-I'+str(LAB),'-I'+str(REF),'-I'+str(ROOT/'labs/communications/include'),'-I'+str(ROOT/'labs/http_depth/include'),str(parser),str(ROOT/'assessor/http_depth/http_depth.c'),str(REF/'flow.c')]
    for kind,source in [('core',LAB/'tests.c'),('probe',LAB/'probe.c')]:
     exe=out/(prefix+'-'+kind);run(prefix+'-'+kind+'-build',common+[source,'-o',exe]);artifacts.add(exe)
     text=run(prefix+'-'+kind,[exe],stdin=wire if kind=='probe' else None)
     if kind=='core':check(prefix+'-core','HF_PASS:all' in text)
     else:
      actual=[json.loads(s) for s in text.splitlines()];groups=parser_cases.verify(expected,actual);observations.append({'profile':prefix,'cases':len(groups),'partitions':len(actual),'groups':groups});check(prefix+'-independent-parser-oracles',len(groups)==144 and len(actual)==len(expected))
    ledger=json.loads(run(prefix+'-ledger',[out/(prefix+'-core'),'ledger']));check(prefix+'-memory-ledger',ledger['line']==8193 and ledger['body']==1024 and ledger['target']==1025 and ledger['four_flows']==4*ledger['HFFlow'])
   common=[cc,'-std=c17','-O2','-g','-Wall','-Wextra','-Werror','-pedantic','-I'+str(LAB),'-I'+str(REF),'-I'+str(ROOT/'labs/communications/include'),'-I'+str(ROOT/'labs/http_depth/include'),str(parser),str(ROOT/'assessor/http_depth/http_depth.c'),str(REF/'flow.c')]
   for kind,source in [('linux',LAB/'linux_tests.c'),('server',REF/'server.c'),('custom',LAB/'custom_tcp.c')]:
    exe=out/(cc+'-'+kind);run(cc+'-'+kind+'-build',common+([custom] if kind=='custom' else ['-DHF_SERVER_SOURCE="'+str(REF/'server.c')+'"'] if kind=='linux' else [])+[source,'-o',exe]);artifacts.add(exe)
    if kind!='server':text=run(cc+'-'+kind,[exe]);check(cc+'-'+kind+'-mechanism',('LINUX_PASS:' if kind=='linux' else 'CUSTOM_HTTP_PASS:') in text)
   for cap,mode in [(2,1),(1,2),(2,0)]:
    socket_report=out/f'{cc}-socket-{cap}-{mode}.json';run(f'{cc}-socket-{cap}-{mode}',[sys.executable,LAB/'socket_client.py','--binary',out/(cc+'-server'),'--capacity',str(cap),'--date-mode',str(mode),'--report',socket_report],timeout=40);r=json.loads(socket_report.read_text());check(f'{cc}-socket-{cap}-{mode}-actual',r['passed'] and r['owned_child_reaped'] and r['exit_code']==0 and len(r['records'])==17);artifacts.update([socket_report,socket_report.with_suffix('.stderr.txt')])
   # Same owned TCP backpressure probe on each actual compiler; real15s output
   # expiry, separate from the1s close drain observed in socket_client.
   text=run(cc+'-blocked-output',[out/(cc+'-linux'),'blocked-output'],timeout=25);check(cc+'-actual-output-abort','output-timeout-ms=' in text and 'offset0:abort-not-drain' in text)
   obj=out/(cc+'-flow.o');run(cc+'-object',[cc,'-std=c17','-O2','-ffreestanding','-fno-builtin','-U_FORTIFY_SOURCE','-fstack-usage','-I'+str(LAB),'-I'+str(ROOT/'labs/communications/include'),'-c',REF/'flow.c','-o',obj]);artifacts.add(obj);artifacts.add(obj.with_suffix('.su'));run(cc+'-undefined',['nm','-u',obj])
   serverobj=out/(cc+'-server.o');run(cc+'-server-stack',[cc,'-std=c17','-O2','-fstack-usage','-I'+str(LAB),'-I'+str(ROOT/'labs/communications/include'),'-I'+str(ROOT/'labs/http_depth/include'),'-c',REF/'server.c','-o',serverobj]);artifacts.update([serverobj,serverobj.with_suffix('.su')])
  avr=Path('/home/frede/cmastery-http-avr/root/usr');env=dict(os.environ);env['PATH']=str(avr/'bin')+':'+env['PATH'];run('avr-version',[avr/'bin/avr-gcc','--version'],env=env);run('avr-emulator-version',['/home/frede/cmastery-b06-tools/usr/bin/qemu-system-avr','--version'])
  elf=out/'length-avr.elf';run('avr-build',[avr/'bin/avr-gcc','-mmcu=atmega328p','-std=c17','-Os','-ffreestanding','-ffunction-sections','-fdata-sections','-fno-builtin','-nostdlib','-Wall','-Wextra','-Werror','-I'+str(ROOT/'labs/http_depth/include'),LAB/'avr_start.S',LAB/'avr_length.c',ROOT/'assessor/http_depth/http_depth.c','-Wl,--gc-sections,-e,_start','-lgcc','-o',elf],env=env);artifacts.add(elf)
  run('avr-symbols',[avr/'bin/avr-nm',elf]);undefined=run('avr-undefined',[avr/'bin/avr-nm','-u',elf]);check('avr-no-undefined',not undefined.strip());sizes=run('avr-size',[avr/'bin/avr-size','-A',elf]);check('avr-no-data-initialization','.data         0' in sizes);check('avr-actual-size_t16','AVR_LENGTH_PASS:' in run('avr-execution',[sys.executable,LAB/'run_avr.py','--binary',elf],timeout=10))
  run('independent-models',[sys.executable,LAB/'models.py'])
  # Learner's unimplemented flow must compile and fail the semantic contract.
  stub=out/'stub';run('stub-build',['gcc','-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I'+str(LAB),'-I'+str(ROOT/'labs/communications/include'),'-I'+str(ROOT/'labs/http_depth/include'),parser,ROOT/'assessor/http_depth/http_depth.c',LAB/'flow.c',LAB/'tests.c','-o',stub]);artifacts.add(stub);check('incomplete-stub-rejected','HF_CHECK:' in run('stub',[stub],code=1))
  check('source-stable',before=={p.relative_to(ROOT).as_posix():sha(p) for p in sources()})
 except Exception as exc:
  error=repr(exc);passed=False
 else:error=None;passed=all(x['passed'] for x in checks)
 result={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'passed':passed,'error':error,'commands':rows,'checks':checks,'inspected_paths':before,'artifact_sha256':{p.relative_to(ROOT).as_posix():sha(p) for p in sorted(artifacts)},'parser_observations':locals().get('observations',[]),'composition':locals().get('composition',{}),'open_gates':['Independent human review and minimum-seven-day unaided learner assessment unobserved.','CustomTCP packet execution and named AVR emulation do not establish physical cable/board or generalTCP conformance.']}
 dest.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8',newline='\n');print(json.dumps({'passed':passed,'commands':len(rows),'checks':len(checks),'error':error}));return 0 if passed else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
