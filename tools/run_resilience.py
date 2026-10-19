"""Tracked hosted producer for the T75 resilience family and explicit successors."""
import argparse
import hashlib
import json
import os
from pathlib import Path
import subprocess
from run_resilience_scoped import run

ROOT=Path(__file__).resolve().parents[1]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--report',required=True)
    args=parser.parse_args()
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/run_resilience.py','--report',args.report,platform='wsl'),cwd=ROOT).returncode
    from register_resilience_followthrough import ROUTES
    extra=['tools/run_resilience.py','tools/register_resilience_followthrough.py','tools/verify_resilience_timers.py','tools/verify_resilience_retry.py','tools/verify_resilience_pressure.py','tools/verify_resilience_breaker.py','tools/verify_resilience_hedge.py','tools/verify_resilience_journal.py',
           'tools/verify_resilience_outbox.py','tools/verify_resilience_service_io.py','tools/verify_resilience_composition.py',
           'research/v5/modern/resilience_followthrough_sources.md',
           'research/v5/modern/resilience_resources.json','research/v5/modern/resilience_research_gate.json']
    extra+=['curriculum/v5/exercises/'+exercise+'.md' for exercise in ROUTES]
    return run(args.report,extra)

if __name__ == '__main__':raise SystemExit(main())
