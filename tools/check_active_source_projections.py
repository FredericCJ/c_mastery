"""Read-only reconstruction of 128 existing source-only dispositions.

The original closers mutate global registers at top level and are never
imported here. Current authority validation is a new observation; the retained
2026-10-17 receipt and its five stale resource-register pins stay historical.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
from common import ROOT, dump, load_taxonomy, read_json, sha
import vocabulary
from resource_inputs_v5 import discover

BASE = 'research/v5/expansion/followthrough'
OLD = 'reviews/v5/expansion/followthrough/vocabulary_sources.json'
SOURCE = BASE+'/source_access_successor.json'


def vocabulary_projection(corrections, entries, resources, ledger):
    current = {r['id']:r for r in entries}
    registry = {r['id']:r for r in resources}
    assert len(corrections)==126 and len({r[2]['entry'] for r in corrections})==126
    gaps=[]; updates=[]
    for path,index,row,default in corrections:
        rid=row.get('replacement_resource',default)
        matches=[a for a in current[row['entry']]['authority'] if a['resource_id']==rid
                 and a['locator']==row['replacement_locator'] and a.get('inspection')=='registry']
        assert len(matches)==1,('current unique authority',row['entry'])
        resource=registry[rid]
        assert resource.get('assignable') is not False and row['replacement_locator'] in resource['locators']
        gap=dict(id='G-SOURCE-'+row['entry'],kind='previously-uninspected-authority',scope=row['entry'],
            originating_records=[path+'#/changed_entries/'+str(index),row['file']],prior_authority=row['prior_authority'],
            requirement='Inspect the actual bounded passage, correct the definition and retain version/semantic boundaries.',
            status='author-source-inspection-complete',replacement_resource=rid,replacement_locator=row['replacement_locator'],
            evidence=[path,row['file'],BASE+'/sources.md'],
            limits='Source traceability is not external technical review, implementation execution or learner evidence.')
        gaps.append(gap)
        updates.append(dict(id=gap['id'],dimensions={
            'teaching':dict(state='complete',evidence=gap['evidence']),
            'implementation':dict(state='not-applicable-source-correction',evidence=gap['evidence']),
            'author_validation':dict(state='passed',evidence=[OLD]),
            'external_review':dict(state='not-run',evidence=[]),
            'independent_learner':dict(state='not-applicable-authority-record',evidence=[]),
            'physical_observation':dict(state='not-applicable-source-correction',evidence=[])},
            closure_rationale='Actual bounded passage inspection, explicit corrected authority, edition limits and glossary consistency validation.',
            evidence_limits=['No implementation-execution, physical, independent-review or learner-mastery claim.']))
    assert gaps==ledger['gaps'][:126], 'Exact original source-ledger projection changed'
    return updates


def build():
    entries,_=vocabulary.load()
    registered={r['id']:r for r in read_json(ROOT/'references/resources.json')['resources']}
    additions, discovery=discover(ROOT)
    registered.update({r['id']:r for r in additions if not r.get('reuse')})
    errors,pending=vocabulary.validate(entries,list(registered.values()),load_taxonomy())
    assert not errors and not pending,(errors,pending)
    corrections=[]; paths={OLD,SOURCE,BASE+'/source_gap_ledger.json',BASE+'/sources.md',BASE+'/close_sources.py',
        'tools/close_followthrough_source_access.py','tools/check_active_source_projections.py',
        'tools/vocabulary.py','tools/common.py','tools/resource_inputs_v5.py','research/v5/resource_merge_policy.json'}
    for p in sorted((ROOT/BASE).glob('*vocabulary_corrections.json')):
        doc=read_json(p);name=p.relative_to(ROOT).as_posix();paths.add(name)
        default=doc.get('inspected_source')
        if p.name=='runtime_vocabulary_corrections.json':
            resource=BASE+'/runtime_vocabulary_resources.json';paths.add(resource)
            default=read_json(ROOT/resource)['resources'][0]['id']
        corrections.extend((name,i,r,default) for i,r in enumerate(doc['changed_entries']))
    ledger=read_json(ROOT/BASE/'source_gap_ledger.json')
    updates=vocabulary_projection(corrections,entries,list(registered.values()),ledger)
    old=read_json(ROOT/OLD)
    assert old['passed'] is True and old['exit_status']==0 and old['corrected_authorities']==126
    assert not old['errors'] and not old['pending_authorities']
    history=[dict(path=p,expected=h,current=sha(ROOT/p)) for p,h in old['input_sha256'].items() if sha(ROOT/p)!=h]
    allowed={'research/v5/modern/'+n+'_resources.json' for n in ['dispatch','messaging','object_runtime','replicated','resilience']}
    assert {r['path'] for r in history}<=allowed,'Unexpected original source receipt change'
    paths.update(p.relative_to(ROOT).as_posix() for p in vocabulary.files())
    used_resources={a['resource_id'] for entry in entries for a in entry['authority']}
    for rid in used_resources:
        paths.update(r['path'] for r in discovery['registration_provenance'].get(rid,[]))
    records=read_json(ROOT/SOURCE)['sources'];assert {r['id'] for r in records}=={'G-ABSTRACTION-KR511','G-CONTRACT-MODERNC31'}
    captures=[]
    for source in records:
        path=Path(source['private_source']);expected=source.get('sha256',source.get('retrieval',{}).get('sha256'))
        assert len(expected)==64 and source['inspected'] and source['limitations']
        observed=path.is_file()
        if observed:assert sha(path)==expected,('changed retained private source',source['id'])
        captures.append(dict(id=source['id'],sha256=expected,local_capture_revalidated=observed,
                             scope='Retained inspection identity; no new passage reading or redistribution.'))
        registry_path,ident=source['closure_evidence'].split('#');paths.add(registry_path)
        assert any(r['id']==ident for r in read_json(ROOT/registry_path)['resources'])
        updates.append(dict(id=source['id'],original_source=source['original_source'],closure_rationale=source['technical_consequence'],
            dimensions={'teaching':dict(state=source['current_status'],evidence=[SOURCE,source['closure_evidence']]),
                'implementation':dict(state='not-applicable-to-source-access-only-obligation',evidence=[SOURCE]),
                'author_validation':dict(state='actual-named-passage-inspection-and-byte-identity',evidence=[SOURCE]),
                'external_review':dict(state='not-inferred',evidence=[]),'independent_learner':dict(state='not-inferred',evidence=[]),
                'physical_observation':dict(state='not-applicable-to-source-access',evidence=[])},
            required_environment='Authorized exact named source access; no private source redistribution.',evidence_limits=[source['limitations']]))
    # Bind selected canonical resource objects rather than unrelated new rows in
    # the generated global registry. Every source addition has its concrete pin.
    selected={rid:registered[rid] for rid in sorted(used_resources)}
    return dict(schema_version=1,passed=True,updates=updates,input_sha256={p:sha(ROOT/p) for p in sorted(paths)},
        selected_resources=selected,selected_resources_sha256=hashlib.sha256(json.dumps(selected,sort_keys=True,ensure_ascii=False).encode()).hexdigest(),
        historical_receipt=dict(path=OLD,sha256=sha(ROOT/OLD),pin_differences=history,new_execution_claim=False),
        captured_inspection_identities=captures,current_vocabulary_validation=dict(passed=True,pending=0,errors=[]),
        limits=['Exact existing source-only rows reconstructed; original closers and global registers are not executed or modified.',
            'Current glossary validation is separate from the retained historical receipt and original bounded passage inspection.',
            'Published C17/C23 gaps, independent review, learners, and physical observations are not closed.'])


def selftest():
    entries,_=vocabulary.load();resources=read_json(ROOT/'references/resources.json')['resources']
    more,_=discover(ROOT);registry={r['id']:r for r in resources};registry.update({r['id']:r for r in more if not r.get('reuse')})
    assert not any(vocabulary.validate(entries,list(registry.values()),load_taxonomy()))
    corrections=[]
    for p in sorted((ROOT/BASE).glob('*vocabulary_corrections.json')):
        d=read_json(p);default=d.get('inspected_source')
        if p.name=='runtime_vocabulary_corrections.json':default=read_json(ROOT/BASE/'runtime_vocabulary_resources.json')['resources'][0]['id']
        corrections.extend((p.relative_to(ROOT).as_posix(),i,r,default) for i,r in enumerate(d['changed_entries']))
    ledger=read_json(ROOT/BASE/'source_gap_ledger.json')
    edits=[lambda c,e,r,l:c.pop(),lambda c,e,r,l:c[0][2].update(replacement_locator='uninspected'),
        lambda c,e,r,l:l['gaps'][0].update(status='open'),lambda c,e,r,l:e[0].update(authority=[])]
    for edit in edits:
        c,e,r,l=copy.deepcopy((corrections,entries,list(registry.values()),ledger));edit(c,e,r,l)
        try:vocabulary_projection(c,e,r,l)
        except (AssertionError,KeyError):continue
        raise AssertionError('Invalid source projection accepted')
    return len(edits)


if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args()
    out=(ROOT/a.output).resolve();assert out.is_relative_to((ROOT/'reviews/v5/expansion').resolve()) and not out.exists()
    d=build();d['negative_controls']=selftest() if a.self_test else 0;dump(out,d)
    print(json.dumps(dict(passed=True,rows=len(d['updates']),negative_controls=d['negative_controls'],sha256=sha(out))))
