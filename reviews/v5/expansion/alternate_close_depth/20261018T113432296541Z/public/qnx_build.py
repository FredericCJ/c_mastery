"""SDK-only build capture. No host-header substitutes, download or guest execution."""
from pathlib import Path
import argparse,datetime,hashlib,json,os,shutil,subprocess
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=Path,required=True);p.add_argument('--out',type=Path,required=True);p.add_argument('--qcc',default='qcc');a=p.parse_args()
 here=Path(__file__).resolve().parent;out=a.out.resolve();out.mkdir(parents=True,exist_ok=True)
 sha=lambda path:hashlib.sha256(path.read_bytes()).hexdigest()
 targets={'probe':[here/'qnx_probe.c',a.source.resolve()],'manager':[here/'qnx_manager.c'],'signal_probe':[here/'qnx_signal_probe.c',a.source.resolve()],'signal_manager':[here/'qnx_signal_manager.c']}
 sources=[here/n for n in ['close_owner.h','qnx_probe.c','qnx_manager.c','qnx_signal_probe.c','qnx_signal_manager.c','qnx_build.py']]+[a.source.resolve()]
 report=dict(scope='Actual QNX SDP8 target compilation only; no target execution',started_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),source_sha256={str(p):sha(p) for p in sources},commands=[],passed=False,status='not-built',target_execution='not-run')
 qcc=shutil.which(a.qcc)
 try:
  if not qcc:raise RuntimeError('QNX SDP8 qcc unavailable; valid myQNX license/deployed SDK and target image required')
  for key in ['QNX_HOST','QNX_TARGET']:
   if not os.environ.get(key):raise RuntimeError(key+' unset: source the authorized SDP8 environment first')
  report['environment']={k:os.environ[k] for k in ['QNX_HOST','QNX_TARGET']}
  def run(argv):
   r=subprocess.run(argv,text=True,capture_output=True,timeout=120);report['commands'].append(dict(argv=argv,exit_code=r.returncode,stdout=r.stdout,stderr=r.stderr))
   if r.returncode:raise RuntimeError('Target build failed: '+str(argv))
  run([qcc,'--version']);run([qcc,'-V'])
  for name,inputs in targets.items():
   run([qcc,'-Vgcc_ntox86_64','-std=c17','-D_QNX_SOURCE','-O2','-Wall','-Wextra','-Werror','-I'+str(here),*[str(p) for p in inputs],'-o',str(out/name)])
  report['outputs_sha256']={str(out/n):sha(out/n) for n in targets};report.update(passed=True,status='built-not-executed')
 except (RuntimeError,OSError,subprocess.TimeoutExpired) as error:report['blocker']=str(error)
 report['finished_utc']=datetime.datetime.now(datetime.timezone.utc).isoformat();(out/'build.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps({'status':report['status'],'passed':report['passed'],'blocker':report.get('blocker')}));return 0 if report['passed'] else 2
if __name__=='__main__':raise SystemExit(main())
