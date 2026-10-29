"""Exact scoped guards for cold cacheable boot; never promotes model into silicon evidence."""
import argparse
import copy
import hashlib
import json
import pathlib
from run_cacheable_boot_depth import ROOT,REPORT,inputs,sha

def validate(d,files=True):
    errors=[]
    def need(ok,why):
        if not ok:errors.append(why)
    need(d.get('passed') is True and d.get('source_stable') is True,'passing stable report')
    need(d.get('physical_observation')=='not-run' and d.get('independent_learner')=='not-assessed','external evidence boundary')
    if files:need(d.get('input_sha256')==inputs(),'current complete source closure')
    profiles=d.get('profiles',[]);need(len(profiles)==6 and {(p.get('compiler'),p.get('profile')) for p in profiles}=={(c,p) for c in ['gcc','clang'] for p in ['debug','release','sanitize']},'six exact profiles')
    need(all(p.get('passed') and p.get('language')=='c17' and p.get('mutants')==[1,2,3,4,5,6] and p.get('learner_rejected') for p in profiles),'host controls and baseline')
    commands=d.get('commands',[]);byid={r.get('id'):r for r in commands}
    need(len(byid)==len(commands) and commands and all(r.get('passed') for r in commands),'unique passing commands')
    for c in ['gcc','clang']:
        for profile in ['debug','release','sanitize']:
            tag=c+'-'+profile
            for m in [0,1,2,3,4,5,6,'stub']:
                compile=byid.get(tag+'.compile.'+str(m),{});run=byid.get(tag+'.execute.'+str(m),{})
                need(compile.get('returncode')==0,'actual compiled '+tag+str(m))
                need(run.get('returncode')==(0 if m==0 else 41) and ('CB_PASS:' if m==0 else 'CB_CHECK:') in run.get('stdout',''),'semantic outcome '+tag+str(m))
            oracle=byid.get(tag+'.independent-oracle',{})
            need(oracle.get('returncode')==0 and '"independent_set_way_words": 3968' in oracle.get('stdout',''),'independent oracle '+tag)
    targets=d.get('target_audits',[])
    need(len(targets)==8 and {(t.get('compiler'),t.get('optimization'),t.get('core')) for t in targets}=={(c,o,k) for c in ['arm-none-eabi-gcc','clang'] for o in ['-O0','-O3'] for k in [0,1]},'eight selected targets')
    for t in targets:
        need(t.get('passed') and t.get('undefined')==[] and t.get('peer_bytes')==32 and t.get('peer_address')=='0x30040000' and t.get('execution')=='not-run','target scope')
        if files:need((ROOT/t['elf']).is_file() and sha(ROOT/t['elf'])==t['sha256'],'current target ELF')
        tag=t.get('compiler','')+t.get('optimization','')+'-core'+str(t.get('core'))
        need(not byid.get(tag+'.undefined',{}).get('stdout','MISSING').strip(),'target helper closure')
        section=byid.get(tag+'.sections',{}).get('stdout','')
        need(any('.peer' in l and 'NOBITS' in l and '30040000' in l and '000020' in l for l in section.splitlines()),'actual target reserved section')
    guests=d.get('cpu_model_executions',[])
    need(len(guests)==4 and {(g.get('core'),g.get('optimization')) for g in guests}=={(c,o) for c in [0,1] for o in ['-O0','-O3']},'four struct lr_cpu model executions')
    for g in guests:
        r=byid.get('guest'+str(g['core'])+g['optimization']+'.execute',{})
        need(r.get('returncode')==0 and 'CB_GUEST_PASS:' in r.get('stdout','') and 'not actual STM32' in g.get('scope',''),'struct lr_cpu execution and limit')
    return errors

def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--output',type=pathlib.Path);a=p.parse_args()
    d=json.loads(REPORT.read_text(encoding='utf8'));errors=validate(d);controls=[]
    if a.self_test:
        edits=[('failed',lambda x:x.update(passed=False)),('stale',lambda x:x.update(source_stable=False)),
               ('profile',lambda x:x['profiles'].pop()),('language',lambda x:x['profiles'][0].update(language='c11')),
               ('mutant',lambda x:x['profiles'][0]['mutants'].pop()),('stub',lambda x:x['profiles'][0].update(learner_rejected=False)),
               ('command',lambda x:x['commands'].pop()),('target',lambda x:x['target_audits'].pop()),
               ('physical',lambda x:x.update(physical_observation='passed')),('learner',lambda x:x.update(independent_learner='accepted')),
               ('target-execution',lambda x:x['target_audits'][0].update(execution='passed')),('helper',lambda x:x['target_audits'][0].update(undefined=['memcpy'])),
               ('line',lambda x:x['target_audits'][0].update(peer_bytes=64)),('guest',lambda x:x['cpu_model_executions'].pop())]
        for name,edit in edits:
            mutant=copy.deepcopy(d);edit(mutant);rejected=bool(validate(mutant,False));controls.append(dict(name=name,rejected=rejected))
            if not rejected:errors.append('accepted corrupt report '+name)
    result=dict(passed=not errors,errors=errors,negative_controls=controls,report=REPORT.relative_to(ROOT).as_posix(),report_sha256=sha(REPORT))
    if a.output:a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
    print(json.dumps(result));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
