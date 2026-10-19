"""Versioned exact Arm successor in the eight-row boundary packet.

Seven rows retain their previous bytes. Only three predeclared provider changes
are admitted, after comparing original and repeated actual execution records.
"""
import argparse
import copy
import json
import re
from common import ROOT, read_json, sha
from check_arm_walker_sources import inspect as check_arm
from register_arm_walker_sources import BASE, IDENT, REPORTS

DEST = 'research/v5/expansion/mixed_boundary_audit/completion_dispositions.json'
OLD = 'reviews/v5/expansion/arm_walker_sources/derivative_history/2-completion_dispositions.json'
OLD_SHA = '5c2e0b837d0cd2db0a94153c04f2912c73286382ab21d8fec70948521aaa631d'
HISTORY = 'reviews/v5/expansion/arm_walker_sources/before_register_correction/manifest.json'
ASM = 'assessor/architecture_ports/guests/aa64.S'
SELF = 'tools/refresh_mixed_boundary_arm.py'
VALIDATION = 'reviews/v5/expansion/mixed_boundary_audit/arm_successor_validation.json'
ARM_PACKET = BASE + '/completion_dispositions.json'


def write(path, data):
    out=ROOT/path;out.parent.mkdir(parents=True,exist_ok=True)
    out.write_text(json.dumps(data,indent=2,ensure_ascii=False)+'\n',encoding='utf-8')


def expected():
    assert sha(ROOT/OLD)==OLD_SHA, 'immutable old mixed packet'
    old=read_json(ROOT/OLD)
    history=read_json(ROOT/HISTORY)
    archive={r['path']:r for r in history['files']}
    assert all(sha(ROOT/r['preserved_path'])==r['sha256'] for r in history['files']), 'history bytes'
    for path in [ASM,*REPORTS]:
        assert old['source_sha256'][path]==archive[path]['sha256'], 'old provider identity'
    def without_comments(text):return re.sub(r'(?m)^\s*//[^\n]*\n','',text)
    before=(ROOT/archive[ASM]['preserved_path']).read_text()
    after=(ROOT/ASM).read_text()
    assert without_comments(before).replace('=0x803519','=0x80993519')==without_comments(after), 'only selected TCR operation changed'
    checks=[]
    for path in REPORTS:
        previous=read_json(ROOT/archive[path]['preserved_path']);current=read_json(ROOT/path)
        assert previous['passed'] and current['passed']
        assert all(r['passed'] for r in current['checks'])
        assert [(r['label'],r.get('expected'),r.get('marker')) for r in previous['checks']]==[(r['label'],r.get('expected'),r.get('marker')) for r in current['checks']], 'same actual check contracts'
        assert set(previous['input_sha256'])==set(current['input_sha256'])
        changed={p for p in previous['input_sha256'] if previous['input_sha256'][p]!=current['input_sha256'][p]}
        assert changed==({ASM} if 'abi_guest' in path else set()), 'scoped campaign source delta'
        assert current['recorded_at']!=previous['recorded_at'], 'new execution receipt'
        assert all(sha(ROOT/p)==h for p,h in current['input_sha256'].items())
        checks.append(dict(path=path,sha256=sha(ROOT/path),checks=len(current['checks']),source_current=True))
    assert all(p in [ASM,*REPORTS] or sha(ROOT/p)==h for p,h in old['source_sha256'].items()), 'unrelated provider changed'
    arm=read_json(ROOT/ARM_PACKET)
    check_arm(arm,read_json(ROOT/BASE/'inspection.json'),read_json(ROOT/BASE/'resources.json'))
    # Preserve every original report, but recheck all current source bindings.
    for p in old['current_reports']:
        r=read_json(ROOT/p)
        if isinstance(r,list):
            assert all(q.get('passed') is True and not q.get('stale') for q in r)
            continue
        assert r.get('passed') is True, p
        for k in ['input_sha256','source_sha256','inspected_paths']:
            assert all(sha(ROOT/f)==h for f,h in r.get(k,{}).items()), ('stale report',p,k)
    result=copy.deepcopy(old)
    result['updates']=[copy.deepcopy(arm['updates'][0]) if r['id']==IDENT else r for r in result['updates']]
    pins={p:sha(ROOT/p) for p in old['source_sha256']}
    pins.update(arm['source_sha256'])
    for p in [OLD,HISTORY,SELF,ARM_PACKET]:pins[p]=sha(ROOT/p)
    validation=dict(passed=True,source_sha256=pins,checks=checks,
        scope='Repeated source-current walker/guest observations; seven other mixed-boundary dispositions unchanged.',
        allowed_changed_previous_inputs=[ASM,*REPORTS],
        limits=['No new physical, independent learner or external review result.','Original packet and old guest source/receipts remain byte-preserved.'])
    result['source_sha256']=pins
    result['current_reports']=list(dict.fromkeys(old['current_reports']+[VALIDATION,'reviews/v5/expansion/arm_walker_sources/export_validation.json']))
    result['arm_successor']=dict(previous_packet=OLD,previous_sha256=OLD_SHA,source_packet=ARM_PACKET,
        replaced_id=IDENT,unchanged_ids=[r['id'] for r in old['updates'] if r['id']!=IDENT],
        rationale='Actual selected primary inspection found a feasible reserved-field correction; current execution and canonical learner export now support the selected source closure.')
    return result,validation


def validate(packet, report):
    wanted,validation=expected()
    assert report==validation, 'exact current validation'
    wanted['source_sha256'][VALIDATION]=sha(ROOT/VALIDATION)
    assert packet==wanted, 'exact successor and seven preserved rows'


def main():
    p=argparse.ArgumentParser();p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);a=p.parse_args()
    if a.write:
        packet,validation=expected();write(VALIDATION,validation)
        packet['source_sha256'][VALIDATION]=sha(ROOT/VALIDATION);write(DEST,packet)
    packet=read_json(ROOT/DEST);validation=read_json(ROOT/VALIDATION);validate(packet,validation)
    controls=[]
    if a.self_test:
        for name in ['missing','old-source-claim','other-row','learner','physical','stale','old-report','wrong-allowed-change']:
            x,y=copy.deepcopy((packet,validation))
            if name=='missing':x['updates'].pop()
            elif name=='old-source-claim':
                next(r for r in x['updates'] if r['id']==IDENT)['dimensions']['teaching']['state']='partial-source-closure'
            elif name=='other-row':x['updates'][0]['closure_rationale']='unrelated claim replaced'
            elif name in ['learner','physical']:
                key='independent_learner' if name=='learner' else 'physical_observation'
                x['updates'][0]['dimensions'][key]['state']='passed'
            elif name=='stale':x['source_sha256'][ASM]='0'*64
            elif name=='old-report':y['checks'][0]['sha256']='0'*64
            else:y['allowed_changed_previous_inputs'].append('unrelated.c')
            try:validate(x,y)
            except AssertionError:controls.append(dict(case=name,rejected=True))
            else:raise AssertionError('control accepted '+name)
    write(a.report,dict(passed=True,rows=len(packet['updates']),negative_controls=controls,
        source_sha256={DEST:sha(ROOT/DEST),SELF:sha(ROOT/SELF)},
        scope='Exact bounded successor metadata and current source/receipt checks; no extra execution claim.'))
    print(json.dumps(dict(passed=True,rows=len(packet['updates']),controls=len(controls),support_sha256=sha(ROOT/DEST))))


if __name__=='__main__':main()
