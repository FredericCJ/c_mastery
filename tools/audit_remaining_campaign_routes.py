"""Read-only exact candidate inspection for the remaining integrated routes."""
import argparse,json
from common import ROOT,sha,dump
from check_expansion_campaigns import pin_errors,execution_errors,generic_errors,special_errors,SPECIAL
BASE='reviews/v5/expansion/'
CHOICES={
 'adt':['b07/adt_campaign.json'],
 'api_depth':['integrated/api_depth_campaign.json'],
 'checkpoint_depth':['legacy_completion/checkpoint_depth_campaign.json'],
 'concurrency_lifecycle_depth':['legacy_concurrency_lifecycle/campaign.json'],
 'coroutines':['integrated/coroutines_campaign.json'],
 'dispatch':['modern_completion/dispatch_campaign.json'],
 'dod':['modern_completion/dod_campaign.json'],
 'errors':['b07/errors_campaign.json'],
 'foundation_depth':['foundation_final/20261018/foundation_depth_campaign.json'],
 'higher_order':['modern_completion/higher_order_campaign.json'],
 'messaging':['modern_completion/messaging_current_campaign.json'],
 'metaprog':['b07/metaprog_campaign.json'],
 'modern_composition':['modern_completion/modern_composition_campaign.json'],
 'object_runtime':['modern_completion/object_runtime_current_campaign.json','modern_followthrough/object_runtime_current_01.json'],
 'ownership':['b07/ownership_campaign.json'],
 'posix_lifecycle_depth':['integrated/posix_lifecycle_depth_campaign.json'],
 'protocols':['integrated/protocols_campaign.json'],
 'replicated':['b07/replicated_campaign.json'],
 'resilience':['integrated/resilience_campaign.json']}
def main():
    p=argparse.ArgumentParser();p.add_argument('--output',default=BASE+'followthrough/remaining_campaign_route_audit.json');a=p.parse_args();rows=[]
    for family,choices in CHOICES.items():
        meta=json.loads((ROOT/'labs'/family/'family.json').read_text())
        for name in choices:
            path=BASE+name
            if not (ROOT/path).is_file():rows.append(dict(family=family,path=path,errors=['missing candidate']));continue
            data=json.loads((ROOT/path).read_text());errors=pin_errors(data)+execution_errors(data)
            if data.get('passed') is not True:errors.append('candidate not passed')
            if data.get('source_changed_during_run') is True or data.get('scope_source_stable') is False:errors.append('candidate scope changed during run')
            if family=='foundation_depth':errors.append('whole-identity final foundation route held for final source freeze')
            elif family in SPECIAL and family!='object_runtime':errors+=special_errors(family,meta,data)
            else:errors+=generic_errors(family,meta,data)
            row=dict(family=family,path=path,sha256=sha(ROOT/path),passed=not errors,errors=errors,
                producer=(meta.get('campaign') or {}).get('producer'),
                schema_note='generic current producer; obsolete SPECIAL path must be corrected' if family=='object_runtime' else None)
            rows.append(row);print(family,name,'PASS' if not errors else 'FAIL '+str(len(errors)),errors[:2],flush=True)
    dump(ROOT/a.output,dict(scope='Read-only exact current candidates; no source hashes or receipts changed.',rows=rows))
if __name__=='__main__':main()
