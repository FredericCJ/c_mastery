"""Exact named-parent evidence bridge; never write shared inventory or updates."""
import json
from audit_named_parents import ROOT,BASE,SELECT,read,sha,original,children

DIMENSIONS=('teaching','implementation','author_validation','external_review','independent_learner','physical_observation')
PUBLIC='labs/named_parent_followthrough/README.md'

def put(p,value):
 p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n',encoding='utf8')

def rows():
 intake=read(BASE/'inspection_input.json');policy=read(BASE/'clause_map.json')['parents']
 assert {r['id'] for r in intake['records']}==set(SELECT)==set(policy) and not intake['missing_provider_ids']
 updates=[];bindings=[];providers=set();files={PUBLIC,'tools/audit_named_parents.py','tools/build_named_parents.py','tools/check_named_parents.py','tools/check_residual_followthrough.py',BASE.relative_to(ROOT).as_posix()+'/inspection_input.json',BASE.relative_to(ROOT).as_posix()+'/clause_map.json'}
 for entry in intake['records']:
  ident=entry['id'];p=policy[ident];expected=children(ident);actual={c['id']:c for c in entry['candidate_children']};assert set(expected)==set(actual)
  evidence={k:set() for k in DIMENSIONS};evidence['teaching'].update([PUBLIC,*p['sources']]);evidence['author_validation'].update(p['sources']);limits=set(p['remaining']);bound=[];partial=[]
  for origin in entry['origins']:assert original(origin)==origin['record'];files.add(origin['path'])
  for cid,c in actual.items():
   assert all(c[k]==expected[cid][k] for k in ['canonical','criterion','variant','task'])
   provider=c['provider'];r=next(r for r in read(ROOT/provider)['updates'] if r['id']==cid);assert r==c['provider_record'];providers.add(provider);files.update([provider,c['canonical']])
   if not (r['dimensions']['implementation']['state'].startswith(('complete','implemented')) and r['dimensions']['author_validation']['state'].startswith('passed')):
    assert (cid=='AX-PR-T25-01-T25.01.c-DELAYED'
            and r['dimensions']['implementation']['state']=='not-applicable-source-access'
            and r['dimensions']['author_validation']['state']=='partial-exact-reference-unavailable')
    partial.append(cid)
   for dim in ['teaching','implementation','author_validation']:evidence[dim].update(r['dimensions'][dim]['evidence'])
   evidence['author_validation'].add(provider)
   for limit in r.get('evidence_limits',[])+r.get('external_blockers',[]):limits.add(limit if isinstance(limit,str) else json.dumps(limit,sort_keys=True,ensure_ascii=False))
   bound.append(dict(id=cid,canonical=c['canonical'],criterion=c['criterion'],variant=c['variant'],original_task=c['task'],provider=provider,provider_sha256=sha(ROOT/provider),provider_record=r,mechanism=r['closure_rationale'],checks=r.get('mechanism_checks',r.get('author_validation',{}).get('checks',[]))))
  states=dict(teaching=p['teaching'],implementation='partial-exact-published-edition-comparison' if partial else p['implementation'],author_validation='passed-retained-experiments;partial-published-edition-comparison' if partial else 'passed-current-source-bound-child-and-source-evidence;no-new-execution-inferred',external_review='not-run',independent_learner='not-assessed',physical_observation='not-claimed;original-physical-obligation-retained')
  binding=dict(id=ident,origins=entry['origins'],links=entry['links'],requirements=entry['requirements'],selected_topics=entry['selected_topics'],clause_analysis=p['analysis'],remaining=p['remaining'],partial_children=partial,children=bound);bindings.append(binding)
  updates.append(dict(id=ident,original_origins=entry['origins'],dimensions={k:dict(state=states[k],evidence=sorted(evidence[k])) for k in DIMENSIONS},closure_rationale=p['analysis'],required_criteria_and_variants=list(actual),partial_children=partial,remaining_required_work=p['remaining'],artifacts=sorted(set().union(*evidence.values())),evidence_limits=sorted(limits|{'This parent bridge rechecks retained evidence; it does not rerun every child or confer independent learner mastery.'})))
  for values in evidence.values():files.update(values)
  updates[-1]['external_blockers']=p.get('external_blockers',[])
 return updates,bindings,files,providers

def main():
 updates,bindings,files,providers=rows();mapping=BASE/'evidence_map.json';put(mapping,dict(schema_version=1,scope='Original parent clauses and complete exact canonical child bindings',bindings=bindings));files.add(mapping.relative_to(ROOT).as_posix())
 assert all((ROOT/p).is_file() for p in files)
 put(BASE/'support.json',dict(schema_version=1,scope='Seven named source/target parents; full proof and exact schematic source gaps remain explicit',updates=updates,source_sha256={p:sha(ROOT/p) for p in sorted(files)},providers=sorted(providers),source_reports=['reviews/v5/expansion/systems_book_followthrough/validation_02.json']))
 print(json.dumps(dict(parents=len(updates),children=len({c['id'] for b in bindings for c in b['children']}),providers=len(providers))))
if __name__=='__main__':main()
