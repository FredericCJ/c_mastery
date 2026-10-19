"""Validate supplied QNX8 probe logs; never manufacture execution from a build."""
from pathlib import Path
import argparse,hashlib,json,re
def validate(build,probe,manager,exit_code,identity):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 need(build.get('passed') is True and build.get('status')=='built-not-executed','actual successful SDK build required')
 need(bool(build.get('source_sha256')) and set(Path(p).name for p in build.get('outputs_sha256',{}))=={'probe','manager'},'source/binary identities required')
 need(identity.get('os')=='QNX' and str(identity.get('release','')).startswith('8.'),'selected QNX8 target required')
 need(all(identity.get(k) for k in ['image_sha256','sdk_packages','compiler_version','guest_command','probe_sha256','manager_sha256']),'target/provisioning/binary provenance required')
 for name in ['probe','manager']:
  expected=next((h for p,h in build.get('outputs_sha256',{}).items() if Path(p).name==name),None)
  need(expected is not None and expected==identity.get(name+'_sha256'),'transferred binary differs: '+name)
 need(exit_code==0,'probe exit must succeed')
 first=re.search(r'^QC_QNX_FIRST rc=1 error=[1-9][0-9]* eintr=1 retained=1 calls=1 live=1 same=1$',probe,re.M)
 final=re.search(r'^QC_QNX_FINAL rc=0 state=2 calls=2 ebadf=1$',probe,re.M)
 target=re.search(r'^QC_QNX_ID sysname=QNX release=8\.[^ ]+ machine=.+$',probe,re.M)
 need(first is not None,'first actual retained EINTR observation required')
 need(final is not None,'second actual success/retirement observation required')
 need(target is not None,'target self-identification required')
 need(first is not None and final is not None and target is not None and first.start()<final.start()<target.start(),'ordered target observations required')
 need('QC_QNX_OK:resource-manager-error-path' in probe,'complete actual probe marker required')
 need(manager.count('QC_MANAGER:injected-EINTR-before-default')==1 and 'QC_MANAGER:default-close' in manager,'original controlled manager trace required')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--build',type=Path,required=True);p.add_argument('--probe-log',type=Path,required=True);p.add_argument('--manager-log',type=Path,required=True);p.add_argument('--identity',type=Path,required=True);p.add_argument('--exit-code',type=int,required=True);p.add_argument('--output',type=Path,required=True);a=p.parse_args()
 build=json.loads(a.build.read_text());ident=json.loads(a.identity.read_text());errors=validate(build,a.probe_log.read_text(),a.manager_log.read_text(),a.exit_code,ident)
 pins={str(f.resolve()):hashlib.sha256(f.read_bytes()).hexdigest() for f in [a.build,a.probe_log,a.manager_log,a.identity,Path(__file__)]}
 a.output.write_text(json.dumps(dict(passed=not errors,errors=errors,input_sha256=pins,scope='Supplied QNX8 actual-libc controlled resource-manager EINTR path; not asynchronous signal interruption',independent_review='not-established'),indent=2)+'\n');print({'passed':not errors,'errors':errors});return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
