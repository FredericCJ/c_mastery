"""Strict read-only origin/child/transitive receipt guard for the81 IW aliases."""
import argparse,copy,datetime,hashlib,json,pathlib,sys
import audit_inventory_aliases as a
import build_inventory_aliases as b
import check_residual_followthrough as transitive
ROOT=a.ROOT;BASE=a.BASE
def require(ok,message):
    if not ok:raise ValueError(message)
def expected():
    inspection=a.read(BASE/'inspection_input.json');policy=a.read(BASE/'clause_map.json');require(len(inspection['records'])==81 and not inspection['missing_provider_ids'],'exact original intake')
    for row in inspection['records']:
        require(len(row['origins'])==1,'one exact content origin')
        o=row['origins'][0];identity='IW-'+hashlib.sha256((o['path']+'\n'+json.dumps(o['record'],sort_keys=True,ensure_ascii=False)).encode()).hexdigest()[:16];require(identity==row['id'] and a.original(o)==o['record'],'original content identity')
    rows,bindings,sources,providers=b.make_rows(inspection,policy);mapping=dict(schema_version=1,scope='Exact original parent bodies, complete canonical ordinary/delayed child selection and actual versioned provider records',bindings=bindings);sources.add((BASE/'evidence_map.json').relative_to(ROOT).as_posix())
    return rows,mapping,{p:a.sha(ROOT/p) for p in sorted(sources)},sorted(providers)
