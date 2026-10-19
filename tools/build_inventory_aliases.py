"""Build additive exact-origin IW dispositions; never write global registers."""
import copy,hashlib,json,pathlib
from audit_inventory_aliases import ROOT,BASE,read,sha,original,children
DIMENSIONS=('teaching','implementation','author_validation','external_review','independent_learner','physical_observation')
PUBLIC='labs/inventory_alias_followthrough/README.md'
REMAINING=['Original physical target/board and deployment observations remain unperformed where required.','Original delayed independent learner, prerequisite portfolio and external review remain unperformed.','Exact final-standard/source-access gaps and the failed N3245 capture remain explicit.']
def put(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2,ensure_ascii=False)+'\n',encoding='utf8')
def key(record):return record.get('topic') or (record.get('project','').split('-')[1] if record.get('project','').startswith('PR-P') else None)
def clause(record,policy):
    if key(record) in policy['topics']:return policy['topics'][key(record)]
    if record.get('objective'):return ('Exact leaf '+record['objective']+': '+record.get('text',record.get('work',''))+' The bound canonical ordinary and original delayed tasks below provide the specific mechanisms and changed-condition evidence. Historical pending text is retained; source, learner, review and physical clauses are not silently closed.')
    return policy['source_attempt']
def make_rows(inspection,policy):
    rows=[];bindings=[];sources={PUBLIC,'tools/audit_inventory_aliases.py','tools/build_inventory_aliases.py','tools/check_inventory_aliases.py','tools/check_residual_followthrough.py',BASE.relative_to(ROOT).as_posix()+'/inspection_input.json',BASE.relative_to(ROOT).as_posix()+'/clause_map.json'};providers=set()
    for entry in inspection['records']:
        records=[o['record'] for o in entry['origins']];assert len(records)==1;record=records[0];ordinary=children(record);expected={c['id']:c for c in ordinary};selected={c['id']:c for c in entry['candidate_children']};assert set(expected)==set(selected)
        for o in entry['origins']:assert original(o)==o['record'];sources.add(o['path'])
        source_only=not expected;assert source_only==(entry['id']=='IW-a8b7587eadd8ddb4');mechanisms=[];evidence={k:set() for k in DIMENSIONS};limits=set();scope=clause(record,policy)
        for ident,c in selected.items():
            e=expected[ident];assert all(c[k]==e[k] for k in ['canonical','criterion','variant','task']);provider=c['selected_provider'];d=read(ROOT/provider);actual=next(x for x in d['updates'] if x['id']==ident);assert actual==c['provider_record'];providers.add(provider);sources.add(provider);sources.add(c['canonical'])
            for dim in ['teaching','implementation','author_validation']:evidence[dim].update(actual['dimensions'][dim]['evidence'])
            evidence['author_validation'].add(provider)
            for item in actual.get('evidence_limits',[])+actual.get('external_blockers',[]):limits.add(item if isinstance(item,str) else json.dumps(item,sort_keys=True,ensure_ascii=False))
            mechanisms.append(dict(id=ident,canonical=c['canonical'],criterion=c['criterion'],variant=c['variant'],original_task=c['task'],provider=provider,provider_sha256=sha(ROOT/provider),provider_record=actual,mechanism=actual['closure_rationale'],checks=actual.get('mechanism_checks',actual.get('author_validation',{}).get('checks',[]))))
        if source_only:
            from source_capture_metadata import require_metadata
            metadata=require_metadata(entry['origins'][0]);sources.update({metadata,'tools/source_capture_metadata.py'});evidence['teaching'].add(entry['origins'][0]['path']);evidence['author_validation'].add(metadata);states=dict(teaching='reference-gap-retained;not-inspected',implementation='not-applicable-source-attempt-record',author_validation='failed-parse-attempt-preserved;not-source-inspection',external_review='not-applicable-source-attempt-record',independent_learner='not-applicable-source-attempt-record',physical_observation='not-applicable-source-attempt-record')
        else:
            assert all(m['provider_record']['dimensions']['implementation']['state'].startswith(('complete','implemented')) for m in mechanisms)
            assert all(m['provider_record']['dimensions']['author_validation']['state'].startswith('passed') for m in mechanisms)
            states=dict(teaching='complete-for-scoped-author-work;original-source-gaps-retained',implementation='complete-for-exact-bound-author-mechanisms',author_validation='passed-current-source-bound-child-evidence;no-new-execution-inferred',external_review='not-run',independent_learner='not-assessed',physical_observation='not-claimed;original-required-physical-work-retained')
            evidence['teaching'].add(PUBLIC)
        original_clauses=record.get('work',record.get('required_remaining',record.get('remaining',record.get('reason',''))))
        binding=dict(id=entry['id'],origins=entry['origins'],links=entry['links'],requirements=entry['requirements'],classification='unavailable-reference-attempt' if source_only else 'exact-author-mechanism-parent-or-alias',original_remaining_clauses=original_clauses,clause_analysis=scope,children=mechanisms)
        bindings.append(binding)
        rows.append(dict(id=entry['id'],original_origins=entry['origins'],classification=binding['classification'],dimensions={k:dict(state=states[k],evidence=sorted(evidence[k])) for k in DIMENSIONS},closure_rationale=scope,required_criteria_and_variants=[m['id'] for m in mechanisms],original_remaining_clauses=original_clauses,artifacts=sorted(set().union(*evidence.values())),evidence_limits=sorted(limits|{'This packet reconciles current author evidence; it does not rerun every child experiment or administer an independent assessment.','Original source-access, prerequisite-portfolio and physical-observation clauses remain required in their own dimensions.','A failed N3245 capture remains unavailable and is never credited as an inspected source.'})))
        for e in evidence.values():sources.update(e)
    return rows,bindings,sources,providers
def build():
    inspection=read(BASE/'inspection_input.json');policy=read(BASE/'clause_map.json');assert len(inspection['records'])==81 and not inspection['missing_provider_ids'];rows,bindings,sources,providers=make_rows(inspection,policy)
    put(BASE/'evidence_map.json',dict(schema_version=1,scope='Exact original parent bodies, complete canonical ordinary/delayed child selection and actual versioned provider records',bindings=bindings));sources.add((BASE/'evidence_map.json').relative_to(ROOT).as_posix());assert all((ROOT/p).is_file() for p in sources)
    put(BASE/'support.json',dict(schema_version=1,scope='80 exact author-evidence aliases/parents and one preserved failed-reference attempt; not global, physical, learner or final-standard acceptance',updates=rows,source_sha256={p:sha(ROOT/p) for p in sorted(sources)},providers=sorted(providers),remaining_external_work=REMAINING))
    print(json.dumps(dict(updates=len(rows),children=len({c['id'] for b in bindings for c in b['children']}),providers=len(providers))))
if __name__=='__main__':build()
