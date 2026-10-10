"""Reconcile and publish v3 without overwriting historical v2 release receipts."""
from datetime import datetime, timezone
import subprocess
import sys
from common import ROOT, dump
from v2_common import source_identity
from publish import prepare

def main():
    for name in ['integrate_v3.py','write_program.py']:
        subprocess.run([sys.executable,'-X','utf8',str(ROOT/'tools'/name)],cwd=ROOT,check=True)
    # Coverage is a generated canonical input. Materialize it before binding
    # validation to one source identity, so an edited objective builds in one run.
    prepare()
    steps=['validate.py','check_fixtures.py','check_contract_guards.py','check_v3.py','check_release_v3.py',
           'publish.py','check_html.py','check_markdown.py']
    records=[]
    for name in steps:
        before=source_identity()
        result=subprocess.run([sys.executable,'-X','utf8',str(ROOT/'tools'/name)],cwd=ROOT)
        identity=source_identity()
        record={'check':name,'exit_code':result.returncode,'status':'passed' if not result.returncode else 'failed',
                'input_sha256':identity['sha256'],'started_input_sha256':before['sha256']}
        records.append(record); dump(ROOT/'reviews/v3'/('check_'+name.removesuffix('.py')+'.json'),record)
        if result.returncode:
            dump(ROOT/'reviews/v3/publication.json',{'status':'failed','checks':records})
            raise SystemExit(result.returncode)
    identity=source_identity(); stale=[r['check'] for r in records
        if r['input_sha256']!=identity['sha256'] or r['started_input_sha256']!=r['input_sha256']]
    dump(ROOT/'reviews/v3/publication.json',{'status':'passed' if not stale else 'failed','passed':not stale,
        'input_sha256':identity['sha256'],'recorded_at':datetime.now(timezone.utc).isoformat(),
        'checks':records,'stale_checks':stale,'scope':'Current publication, source, fixture and migration checks.'})
    dump(ROOT/'reviews/v3/source_manifest.json',identity)
    if stale: raise SystemExit('Source changed during publication checks: '+str(stale))
if __name__=='__main__': main()
