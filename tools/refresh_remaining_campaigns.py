"""Run only selected stale family producers, preserving old designated bytes."""
import argparse,json,subprocess,sys
from datetime import datetime,timezone
from common import ROOT,sha,dump
from check_expansion_campaigns import inspect_family,pin_errors,execution_errors,generic_errors,special_errors,SPECIAL
FAMILIES=['api_depth','adt','errors','ownership','metaprog','protocols','coroutines','replicated','resilience','posix_lifecycle_depth']
def main():
    p=argparse.ArgumentParser();p.add_argument('families',nargs='+',choices=FAMILIES);a=p.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/followthrough'/('rerun_remaining_'+stamp);base.mkdir()
    rows=[]
    for family in a.families:
        meta=json.loads((ROOT/'labs'/family/'family.json').read_text());producer=meta['campaign']['producer']
        target=base/(family+'_campaign.json');cmd=[sys.executable,producer,'--report',str(target)]
        if family in {'protocols','coroutines'}:cmd+=['--build',str(ROOT/'build'/('final-'+family+'-'+stamp))]
        producer_hash=sha(ROOT/producer)
        print('RUN_START',family,flush=True)
        result=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=5400)
        log=base/(family+'.log');log.write_text(result.stdout+'\n'+result.stderr,encoding='utf-8',newline='\n')
        row=dict(family=family,command=cmd,exit_code=result.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),producer_sha256=producer_hash)
        rows.append(row)
        if result.returncode or not target.is_file():
            row['passed']=False;dump(base/'result.json',dict(passed=False,rows=rows));print('RUN_FAIL',family,result.stdout[-1500:],result.stderr[-1500:],flush=True);return 1
        report=json.loads(target.read_text());errors=pin_errors(report)+execution_errors(report)
        if family in {'ownership','errors','metaprog','replicated'}:
            import importlib
            try:importlib.import_module('check_'+family+'_current').validate(report,files=True)
            except (AssertionError,KeyError,ValueError,IndexError) as error:errors.append('Complete current contract: '+str(error))
        else:errors+=special_errors(family,meta,report) if family in SPECIAL else generic_errors(family,meta,report)
        assert sha(ROOT/producer)==producer_hash,'Producer changed during execution'
        if report.get('passed') is not True or report.get('scope_source_stable') is False or report.get('source_changed_during_run') is True:errors.append('Failed or unstable source campaign')
        row['errors']=errors
        if errors:row['passed']=False;dump(base/'result.json',dict(passed=False,rows=rows));print('SCOPE_FAIL',family,errors,flush=True);return 1
        destination=ROOT/'reviews/v5/expansion/integrated'/meta['campaign']['receipt']
        if destination.exists():
            backup=base/(family+'_selected_before.json');backup.write_bytes(destination.read_bytes());row['previous_sha256']=sha(backup)
        destination.write_bytes(target.read_bytes());row['campaign_sha256']=sha(target)
        row['source_report']=target.relative_to(ROOT).as_posix();row['selected_report']=destination.relative_to(ROOT).as_posix()
        row['inspection']=inspect_family(family);row['passed']=row['inspection']['passed']
        dump(base/'result.json',dict(passed=all(r['passed'] for r in rows),rows=rows))
        print('RUN_PASS' if row['passed'] else 'ROUTE_FAIL',family,flush=True)
        if not row['passed']:return 1
    return 0
if __name__=='__main__':raise SystemExit(main())