def inspect(support,mapping,audit,gold):
    rows,wanted,pins,providers=gold
    require(support['updates']==rows and len(rows)==81,'exact parent dimensions, limits, tasks and child sets')
    require(mapping==wanted,'exact origin/criterion/variant/provider binding')
    require(support['source_sha256']==pins,'exact current source pins')
    require(support['providers']==providers==audit['providers'],'provider selection')
    require(set(pins).isdisjoint({'curriculum/v5/expansion/work_inventory.json','curriculum/v5/expansion/implementation_updates.json'}),'mutable global register pinned')
    require(sum(bool(x['required_criteria_and_variants']) for x in rows)==80,'exact80 author parents and one source attempt')
    require(len({c['id'] for r in mapping['bindings'] for c in r['children']})==406,'complete ordinary/delayed canonical selection')
    require(not audit['missing'] and not audit['stale'],'missing/stale transitive evidence')
    require(audit.get('external_validation',{}).get('passed') is True,'external identities not validated')
    require(audit['documents']>=85 and audit['files']>=11757,'full transitive inputs')
    require(not any(v.get('passed') is False or v.get('status') in ['failed','fail','not-run'] or v.get('source_stable') is False or v.get('scope_source_stable') is False for v in audit['report_statuses'].values()),'failed or unstable referenced receipt')
    require(support.get('remaining_external_work')==b.REMAINING,'remaining source/physical/independent clauses')
    for row in rows:
        if row['classification']=='unavailable-reference-attempt':
            require(not row['required_criteria_and_variants'] and row['dimensions']['teaching']['state']=='reference-gap-retained;not-inspected','failed capture promoted to inspection')
        else:
            require(row['dimensions']['external_review']['state']=='not-run' and row['dimensions']['independent_learner']['state']=='not-assessed' and row['dimensions']['physical_observation']['state']=='not-claimed;original-required-physical-work-retained','invented independent or physical evidence')
    return dict(parents=81,author_parents=80,failed_reference_attempts=1,children=406,providers=len(providers))
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);args=p.parse_args();destination=(ROOT/args.report).resolve();require(destination.is_relative_to((ROOT/'reviews/v5/expansion/inventory_alias_followthrough').resolve()),'scoped review path');require(not destination.exists(),'preserve original review')
    support=a.read(BASE/'support.json');mapping=a.read(BASE/'evidence_map.json');audit=transitive.evidence_audit(support['providers']);errors=[];controls=[];summary={}
    try:
        gold=expected();audit['external_validation']=transitive.external_check(audit['external']);summary=inspect(support,mapping,audit,gold)
        require(all(a.sha(ROOT/p)==h for p,h in audit['document_sha256'].items()),'child envelopes changed during guard')
        if args.self_test:
            names=['lost-origin','changed-origin','changed-task','lost-child','lost-delayed','zero-delay','wrong-provider','changed-mechanism','wrong-check','missing-parent','inflated-review','inflated-learner','inflated-physical','promoted-source-attempt','stale-source','stale-transitive','lost-transitive','failed-receipt','external-unchecked','global-pin','lost-remaining']
            for name in names:
                d=copy.deepcopy(support);m=copy.deepcopy(mapping);r=copy.deepcopy(audit)
                if name=='lost-origin':m['bindings'][0]['origins']=[]
                elif name=='changed-origin':m['bindings'][0]['origins'][0]['record']={}
                elif name=='changed-task':m['bindings'][0]['children'][0]['original_task']='easier'
                elif name=='lost-child':m['bindings'][0]['children'].pop(0)
                elif name=='lost-delayed':m['bindings'][0]['children']=[c for c in m['bindings'][0]['children'] if not c['variant']]
                elif name=='zero-delay':next(c for c in m['bindings'][0]['children'] if c['variant'])['variant']['minimum_days']=0
                elif name=='wrong-provider':m['bindings'][0]['children'][0]['provider']=support['providers'][-1]
                elif name=='changed-mechanism':m['bindings'][0]['children'][0]['mechanism']='heading only'
                elif name=='wrong-check':m['bindings'][0]['children'][0]['checks']=['compiler --version']
                elif name=='missing-parent':d['updates'].pop()
                elif name.startswith('inflated-'):d['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[name]]['state']='passed'
                elif name=='promoted-source-attempt':next(q for q in d['updates'] if q['classification']=='unavailable-reference-attempt')['dimensions']['teaching']['state']='complete-source-inspected'
                elif name=='stale-source':d['source_sha256'][next(iter(d['source_sha256']))]='0'*64
                elif name=='stale-transitive':r['stale']=[dict(path='a changed actual C provider')]
                elif name=='lost-transitive':r['missing']=['required command log']
                elif name=='failed-receipt':r['report_statuses'][next(iter(r['report_statuses']))]['status']='fail'
                elif name=='external-unchecked':r['external_validation']['passed']=False
                elif name=='global-pin':d['source_sha256']['curriculum/v5/expansion/implementation_updates.json']='0'*64
                else:d['remaining_external_work']=[]
                try:inspect(d,m,r,gold);rejected=False
                except (ValueError,KeyError,AssertionError):rejected=True
                controls.append(dict(control=name,rejected=rejected));require(rejected,'false packet accepted '+name)
        require(all(a.sha(ROOT/p)==h for p,h in gold[2].items()) and all(a.sha(ROOT/p)==h for p,h in audit['document_sha256'].items()),'inputs changed during negative controls')
    except (ValueError,KeyError,AssertionError) as ex:errors.append(str(ex))
    audit.update(schema_version=1,created_utc=datetime.datetime.now(datetime.timezone.utc).isoformat(),passed=not errors and not audit['missing'] and not audit['stale'],scope='Exact original IW aliases/parents; no new child execution or independent/physical/source-access acceptance inferred',summary=summary,negative_controls=controls,errors=errors,original_execution_reports_rewritten=False,whole_source_execution_current=not audit['historical_publication_differences'])
    destination.parent.mkdir(parents=True,exist_ok=True);destination.write_text(json.dumps(audit,indent=2)+'\n',encoding='utf8');print(json.dumps(dict(passed=audit['passed'],documents=audit['documents'],files=audit['files'],summary=summary,errors=errors)));return int(not audit['passed'])
if __name__=='__main__':raise SystemExit(main())
