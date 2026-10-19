"""Execute the current declared messaging campaign and pin its explicit adapters."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from run_expansion_family import run

ROOT = Path(__file__).resolve().parents[1]


def main():
    parser=argparse.ArgumentParser()
    parser.add_argument('--report',default='reviews/v5/expansion/modern_completion/messaging_followthrough_campaign.json')
    args=parser.parse_args()
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_messaging.py','--report',args.report,platform='wsl'),cwd=ROOT).returncode
    from register_messaging_followthrough import ROUTES
    extra=['tools/run_messaging.py','tools/register_messaging_followthrough.py','tools/verify_messaging_fixtures.py']
    extra+=['tools/verify_messaging_registration.py','tools/verify_messaging_actor_executor.py','tools/verify_messaging_completion_winner.py','tools/run_messaging_cost.py']
    extra+=['curriculum/v5/exercises/'+exercise+'.md' for exercise in ROUTES]
    extra+=['research/v5/modern/messaging_resources.json','research/v5/modern/messaging_research_gate.json']
    extra+=['research/v5/modern/messaging_model_resources.json','research/v5/modern/messaging_model_research_gate.json']
    extra+=['research/v5/modern/messaging_versions_resources.json','research/v5/modern/messaging_versions_research_gate.json']
    extra+=['research/v5/modern/messaging_cost_resources.json','research/v5/modern/messaging_cost_research_gate.json']
    sha=lambda p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
    before={p:sha(p) for p in extra}
    code=run('messaging',args.report)
    report=(ROOT/args.report).resolve()
    data=json.loads(report.read_text(encoding='utf8'))
    stable=before=={p:sha(p) for p in extra}
    data['inspected_paths'].update(before)
    data['scope_input_sha256']=hashlib.sha256(json.dumps(data['inspected_paths'],sort_keys=True).encode()).hexdigest()
    data['scope_source_stable']=data['scope_source_stable'] and stable
    data['passed']=data['passed'] and stable and code==0
    data['status']='passed' if data['passed'] else 'failed'
    data['producer']='tools/run_messaging.py'
    data['scope']='Declared original cores plus actual nested nursery, inline owner-free chain, retained concurrent completion winner, owned channel/actor successors and channel-to-actor executor, separately locked registered select, fallible two-level supervision, weaker-order native clients and retained pointer payload/array lifetimes, fallible owned versions and predicate/write-conflict validation, staged transaction effects, retained allocated nodes and commit-phase fault injection, and executor-serialized demand/channel signals with quiescent context release. Equal-work cost assertions are correctness checks; uninstrumented timing and RC11 model campaigns are separate evidence. Per-rubric readiness is assessed separately.'
    report.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
    return 0 if data['passed'] else 1


if __name__=='__main__':raise SystemExit(main())
