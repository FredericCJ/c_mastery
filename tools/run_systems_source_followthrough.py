"""Execute the original six-record format companion; source inputs stay separate."""
import argparse,datetime,hashlib,json,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def record(kind,offset,data):
 raw=bytes([len(data),offset>>8,offset&255,kind])+bytes(data);return ':'+(raw+bytes([-sum(raw)&255])).hex().upper()+'\n'
def cases():
 result=[]
 for mode in (1,2):
  for base in (0,1,0x1234,0xffff):
   for off in (0,1,0xfffe,0xffff):
    for count in (0,1,2,4,64):
     data=[(i*37+11)%256 for i in range(count)];text=record(2 if mode==1 else 4,0,[base>>8,base&255])+record(0,off,data)+record(1,0,[])
     addresses=[base*16+(off+i)%65536 if mode==1 else (base*65536+off+i)%4294967296 for i in range(count)]
     expected='0 '+str(count)+' 0 0'+''.join(' '+str(a)+':'+str(v) for a,v in zip(addresses,data))
     result.append(dict(name=f'mode{mode}-base{base}-offset{off}-count{count}',mode=mode,text=text,expected=expected))
 for mode in (1,2):
  for kind in ([3] if mode==1 else [3,5]):
   for value in (0,0x12345678,0xffffffff):
    result.append(dict(name=f'entry-{mode}-{kind}-{value}',mode=mode,text=record(kind,0,value.to_bytes(4,'big'))+record(1,0,[]),expected=f'0 0 {kind} {value}'))
 for count in [64,65,255]:
  data=[i%256 for i in range(count)];result.append(dict(name=f'capacity{count}',mode=0,text=record(0,0,data)+record(1,0,[]),expected='0 64 0 0'+''.join(f' {i}:{i}' for i in range(64)) if count==64 else '5'))
 for records in [15,16]:result.append(dict(name=f'records{records+1}',mode=0,text=record(0,0,[])*records+record(1,0,[]),expected='0 0 0 0' if records==15 else '5'))
 return result
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--report',required=True);args=parser.parse_args();report=ROOT/args.report
 if report.exists() or not report.resolve().is_relative_to((ROOT/'reviews/v5/expansion/systems_source_followthrough').resolve()):raise ValueError('fresh scoped report')
 stamp=datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work=ROOT/'build/systems_source_followthrough'/stamp;work.mkdir(parents=True)
 paths=[pathlib.Path(__file__).resolve(),*sorted((ROOT/'labs/systems_source_followthrough').glob('*')),*sorted((ROOT/'assessor/systems_source_followthrough').glob('*'))];pins={p.relative_to(ROOT).as_posix():sha(p) for p in paths if p.is_file()};checks=[];commands=[];error=None
 def run(name,argv,expected=0,data=None,contains=None):
  p=subprocess.run(list(map(str,argv)),input=data,text=True,capture_output=True,timeout=30);ok=p.returncode==expected and (contains is None or contains in p.stdout+p.stderr);commands.append(dict(name=name,argv=list(map(str,argv)),exit_code=p.returncode,expected=expected,stdin=data,stdout=p.stdout,stderr=p.stderr,passed=ok));assert ok,name;return p.stdout.strip()
 try:
  src=ROOT/'assessor/systems_source_followthrough/hex.c';pub=ROOT/'labs/systems_source_followthrough'
  for compiler in ['gcc','clang']:
   for profile,flags in [('debug',['-O0','-g']),('optimized',['-O2']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
    prefix=compiler+'-'+profile;exe=work/prefix;common=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*flags,'-I'+str(pub)]
    run(prefix+'-compile',[*common,src,pub/'tests.c','-o',exe]);run(prefix+'-test',[exe],contains='SOURCE_HEX_PASS:')
    driver=work/(prefix+'-driver');run(prefix+'-driver-compile',[*common,src,pub/'driver.c','-o',driver])
    for case in cases():
     observed=run(prefix+'-'+case['name'],[driver,str(case['mode'])],data=case['text']);assert observed==case['expected'],prefix+' '+case['name'];checks.append(dict(profile=prefix,case=case['name'],expected=case['expected'],observed=observed,passed=True))
   for mutant in ['SF_MUT_CHECKSUM','SF_MUT_SEGMENT','SF_MUT_ENTRY','SF_MUT_WINDOW']:
    exe=work/(compiler+'-'+mutant);run(compiler+'-'+mutant+'-compile',[compiler,'-std=c17','-O2','-Wall','-Wextra','-Werror','-D'+mutant,'-I'+str(pub),src,pub/'tests.c','-o',exe]);run(compiler+'-'+mutant+'-reject',[exe],expected=1,contains='SOURCE_HEX_ASSERT:')
   exe=work/(compiler+'-stub');run(compiler+'-stub-compile',[compiler,'-std=c17','-O2','-Wall','-Wextra','-Werror','-I'+str(pub),pub/'hex.c',pub/'tests.c','-o',exe]);run(compiler+'-stub-reject',[exe],expected=1,contains='SOURCE_HEX_ASSERT:')
  assert all(sha(ROOT/p)==h for p,h in pins.items()),'source changed during campaign'
 except Exception as e:error=str(e)
 result=dict(schema_version=1,passed=error is None,created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256=pins,commands=commands,checks=checks,error=error,scope='Six-record original byte parser and independent arithmetic oracle; no firmware load or runtime-loader conformance',limits=['Explicit declared format subset policies, not arbitrary programmer compatibility.','Author execution only; learner and external-review evidence absent.','No physical device, filesystem deployment, flash or entry-point execution.'])
 report.parent.mkdir(parents=True,exist_ok=True);report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=result['passed'],commands=len(commands),checks=len(checks),error=error)));return int(error is not None)
if __name__=='__main__':raise SystemExit(main())
