"""Check source/assessment preservation, not whether a learner has mastered it."""
import copy, json
from common import ROOT, read_json, sha
BASE='research/v5/expansion/guard_comparison'

def inspect(packet):
    assert packet['source_sha256'] and all(sha(ROOT/p)==h for p,h in packet['source_sha256'].items()), 'source identity'
    assert {x['id'] for x in packet['updates']}=={'MBR-SOURCE-GUARD','SE-EPOCH-ENFORCEMENT-01'}, 'exact obligations'
    for row in packet['updates']:
        assert row['dimensions']['independent_learner']['state']=='not-assessed', 'learner evidence'
        assert row['dimensions']['external_review']['state']=='not-run', 'review evidence'
        assert row['evidence_limits'], 'bounded source scope'
    resources=read_json(ROOT/BASE/'resources.json')['resources']
    for row in resources:
        capture=row['inspection']['capture']
        assert capture['status']==200 and sha(ROOT/capture['path'])==capture['sha256'], 'actual capture'
        assert row['inspection']['scope'] and row['inspection']['limits'], 'inspection scope'

def main():
    packet=read_json(ROOT/BASE/'completion_dispositions.json');inspect(packet);controls=[]
    for name in ['stale','missing','learner','review','unbounded']:
        bad=copy.deepcopy(packet)
        if name=='stale':bad['source_sha256'][next(iter(bad['source_sha256']))]='0'*64
        elif name=='missing':bad['updates'].pop()
        elif name in ['learner','review']:bad['updates'][0]['dimensions']['independent_learner' if name=='learner' else 'external_review']['state']='passed'
        else:bad['updates'][0]['evidence_limits']=[]
        try:inspect(bad);rejected=False
        except AssertionError:rejected=True
        assert rejected;controls.append(dict(case=name,rejected=True))
    print(json.dumps(dict(passed=True,source_sha256={BASE+'/completion_dispositions.json':sha(ROOT/BASE/'completion_dispositions.json')},controls=controls,scope='Reasoning-package identity and evidence-boundary guard; no learner score or compiler result')))

if __name__=='__main__':main()
