"""Execute Cortex-M target gates and actual x86 guests at 1/2/4 CPUs."""
from datetime import datetime, timezone
import argparse
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from cmastery.cli import instantiate, run_checks, doctor
from common import ROOT, dump
from v2_common import source_identity

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--native',action='store_true')
    args=parser.parse_args()
    if os.name=='nt' and not args.native:
        mapping=subprocess.check_output(['wsl','--exec','wslpath','-a',str(ROOT)],text=True).strip()
        raise SystemExit(subprocess.run(['wsl','--exec','python3',mapping+'/tools/run_target_matrix.py','--native'],cwd=ROOT).returncode)
    identity=source_identity(); results=[]
    base=ROOT/'build/target_matrix'; base.mkdir(parents=True,exist_ok=True)
    for family,project,target,cpus in [
        ('execution','PR-T50-01','cortex-m4-qemu',None),
        ('execution','PR-T50-01','stm32f439',None),
        ('execution','PR-T54-01','stm32h745',None),
        *[('kernel_smp','PR-T53-01','x86-smp',n) for n in [1,2,4]]]:
        with tempfile.TemporaryDirectory(prefix=family+' ',dir=base) as directory:
            workspace=Path(directory)/'learner'
            instantiate(project,workspace,family_name=family)
            record=run_checks(workspace,'gcc','debug',True,600,all_tests=True,target=target,cpus=cpus)
            results.append(record)
            dump(ROOT/'reviews/v3'/f'target_{target}_{cpus or "default"}.json',record)
            print(f'{family}/{target}/{cpus}: {record["status"]}',flush=True)
    changed=source_identity()['sha256']!=identity['sha256']
    passed=not changed and len(results)==6 and all(r['status']=='passed' for r in results)
    report={'schema_version':3,'status':'passed' if passed else 'failed','passed':passed,
        'input_sha256':identity['sha256'],'source_changed_during_run':changed,
        'recorded_at':datetime.now(timezone.utc).isoformat(),'environment':doctor(),
        'targets':[{'family':r['family'],'target':r['target'],'cpus':r['cpu_count'],'status':r['status'],
                    'tests':[t['name'] for t in r.get('tests',[])],'evidence_kind':r['evidence_kind']} for r in results],
        'physical_observations':'not-run',
        'scope':'Actual guest execution and strict target image audits. Neither image compilation nor QEMU execution establishes physical MCU timing/cache/DMA behavior.'}
    dump(ROOT/'reviews/v3/target_matrix.json',report)
    print(json.dumps(report,indent=2)); return 0 if passed else 1

if __name__=='__main__': raise SystemExit(main())
