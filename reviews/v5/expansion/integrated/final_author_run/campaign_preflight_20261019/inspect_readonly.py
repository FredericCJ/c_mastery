"""Read-only dispatch preflight; never invokes a campaign or normal receipt writer."""
import hashlib,json,os,shutil,sys,time,traceback
from pathlib import Path
ROOT=Path.cwd().resolve()
OUT=Path(__file__).resolve().parent
EXPECTED='59d3c902835a2a61dd95baec894d612c14fbf2eea2a3160948bbad0dd39a356b'
GIT=['git','show','fd85773420a2fbb11003e83c8289417d62282ec4:curriculum/v5/expansion/legacy_assessments/PR-T43-10.json']
sys.dont_write_bytecode=True
sys.path.insert(0,str(ROOT/'tools'))
def sha(p):
 h=hashlib.sha256()
 with Path(p).open('rb') as f:
  for b in iter(lambda:f.read(1024*1024),b''):h.update(b)
 return h.hexdigest()
git=Path(shutil.which('git')).resolve()
read_only=True;events=[];denied=[];opened_python=set()
mutations={'os.mkdir','os.remove','os.rename','os.rmdir','os.chmod','os.chown','os.utime','os.link','os.symlink','os.truncate','os.setxattr','os.removexattr','shutil.copyfile','shutil.copymode','shutil.copystat','os.system','os.posix_spawn','os.posix_spawnp','os.exec'}
class ProhibitedSideEffect(BaseException):pass
def forbid(event,args):
 denied.append({'event':event,'arguments':repr(args)})
 raise ProhibitedSideEffect('read-only preflight rejected '+event)
def audit(event,args):
 if not read_only:return
 if event=='open':
  path,mode,flags=args
  if (isinstance(mode,str) and any(c in mode for c in 'wax+')) or (isinstance(flags,int) and flags&(os.O_WRONLY|os.O_RDWR|os.O_CREAT|os.O_TRUNC|os.O_APPEND)):
   forbid(event,args)
  if isinstance(path,(str,bytes)) and str(path).endswith('.py'):opened_python.add(os.fsdecode(path))
 elif event in mutations:forbid(event,args)
 elif event=='subprocess.Popen':
  exe,argv,cwd,env=args
  item={'executable':str(exe),'argv':list(argv),'cwd':str(cwd),'resolved_executable':str(git),'resolved_executable_sha256':sha(git)}
  events.append(item)
  if list(argv)!=GIT or Path(cwd).resolve()!=ROOT or Path(shutil.which(str(exe))).resolve()!=git:forbid(event,args)
sys.addaudithook(audit)
started=time.time();result=None;error=None;selection=[]
try:
 import check_expansion_campaigns as checker
 from common import read_json
 from v5_common import identity,manifest
 source=identity()['sha256']
 if source!=EXPECTED:raise ValueError('frozen source identity differs: '+source)
 for name in manifest()['new_families']:
  meta=read_json(ROOT/'labs'/name/'family.json')
  candidates=checker.candidates(name,meta)
  chosen=next((p for p in candidates if p.is_file()),None)
  selection.append({'family':name,'selected':chosen.relative_to(ROOT).as_posix() if chosen else None,'sha256':sha(chosen) if chosen else None})
 foundation=next(x for x in selection if x['family']=='foundation_depth')
 assert foundation['selected']=='reviews/v5/expansion/integrated/foundation_depth_campaign.json'
 assert foundation['sha256']=='e0041466c79e564433e5be3d3e488f45353c4dc4a61144004eb6c45284ac59c1'
 result=checker.inspect()
 assert result['input_sha256']==EXPECTED
 for row in result['families']:
  original=next(x for x in selection if x['family']==row['family'])
  if 'receipt' in row:assert (row['receipt'],row['receipt_sha256'])==(original['selected'],original['sha256'])
except BaseException:
 error=traceback.format_exc()
finally:
 read_only=False
sources={}
for name in sorted(opened_python):
 p=Path(name).resolve()
 if p.is_file() and p.is_relative_to(ROOT):sources[p.relative_to(ROOT).as_posix()]=sha(p)
envelope={'scope':'Read-only preliminary evidence inspection; not the final integrated validation or new laboratory execution.','expected_source_identity':EXPECTED,'read_only_filesystem_guard':True,'blocked_mutations':denied,'approved_subprocesses':events,'selected_receipts_before':selection,'opened_repository_python_sha256':sources,'elapsed_seconds':time.time()-started,'error':error,'inspection':result}
with (OUT/'result.json').open('x',encoding='utf8',newline='\n') as f:json.dump(envelope,f,indent=2);f.write('\n')
print(json.dumps({'passed':error is None and bool(result and result['passed']),'families':len(result['families']) if result else 0,'failures':{r['family']:r['errors'] for r in result['families'] if not r['passed']} if result else {},'required_supplements':result['required_supplements'] if result else [],'error':error,'blocked_mutations':denied,'read_only_subprocesses':events,'report':str((OUT/'result.json').relative_to(ROOT))},indent=2))
raise SystemExit(0 if error is None and result and result['passed'] else 1)
