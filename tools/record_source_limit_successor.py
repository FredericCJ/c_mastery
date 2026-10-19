"""Append exact failed access observations without relabeling source gaps."""
import copy,json
from common import ROOT,dump,sha
BASE='research/v5/expansion/source_limits_followthrough'
ARCHIVE='reviews/v5/expansion/source_limit_successor_20261018'
ATTEMPTS={
 'build/source-gap-final/rm0440-access.json':['G-PA-RM0440-FULL','G-PL-G474-FULL-PORT-MANUAL'],
 'build/board_route_recheck/attempts.json':['G-DG-BOARD-ROUTE','G-PA-F439-PA4-HEADER']}

def main():
    path=ROOT/BASE/'completion_dispositions.json'
    old_path=ROOT/ARCHIVE/'before.json';old_path.parent.mkdir(parents=True,exist_ok=True)
    if not old_path.exists():old_path.write_bytes(path.read_bytes())
    original=json.loads(old_path.read_text());data=copy.deepcopy(original)
    for p,h in original['source_sha256'].items():assert sha(ROOT/p)==h,('changed prior source',p)
    records=[]
    for p,ids in ATTEMPTS.items():
        raw=json.loads((ROOT/p).read_text());values=raw if isinstance(raw,list) else [raw]
        assert all(v['status'] in ['failed','unavailable'] and 'timed out' in v['error'] for v in values)
        records.append(dict(affected_ids=ids,observation_path=p,observation_sha256=sha(ROOT/p),actual_observations=values))
    successor=BASE+'/access_attempts_20261018_final.json'
    dump(ROOT/successor,dict(schema_version=1,date='2026-10-18',predecessor=BASE+'/access_attempts.json',observations=records,
        partial_inspection=dict(resource='UM1974 Rev11',actual_scope='Initial title/order-code Table1 and contents only',limits='Exact Table17 PA4 route and section7.11 Ethernet route were not retrieved; no as-built board revision identified.'),
        result='No new register, wiring or normative route fact established. Exact existing source-dependent obligations remain open.',
        authorization='Normal public HTTPS GET only; no access-control bypass, purchase, flashing or host security change.'))
    affected={i for ids in ATTEMPTS.values() for i in ids}
    for row in data['updates']:
        if row['id'] not in affected:continue
        row['source_access_successor']=successor
        row['dimensions']['author_validation']['evidence'].append(successor)
        for block in row['external_blockers']:
            block['additional_attempts']=successor
        row['evidence_limits'].append('Final normal public retrievals also timed out; initial title/contents access is not the missing route/chapter inspection.')
    data['source_sha256'].update({successor:sha(ROOT/successor),'tools/record_source_limit_successor.py':sha(ROOT/'tools/record_source_limit_successor.py')})
    data['preserved_predecessor']=dict(path=old_path.relative_to(ROOT).as_posix(),sha256=sha(old_path))
    assert [r['id'] for r in data['updates']]==[r['id'] for r in original['updates']]
    for a,b in zip(original['updates'],data['updates']):
        assert a['originating_record']==b['originating_record']
        for key in a['dimensions']:
            assert a['dimensions'][key]['state']==b['dimensions'][key]['state']
    dump(path,data)
    dump(ROOT/ARCHIVE/'guard.json',dict(passed=True,scope='Exact existing source-gap states and origins preserved; failed access observations only',unchanged_ids=len(data['updates']),affected_ids=sorted(affected),source_sha256={p:sha(ROOT/p) for p in [successor,BASE+'/completion_dispositions.json','tools/record_source_limit_successor.py',old_path.relative_to(ROOT).as_posix()]},checks=['All normal-access observations report timeouts','Original source identities remain current','Original IDs/origin records/dimension states unchanged','Four affected rows gain explicit failed-attempt references only']))
    print(json.dumps(dict(passed=True,retained=len(data['updates']),additional_attempts=len(records),affected_ids=sorted(affected))))
if __name__=='__main__':main()
