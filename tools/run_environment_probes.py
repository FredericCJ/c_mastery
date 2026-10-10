"""Record actual Linux diagnostic availability without treating absence as a pass."""
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery.cli import instantiate, run_checks, invoke
from common import ROOT, dump
from v2_common import source_identity

def main():
    p=argparse.ArgumentParser(description=__doc__)
    p.add_argument('--native',action='store_true')
    p.add_argument('--cgroup-parent',default='/sys/fs/cgroup')
    args=p.parse_args()
    if os.name=='nt' and not args.native:
        path=subprocess.check_output(['wsl','--exec','wslpath','-a',str(ROOT)],text=True).strip()
        raise SystemExit(subprocess.run(['wsl','--exec','python3',path+'/tools/run_environment_probes.py',
            '--native','--cgroup-parent',args.cgroup_parent],cwd=ROOT).returncode)
    identity=source_identity(); base=ROOT/'build/environment_probes'; base.mkdir(parents=True,exist_ok=True)
    record={'status':'observed','input_sha256':identity['sha256'],
        'recorded_at':datetime.now(timezone.utc).isoformat(),
        'scope':'Bounded process-local policy attempts, delegated child cgroup attempt and actual TSan runtime probe; unavailable experiments remain open.'}
    record['context']=invoke([sys.executable,str(ROOT/'labs/os_observatory/observe.py')])
    with tempfile.TemporaryDirectory(prefix='probe ',dir=base) as tmp:
        folder=Path(tmp); binary=folder/'policy'
        build=invoke(['gcc','-std=c17','-O2','-Wall','-Wextra','-Werror',
                      str(ROOT/'labs/os_observatory/policy_probe.c'),'-o',str(binary)])
        record['policy_build']=build
        if build['exit_code']==0:
            record['policies']={policy:invoke([str(binary),policy],timeout=5)
                                for policy in ['other','fifo','rr','deadline']}
        record['cgroup']=invoke([sys.executable,str(ROOT/'labs/os_observatory/cgroup_run.py'),
            '--parent',args.cgroup_parent,'--timeout','1','--',sys.executable,'-c',
            'x=0\nfor i in range(200000): x+=i\nprint(x)'],timeout=10)
        workspace=folder/'learner'
        instantiate('PR-T49-02',workspace,family_name='os_observatory')
        record['thread_sanitizer']=run_checks(workspace,'gcc','tsan',True,90)
        concurrent=folder/'reclamation'
        instantiate('PR-T55-06',concurrent,family_name='execution')
        record['reclamation_thread_sanitizer']=run_checks(concurrent,'gcc','tsan',True,120)
    record['source_changed_during_run']=source_identity()['sha256']!=identity['sha256']
    dump(ROOT/'reviews/v3/environment_probes.json',record)
    print(json.dumps({'status':'observed','policies':{k:v['exit_code'] for k,v in record.get('policies',{}).items()},
        'cgroup_exit':record['cgroup']['exit_code'],'thread_sanitizer':record['thread_sanitizer']['status'],
        'reclamation_thread_sanitizer':record['reclamation_thread_sanitizer']['status']},indent=2))

if __name__=='__main__': main()
