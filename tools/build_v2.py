"""Build the second edition and bind each actual check to its current inputs."""
from __future__ import annotations
from datetime import datetime, timezone
import subprocess
import sys
from common import ROOT, dump, read_json
from v2_common import source_identity

def main():
    # Reconciliation steps precede check identities because they generate active
    # canonical metadata. No legacy receipt is silently carried into v2.
    for name in ['integrate_v2.py','write_program.py']:
        result=subprocess.run([sys.executable,'-X','utf8',str(ROOT/'tools'/name)],cwd=ROOT)
        if result.returncode: raise SystemExit(result.returncode)
    steps=['validate.py','check_fixtures.py','check_contract_guards.py','check_v2.py',
           'publish.py','check_html.py','check_markdown.py']
    records=[]
    for name in steps:
        identity=source_identity()
        result=subprocess.run([sys.executable,'-X','utf8',str(ROOT/'tools'/name)],cwd=ROOT)
        current=source_identity()
        # validate/publish may regenerate deterministic coverage metadata. Bind
        # their receipt to the resulting source state; a later step changing it
        # makes the overall receipt stale instead of preserving an old success.
        record={'check':name,'exit_code':result.returncode,
                'status':'passed' if result.returncode==0 else 'failed',
                'input_sha256':current['sha256'],'started_input_sha256':identity['sha256']}
        records.append(record)
        dump(ROOT/'reviews/v2'/('check_'+name.removesuffix('.py')+'.json'),record)
        if result.returncode:
            dump(ROOT/'reviews/v2/publication.json',{'status':'failed','passed':False,'checks':records})
            raise SystemExit(result.returncode)
    final=source_identity()
    stale=[r['check'] for r in records if r['input_sha256']!=final['sha256']]
    report={'schema_version':2,'status':'passed' if not stale else 'failed','passed':not stale,
        'recorded_at':datetime.now(timezone.utc).isoformat(),'input_sha256':final['sha256'],
        'checks':records,'stale_checks':stale,
        'scope':'Source contracts, migration, fixture oracles and publication; actual C and target execution have separate receipts.'}
    dump(ROOT/'reviews/v2/publication.json',report)
    dump(ROOT/'reviews/v2/source_manifest.json',final)
    if stale: raise SystemExit('Checks became stale: '+str(stale))

if __name__=='__main__': main()
