"""Run actual assessor and unsolved-scaffold gates; record their limited scope."""
from __future__ import annotations
import argparse
from datetime import datetime, timezone
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
import time
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery.cli import instantiate, run_checks, families, doctor, save
from v2_common import source_identity
from common import ROOT

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--family',action='append')
    parser.add_argument('--native',action='store_true')
    args=parser.parse_args()
    if os.name=='nt' and not args.native:
        mapping=subprocess.run(['wsl','--exec','wslpath','-a',str(ROOT)],capture_output=True,text=True,check=True).stdout.strip()
        command=['wsl','--exec','python3',mapping+'/tools/run_lab_matrix.py','--native']
        for family in args.family or []: command+=['--family',family]
        raise SystemExit(subprocess.run(command,cwd=ROOT).returncode)
    all_families=families(); selected=args.family or list(all_families)
    unknown=set(selected)-set(all_families)
    if unknown: raise ValueError('Unknown families: '+str(sorted(unknown)))
    identity=source_identity(); results=[]; started=time.monotonic()
    edition='v3' if (ROOT/'curriculum/baseline_v2.json').exists() else 'v2'
    for family in selected:
        manifest=all_families[family]['meta']; entries=manifest.get('projects',{})
        if isinstance(entries,dict): project=next(iter(entries))
        else:
            first=entries[0];project=first if isinstance(first,str) else first['id']
        # Native temp storage also exercises arbitrary paths containing spaces.
        with tempfile.TemporaryDirectory(prefix='c mastery '+family+' ') as directory:
            workspace=Path(directory)/'learner'
            instantiate(project,workspace,family_name=family)
            # A representative unsolved milestone must compile and reject.
            # Running every later guest milestone against an empty kernel only
            # accumulates deadlines and obscures this scaffold check.
            learner=run_checks(workspace,'gcc','debug',False,240,all_tests=False)
            steps=learner['steps']
            rejects=(learner['status']=='failed' and len(steps)==3 and
                     steps[0]['exit_code']==0 and steps[1]['exit_code']==0 and
                     steps[2]['exit_code'] not in (None,0) and not steps[2].get('timed_out'))
            checks=[]
            for compiler,profile in [('gcc','debug'),('gcc','release'),('clang','debug'),('clang','asan')]:
                result=run_checks(workspace,compiler,profile,True,300,all_tests=True)
                checks.append(result)
                print(f'{family} {compiler}/{profile}: {result["status"]}',flush=True)
            result={'family':family,'representative_project':project,
                'learner_scaffold_compiles_and_rejects_unimplemented':rejects,
                'learner_attempt':learner,'checks':checks,
                'passed':rejects and all(c['status']=='passed' for c in checks),
                'scope':'Actual complete-family host suites; per-objective reasoning, target and physical evidence remain separate.'}
            results.append(result)
            save(ROOT/'reviews'/edition/('lab_'+family+'.json'),result)
    current=source_identity()
    passed=all(r['passed'] for r in results) and current['sha256']==identity['sha256']
    record={'schema_version':2,'status':'passed' if passed else 'failed','passed':passed,
        'recorded_at':datetime.now(timezone.utc).isoformat(),'input_sha256':identity['sha256'],
        'source_changed_during_run':current['sha256']!=identity['sha256'],
        'elapsed_seconds':round(time.monotonic()-started,3),'environment':doctor(),
        'families':[{'family':r['family'],'passed':r['passed']} for r in results],
        'physical_observations':'not-run','learner_time_calibration':'not-run'}
    suffix='lab_matrix' if not args.family else 'lab_matrix_'+'_'.join(selected)
    save(ROOT/'reviews'/edition/(suffix+'.json'),record)
    print(json.dumps(record,indent=2))
    return 0 if passed else 1

if __name__=='__main__': raise SystemExit(main())
