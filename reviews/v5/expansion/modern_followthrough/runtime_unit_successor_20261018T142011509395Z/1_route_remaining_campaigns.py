"""Validate and byte-copy current scoped campaigns to their designated routes."""
import argparse,json
from datetime import datetime,timezone
from common import ROOT,sha,dump
from check_expansion_campaigns import inspect_family,pin_errors,execution_errors,generic_errors,object_runtime_errors
SOURCES={
 'checkpoint_depth':'legacy_completion/checkpoint_depth_campaign.json',
 'concurrency_lifecycle_depth':'legacy_concurrency_lifecycle/campaign.json',
 'dod':'modern_completion/dod_campaign.json',
 'dispatch':'modern_completion/dispatch_campaign.json',
 'higher_order':'modern_completion/higher_order_campaign.json',
 'messaging':'modern_completion/messaging_current_campaign.json',
 'modern_composition':'modern_completion/modern_composition_campaign.json',
 'object_runtime':'modern_followthrough/object_runtime_current_01.json',
 'errors':'followthrough/rerun_remaining_20261018T140501401459Z/errors_campaign.json',
 'ownership':'ownership_current/final/ownership_campaign.json',
 'corpus_treiber':'corpus_treiber/current_validation.json',
 'corpus_escape':'corpus_escape/corpus_escape_campaign.json'}
def main():
    p=argparse.ArgumentParser();p.add_argument('--family',choices=SOURCES);a=p.parse_args()
    stamp=datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');base=ROOT/'reviews/v5/expansion/followthrough'/('routed_remaining_'+stamp);base.mkdir()
    rows=[]
    for family,tail in SOURCES.items():
        if a.family and family!=a.family:continue
        source=ROOT/'reviews/v5/expansion'/tail;data=json.loads(source.read_text());meta=json.loads((ROOT/'labs'/family/'family.json').read_text())
        errors=pin_errors(data)+execution_errors(data)
        if family in {'ownership','errors'}:
            import importlib
            importlib.import_module('check_'+family+'_current').validate(data,files=True)
        else:errors+=object_runtime_errors(meta,data) if family=='object_runtime' else generic_errors(family,meta,data)
        assert data.get('passed') is True and data.get('scope_source_stable') is not False and not errors,(family,errors)
        destination=ROOT/'reviews/v5/expansion/integrated'/meta['campaign'].get('receipt',family+'_campaign.json');source_hash=sha(source);old=None
        if destination.exists():
            old=sha(destination);backup=base/destination.name;backup.write_bytes(destination.read_bytes());assert sha(backup)==old
        destination.write_bytes(source.read_bytes());assert sha(destination)==source_hash
        current=inspect_family(family);assert current['passed'],current
        row=dict(family=family,source=source.relative_to(ROOT).as_posix(),source_sha256=source_hash,
            destination=destination.relative_to(ROOT).as_posix(),destination_sha256=sha(destination),previous_sha256=old,
            mode='exact byte copy of source-current observation; no new execution',inspection=current)
        rows.append(row);dump(base/'result.json',dict(passed=True,routes=rows));print('ROUTE_PASS',family,flush=True)
    summary=ROOT/'reviews/v5/expansion/followthrough/remaining_routing_current.json'
    previous=json.loads(summary.read_text()) if summary.is_file() else {}
    retained=[r for r in previous.get('routes',[]) if r['family'] not in {x['family'] for x in rows}]
    dump(summary,dict(passed=True,routes=retained+rows,preservation=base.relative_to(ROOT).as_posix()))
if __name__=='__main__':main()
