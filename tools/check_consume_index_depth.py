"""Exact source/command/output guard for the consume/index companion."""
import argparse
import copy
import json
import pathlib
from run_consume_index_depth import HERE,ROOT,pins,sha

def validate(d):
    errors=[]
    def need(ok,reason):
        if not ok:errors.append(reason)
    need(d.get('status')=='pass','campaign status')
    need(d.get('source_sha256')==pins(),'source closure')
    for path,h in d.get('outputs_sha256',{}).items():
        p=ROOT/path;need(p.is_file() and sha(p)==h,'output '+path)
    commands=d.get('commands',[]);by_id={x['id']:x for x in commands}
    need(len(by_id)==len(commands),'unique commands')
    need(bool(commands) and all(x.get('passed') is True and x.get('returncode')==x.get('expected_returncode') and (x.get('marker') is None or x['marker'] in x.get('stdout','')) for x in commands),'command evidence')
    for cc in ['gcc','clang']:
        for profile in ['debug','optimized','sanitized']:
            tag=cc+'.'+profile
            names=['compile','value','branch','other','oracle','learner.compile','learner.value','learner.branch','learner.other']
            names += [f'mutant{n}.{phase}' for n in [1,2,3] for phase in ['compile','reject']]
            for name in names:need(tag+'.'+name in by_id,'missing '+tag+'.'+name)
            for name in ['compile','learner.compile','mutant1.compile','mutant2.compile','mutant3.compile']:
                need('-std=c17' in by_id.get(tag+'.'+name,{}).get('command',[]),'C17 '+tag+'.'+name)
                need(by_id.get(tag+'.'+name,{}).get('returncode')==0,'successful compile '+tag+'.'+name)
            for name in ['learner.value','learner.branch','learner.other','mutant1.reject','mutant2.reject','mutant3.reject']:
                row=by_id.get(tag+'.'+name,{})
                need(row.get('returncode')==41 and ('CI_ORACLE:' if name=='mutant3.reject' else 'CI_CHECK:') in row.get('stdout',''),'exact rejection '+tag+'.'+name)
            need('"graphs": 729' in by_id.get(tag+'.oracle',{}).get('stdout',''),'independent graph count')
    for cc in ['arm-none-eabi-gcc','clang']:
        for opt in ['-O0','-O3']:
            tag=cc+'.m4'+opt
            for part in ['compile','undefined','link','link-undefined','disassembly','ci_pointer','ci_value_index','ci_branch_index','ci_killed_index','ci_acquire_branch']:
                need(tag+'.'+part in by_id,'missing target '+tag+'.'+part)
            for part in ['undefined','link-undefined']:need(by_id.get(tag+'.'+part,{}).get('stdout')=='','unresolved helper')
    need('CI_CHECK:' in by_id.get('export.learner',{}).get('stdout',''),'isolated unsolved export')
    need(by_id.get('export.explicit-reference',{}).get('returncode')==0,'explicit reference export')
    need(len(d.get('observations',[]))==10,'six executions/four object audits')
    need(len(d.get('limits',[]))==4,'evidence limitations')
    return errors

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--self-test',action='store_true');parser.add_argument('--output',type=pathlib.Path);a=parser.parse_args()
    report=json.loads((HERE/'validation.json').read_text(encoding='utf8'));errors=validate(report);controls=[]
    if a.self_test:
        changes=[('status',lambda x:x.update(status='fail')),
                 ('source',lambda x:x['source_sha256'].pop(next(iter(x['source_sha256'])))),
                 ('output',lambda x:x['outputs_sha256'].update({next(iter(x['outputs_sha256'])):'0'*64})),
                 ('missing',lambda x:x['commands'].pop(1)),
                 ('false-pass',lambda x:x['commands'][1].update(passed=False)),
                 ('returncode',lambda x:x['commands'][1].update(returncode=41)),
                 ('C17',lambda x:x['commands'][1]['command'].remove('-std=c17')),
                 ('oracle',lambda x:next(r for r in x['commands'] if r['id']=='gcc.debug.oracle').update(stdout='{}')),
                 ('mutant',lambda x:next(r for r in x['commands'] if r['id']=='gcc.debug.mutant1.reject').update(stdout='unrelated failure')),
                 ('compile-is-not-rejection',lambda x:next(r for r in x['commands'] if r['id']=='gcc.debug.mutant1.compile').update(returncode=1,expected_returncode=1)),
                 ('helpers',lambda x:next(r for r in x['commands'] if r['id']=='clang.m4-O3.undefined').update(stdout='U hidden_helper')),
                 ('limits',lambda x:x.update(limits=[]))]
        for name,change in changes:
            altered=copy.deepcopy(report);change(altered);rejected=bool(validate(altered));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('guard accepted '+name)
    result=dict(status='pass' if not errors else 'fail',errors=errors,controls=controls,campaign=sha(HERE/'validation.json'))
    if a.output:a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
