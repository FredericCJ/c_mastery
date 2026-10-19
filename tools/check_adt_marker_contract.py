"""Bind original and successor ADT semantic failures to their actual harnesses.

The generic predecessor expects ADT_CHECK for every case. Successor sources
deliberately emit ADO_CHECK. Reclassification changes only these observed
learner outcomes; all profiles, controls, logs and the predecessor are retained.
"""
import argparse,copy,json,re
from pathlib import Path
from common import ROOT,sha,dump

SOURCES=['tools/run_adt.py','tools/check_adt_marker_contract.py']
ORIGINAL={'adt.'+x for x in ['sums','quantities','flags','text','brands','views','arena','optics','evolution','packet']}

def corrected(raw):
    meta=json.loads((ROOT/'labs/adt/family.json').read_text())
    tests=meta['tests'];out=copy.deepcopy(raw);rows=[]
    assert set(ORIGINAL)<=set(tests)
    for test in tests:
        marker='ADT_CHECK' if test in ORIGINAL else 'ADO_CHECK'
        expected='^'+re.escape(test)+'$'
        matches=[r for r in raw['executions'] if '-R' in r['command'] and r['command'][r['command'].index('-R')+1]==expected]
        assert len(matches)==1,'Exact learner observation '+test
        r=matches[0];p=ROOT/r['log'];assert sha(p)==r['log_sha256'],'Changed learner log'
        text=p.read_text(encoding='utf8')
        rejected=(r['exit_code']==8 and marker in text and '1/1 Test' in text
                  and test in text and not any(s in text for s in ['AddressSanitizer','runtime error:','SegFault','No tests were found']))
        rows.append(dict(test=test,rejected=rejected,assertion_marker=marker,log=r['log'],log_sha256=r['log_sha256']))
    out['learner_stub_rejections']=rows
    learner_logs={r['log'] for r in rows}
    all_other_commands=all(r['exit_code']==0 for r in raw['executions'] if r['log'] not in learner_logs)
    out['passed']=all_other_commands and all(p['passed'] for p in raw['profiles']) and all(r['rejected'] for r in rows) and raw['scope_source_stable'] is True
    out['status']='passed' if out['passed'] else 'failed'
    return out

def validate(d,files=True):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    from check_expansion_campaigns import generic_errors,pin_errors,execution_errors
    meta=json.loads((ROOT/'labs/adt/family.json').read_text())
    errors+=generic_errors('adt',meta,d)
    need(d.get('passed') is True and d.get('scope_source_stable') is True,'Passed stable ADT observations')
    need(d.get('physical_validation')=='not-run' and d.get('learner_mastery')=='not-assessed','External evidence boundary')
    for r in d.get('learner_stub_rejections',[]):
        need(r.get('assertion_marker')==('ADT_CHECK' if r.get('test') in ORIGINAL else 'ADO_CHECK'),'Exact per-case assertion marker')
    need(set(SOURCES)<=set(d.get('inspected_paths',{})),'Marker producer/guard identities')
    ref=d.get('generic_predecessor',{});need(bool(ref.get('path')) and bool(ref.get('sha256')),'Retained generic predecessor')
    if files:
        errors+=pin_errors(d)+execution_errors(d)
        try:
            p=ROOT/ref['path'];need(sha(p)==ref['sha256'],'Predecessor bytes')
            raw=json.loads(p.read_text());expected=corrected(raw)
            expected['generic_predecessor']=ref
            expected['inspected_paths'].update({s:sha(ROOT/s) for s in SOURCES})
            need(d==expected,'Only evidenced marker classification changed')
        except (OSError,ValueError,KeyError,AssertionError) as e:errors.append(str(e))
    return errors

def main():
    p=argparse.ArgumentParser();p.add_argument('--report',required=True);p.add_argument('--self-test',action='store_true');p.add_argument('--output');a=p.parse_args()
    d=json.loads((ROOT/a.report).read_text());errors=validate(d);controls=[]
    if a.self_test:
        changes=[('profile',lambda x:x['profiles'].pop()),('anchor',lambda x:x['profiles'][0]['anchors'].pop()),
          ('control',lambda x:x['profiles'][0]['compiled_mutant_controls'].pop()),('learner',lambda x:x['learner_stub_rejections'].pop()),
          ('marker',lambda x:x['learner_stub_rejections'][-1].update(assertion_marker='ADT_CHECK')),
          ('producer',lambda x:x['inspected_paths'].pop(SOURCES[0])),('predecessor',lambda x:x.pop('generic_predecessor')),
          ('physical',lambda x:x.update(physical_validation='passed'))]
        for name,mutate in changes:
            c=copy.deepcopy(d);mutate(c);rejected=bool(validate(c,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('Accepted '+name)
    r=dict(passed=not errors,errors=errors,negative_controls=controls,report=a.report,report_sha256=sha(ROOT/a.report))
    if a.output:dump(ROOT/a.output,r)
    print(json.dumps(r));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
