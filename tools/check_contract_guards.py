"""Check rejection of meaningful malformed curriculum contracts without editing sources."""
from copy import deepcopy
from common import ROOT, load, dump, check_program_order, read_json, verify_research_hashes
from validate import check_data
from check_fixtures import required_oracles, check_oracle_presence, selected_oracles
from publish import publication_links

def main():
    original=load(); baseline,_=check_data(*original)
    if baseline: raise ValueError('Baseline must pass before negative checks')
    cases=[]
    interval='Offsets [0,96),[96,496),[496,560).\n\n'
    converted=publication_links(interval+'[fixture](../../fixtures/memory_lab/example.json)')
    assert converted==interval+'{download}`fixture <../fixtures/memory_lab/example.json>`','Download conversion consumed interval/prose'
    cases.append({'case':'half-open intervals before fixture link','passed':True,'expected_guard':'Download conversion preserves preceding prose'})
    scripts=selected_oracles(ROOT)
    check_oracle_presence(scripts)
    for missing in sorted(required_oracles()):
        try:
            check_oracle_presence([p for p in scripts if p.name!=missing])
        except ValueError as error:
            assert 'Missing required fixture oracles' in str(error) and missing in str(error),str(error)
        else: raise AssertionError('Missing oracle accepted: '+missing)
        cases.append({'case':'missing required oracle '+missing,'rejected':True,'expected_guard':'Missing required fixture oracles'})
    def trial(name,modify,needle):
        data=deepcopy(original); modify(data)
        errors,_=check_data(*data)
        assert any(needle in e for e in errors),(name,errors)
        cases.append({'case':name,'rejected':True,'expected_guard':needle})
    def cycle(data):
        data[1]['T01']['meta']['prerequisites']=['T02']; data[1]['T02']['meta']['prerequisites']=['T01']
    trial('prerequisite cycle',cycle,'cycle')
    trial('selected-prelude cycle',lambda d:d[1]['T02']['meta']['prelude_objectives'].append('T03.01.a'),'cycle')
    trial('invalid project prelude',lambda d:next(iter(d[2].values()))['meta'].update(prelude_objectives=['T99.01.a']),'invalid project prelude')
    trial('invalid optional runtime prelude',lambda d:next(iter(d[2].values()))['meta'].update(optional_runtime_prelude_objectives=['T99.01.a']),'invalid project prelude')
    trial('project-prelude cycle',lambda d:next(p for p in d[2].values() if p['meta']['module']=='T01')['meta'].update(prelude_objectives=['T02.01.a']),'cycle')
    order=list(original[1]); order.remove('T02'); order.insert(0,'T02')
    assert check_program_order(order,original[1],original[2]),'Stale invalid order must be rejected'
    cases.append({'case':'stale invalid program order','rejected':True,'expected_guard':'program order precedes foundations'})
    trial('uninspected locator',lambda d:d[1]['T02']['meta']['readings'][0].update(locator='uninspected chapter'), 'uninspected reading locator')
    trial('budget inconsistency',lambda d:d[1]['T02']['meta']['workload'].update(implementation=0), 'do not reconcile')
    trial('boolean masquerading as an hour budget',lambda d:d[1]['T02']['meta']['workload'].update(assessment=True), 'invalid workload breakdown')
    trial('floating-point descriptive allocation',lambda d:next(iter(d[2].values()))['meta'].update(hours=8.0), 'positive integral descriptive hours')
    trial('boolean reading budget',lambda d:d[1]['T02']['meta']['readings'][0].update(hours=True), 'unbounded reading assignment')
    trial('fixture escape',lambda d:next(iter(d[2].values()))['meta'].update(fixtures=['../outside.json']), 'escaping fixture')
    trial('missing teaching',lambda d:d[1]['T02'].update(body=d[1]['T02']['body'].replace('### T02.01','### LOST.01')), 'missing substantive')
    v2=(ROOT/'curriculum/edition.json').exists()
    trial('missing leaf',lambda d:d[0][0]['subtopics'][0]['objectives'].pop(), 'Unmapped prior objective' if v2 else '1,185 unique')
    trial('changed original objective',lambda d:d[0][0]['subtopics'][0]['objectives'][0].update(text='replacement'), 'Retained objective changed' if v2 else 'preserved exactly')
    trial('duplicate taxonomy domain',lambda d:d[0].append(deepcopy(d[0][-1])), 'Duplicate taxonomy domain')
    if len(original[0])>35 and not v2:
        trial('missing enhancement objective',lambda d:d[0][-1]['subtopics'][0]['objectives'].pop(), 'three distinct')
    if (ROOT/'curriculum/performance_debugging_taxonomy.json').exists():
        def alter_prior_extension(data):
            next(d for d in data[0] if d['id']=='T36')['subtopics'][0]['objectives'][0]['text']='replacement'
        trial('changed prior enhancement objective',alter_prior_extension,'Retained objective changed' if v2 else 'Prior 198 enhancement objectives')
        gate=deepcopy(read_json(ROOT/'reviews/performance_debugging_research_gate.json'))
        path=next(iter(gate['research_input_sha256']))
        gate['research_input_sha256'][path]='0'*64
        try:
            verify_research_hashes(gate,'research/performance_debugging')
        except ValueError as error:
            assert 'Research input changed after gate' in str(error),str(error)
        else: raise AssertionError('Stale source hash accepted')
        cases.append({'case':'stale new research evidence hash','rejected':True,'expected_guard':'Research input changed after gate'})
    def lead(data):
        rid=next(r['id'] for r in data[3] if r.get('assignable') is False)
        data[1]['T02']['meta']['readings'][0]['resource_id']=rid
    trial('metadata-only reading',lead,'bibliography-only lead')
    dump(ROOT/'reviews/contract_guard_checks.json',{'passed':True,'cases':cases})
    print(f'{len(cases)} contract, dispatcher and publication guards passed; canonical files unchanged.')

if __name__=='__main__': main()
