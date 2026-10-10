"""Build current v4 sources while preserving historical v2/v3 receipts."""
from datetime import datetime, timezone
import subprocess
import sys
from common import ROOT, dump
from v2_common import source_identity


def main():
    historical={p:p.read_bytes() for folder in ['reviews/v2','reviews/v3'] for p in (ROOT/folder).rglob('*') if p.is_file()}
    try:
        subprocess.run([sys.executable,'-X','utf8',str(ROOT/'tools/author_v4.py')],cwd=ROOT,check=True)
        from publish_v4 import prepare
        prepare()  # Materialize coverage before binding the checks to their inputs.
        started=source_identity();records=[]
        for name in ['validate.py','check_fixtures.py','check_contract_guards.py','check_v3.py','check_v4.py',
                     'check_learning_v4.py','check_release_v4.py','publish_v4.py','check_html.py','check_markdown.py']:
            before=source_identity()
            run=subprocess.run([sys.executable,'-X','utf8',str(ROOT/'tools'/name)],cwd=ROOT)
            after=source_identity()
            record={'check':name,'status':'passed' if not run.returncode else 'failed','exit_code':run.returncode,
                'started_input_sha256':before['sha256'],'input_sha256':after['sha256']}
            records.append(record);dump(ROOT/'reviews/v4'/('check_'+name.removesuffix('.py')+'.json'),record)
            if run.returncode:
                dump(ROOT/'reviews/v4/publication.json',{'status':'failed','passed':False,'checks':records})
                raise SystemExit(run.returncode)
        final=source_identity()
        stale=[r['check'] for r in records if r['input_sha256']!=final['sha256'] or r['started_input_sha256']!=final['sha256']]
        report={'schema_version':4,'status':'passed' if not stale else 'failed','passed':not stale,
            'input_sha256':final['sha256'],'recorded_at':datetime.now(timezone.utc).isoformat(),
            'checks':records,'stale_checks':stale,'scope':'Current v4 publication, preserved teaching, curriculum structure and local learner workflow.'}
        dump(ROOT/'reviews/v4/publication.json',report);dump(ROOT/'reviews/v4/source_manifest.json',final)
        if stale:raise SystemExit('Source changed while validating: '+str(stale))
    finally:
        for path,content in historical.items():path.write_bytes(content)


if __name__=='__main__':main()
