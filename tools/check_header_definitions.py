"""Reject incomplete, stale or inflated PR-T02-04 implementation evidence."""
import argparse,copy,functools,hashlib,json,pathlib
from close_header_definitions import ROOT,HERE,CARD,original,rows,ARTIFACTS,attach
import run_header_definitions as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    c=read(ROOT/CARD);old=original();need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'exact original criteria/baseline');need(attach(copy.deepcopy(c))==c,'idempotent attachment');need(ARTIFACTS[0] in c['shared_contracts'],'required public contract')
    need(d.get('updates')==rows(r) and len(d.get('updates',[]))==12,'all exact ordinary/delayed tasks, mappings and dimensions')
    need(d.get('remaining_parents')==['Exact final published C17/C23 source access remains open.','External review, independent learner delayed assessment and physical observations remain separate.'],'honest remaining dimensions')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'bound actual artifact '+n)
    for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for key in keys:
            need(bool(owner.get(key)),'nonempty '+key)
            for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact code/source closure');need(r.get('external_sha256')==external,'actual current compiler/libc identities')
    need({'tools/check_header_definitions.py','tools/close_header_definitions.py',CARD,'research/v5/expansion/legacy_language_headers/research_gate.json'}<=set(d.get('source_sha256',{})),'canonical guards/source bindings')
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact passed command inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text
        need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n)
        expected=q.get('expected_exit');need((expected is None or q.get('exit_code')==expected) and(q.get('required_marker') is None or q['required_marker'] in text),'actual command outcome '+n)
    def cmd(n,code,marker):
        q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def inputs(n,paths):
        args=commands.get(n,{}).get('command',[])
        for p in paths:need(any(str(x).endswith('/'+p) for x in args),'actual input '+n+' '+p)
    obs=[]
    for cc in ['gcc','clang']:
        for profile,flags in runner.PROFILES:
            t=cc+'.'+profile;args=commands.get(t+'.compile',{}).get('command',[]);need('-std=c17' in args and '-Werror' in args and all(f in args for f in flags),'actual behavioral profile '+t);inputs(t+'.compile',['assessor/language_followthrough/headers/headers.c','labs/language_followthrough/headers/consumer_a.c','labs/language_followthrough/headers/consumer_b.c']);inputs(t+'.stub.compile',['labs/language_followthrough/headers/learner.c']);need('-I'+str(runner.PUBLIC) in commands.get(t+'.stub.compile',{}).get('command',[]),'public unfinished header '+t)
            cmd(t+'.definitions',0,'HH_DEFINITIONS_PASS');cmd(t+'.state',0,'HH_LOCAL_STATE_PASS')
            for name in ['definitions','state']:cmd(t+'.stub.'+name,41,'HH_ASSERT:')
            for m,(_,marker) in runner.MUTANTS.items():cmd(t+'.mutant.'+m,41,'HH_ASSERT:'+marker);need('-DHH_MUTANT_'+m in commands.get(t+'.mutant.'+m+'.compile',{}).get('command',[]),'actual semantic defect '+t+m)
        for opt in ['O0','O3']:
            t=cc+'.static.'+opt;cmd(t+'.missing',1,'hh_read');cmd(t+'.duplicate',1,'hh_read');s=json.loads(logs.get(t+'.symbols','{}'));need(s.get('expected')=='two internal definitions; one external definition' and s.get('a',{}).get('hh_mix')==['t'] and s.get('b',{}).get('hh_mix')==['t'],'actual internal symbol distinction '+t)
            for dialect,dflags in runner.DIALECTS:
                for form in ['inline','extern']:
                    t=cc+'.'+dialect+'.'+opt+'.'+form;emits=(form=='extern')!=(dialect=='gnu89-inline');kind='T' if emits else 'U'
                    for part in ['a','b']:
                        args=commands.get(t+'.'+part,{}).get('command',[]);need(all(f in args for f in [*dflags,'-'+opt,'-DHH_EXTERN='+str(int(form=='extern')),'-DHH_CALL_ONLY=0']),'actual dialect/address/optimization '+t)
                        need(any(z.split()[:2]==['hh_experiment',kind] for z in logs.get(t+'.'+part+'.nm','').splitlines()),'retained exact symbol '+t+part)
                    cmd(t+'.symbols',0,'"expected": "'+kind+'"');cmd(t+'.link',1,'hh_experiment')
                    if not emits:cmd(t+'.repaired.run',0,None);inputs(t+'.definition',['labs/language_followthrough/headers/probes/inline_external.c'])
                    obs.append(dict(id=t,expected_definition=kind,missing_or_duplicate='duplicate' if emits else 'missing',address_taken=True))
                t=cc+'.'+dialect+'.'+opt+'.callonly';q=commands.get(t+'.link',{});code=q.get('exit_code');need(code in [0,1] and q.get('expected_exit') is None,'separate observed call-only link '+t)
                if code==1:need('hh_experiment' in logs.get(t+'.link',''),'relevant failed call-only link '+t)
                exe=str(runner.ROOT/pathlib.Path(q.get('command',['missing'])[-1]));need(not any(z.get('command',[None])[0]==exe for z in commands.values()),'unexecuted incomplete definition program '+t)
                obs.append(dict(id=t,observed_link_exit=code,executed=False,iso_external_definition_obligation='still present; no definition supplied',gnu89_limit='duplicate-definition dialect experiment' if dialect=='gnu89-inline' else None))
        for locale in ['C','C.utf8']:
            for name in ['syntax','redeclaration','arity','type','repair']:
                t=cc+'.diagnostic.'+locale+'.'+name;cmd(t,0 if name=='repair' else 1,None if name=='repair' else name+'.c:');need(commands.get(t,{}).get('locale')==locale,'actual locale '+t)
                if name!='repair':need('error:' in logs.get(t,'') and {'syntax':'expected','redeclaration':'hh_conflict','arity':'hh_arity','type':'int-conversion'}[name] in logs.get(t,'') and not any(w in logs.get(t,'').lower() for w in ['no such file','unknown argument','unrecognized command-line']),'intended diagnostic class '+t)
        for name in ['syntax','redeclaration','arity','type','repair','runtime']:
            t=cc+'.permissive.'+name;cmd(t,1 if name in ['syntax','redeclaration','arity'] else 0,name+'.c:' if name in ['syntax','redeclaration','arity','type'] else None);need('-std=gnu17' in commands.get(t,{}).get('command',[]),'declared permissive mode '+t)
        cmd(cc+'.constraint.warning',0,'int-conversion');cmd(cc+'.extension.strict',1,'extension.c:');cmd(cc+'.extension.gnu',0,None);cmd(cc+'.runtime.valid',0,'HH_VALUE:40');cmd(cc+'.runtime.invalid',77,'AddressSanitizer: stack-buffer-overflow');need('-fsanitize=address' in commands.get(cc+'.runtime.sanitized.compile',{}).get('command',[]),'actual isolated sanitizer '+cc)
        for name in ['first','reverse','masked','installed.compile','installed.run']:cmd(cc+'.header.'+name,0,None)
        cmd(cc+'.header.missing',1,'size_t');args=commands.get(cc+'.header.installed.compile',{}).get('command',[]);need('-I.' in args and not any(x.startswith('-I') and x!='-I.' for x in args) and 'libhh.a' in args,'isolated installed client '+cc)
    cmd('analyzer.capability',0,'security.ArrayBound')
    for mode,name in [(0,'unknown'),(1,'known'),(2,'repaired')]:
        t='analyzer.'+name;cmd(t,0,'security.ArrayBound' if mode==1 else None);args=commands.get(t,{}).get('command',[]);need('-DHH_MODEL='+str(mode) in args and '--analyze' in args and '-analyzer-checker=security.ArrayBound' in args,'actual analyzer assumption '+t);need(('security.ArrayBound' in logs.get(t,''))==(mode==1),'analyzer finding/silence '+t);obs.append(dict(id=t,warning=mode==1,scope='selected Clang checker with stated path constraint; silence is not memory safety'))
    need(r.get('observations')==obs,'exact observation and uncertainty inventory')
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==9 and all(q.get('passed') is True for q in e['commands']),'actual public export controls')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate pin '+n)
    for s in read(HERE/'resources.json')['resources']:
        ins=s['inspection'];need(ins.get('scope')=='; '.join(s['locators']) and ins.get('passages')==s['locators'] and ins.get('method'),'actual bounded source inspection');cap=ins.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'primary capture '+cap['path'])
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'inspected actual source '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();external=runner.external();errors=validate(d,r,pins,external);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-inline-mode','lost-address','lost-optimization','lost-locale','lost-installation','lost-runtime','lost-analyzer','lost-repair','lost-stub','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='easier replacement'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutant.'+k[7:]]
            elif k=='lost-inline-mode':next(q for q in y['commands'] if q['id']=='gcc.gnu89-inline.O0.inline.a')['command'].remove('-fgnu89-inline')
            elif k=='lost-address':next(q for q in y['commands'] if q['id']=='gcc.c17.O0.inline.a')['command'].remove('-DHH_CALL_ONLY=0')
            elif k=='lost-optimization':next(q for q in y['commands'] if q['id']=='clang.c17.O3.inline.a')['command'].remove('-O3')
            elif k=='lost-locale':next(q for q in y['commands'] if q['id']=='clang.diagnostic.C.utf8.type')['locale']='C'
            elif k=='lost-installation':next(q for q in y['commands'] if q['id']=='gcc.header.installed.compile')['command'].append('-Iassessor/language_followthrough/headers')
            elif k=='lost-analyzer':next(q for q in y['commands'] if q['id']=='analyzer.known')['command'].remove('-DHH_MODEL=1')
            elif k in ['lost-runtime','lost-repair','lost-stub']:
                tag={'lost-runtime':'clang.runtime.invalid','lost-repair':'gcc.diagnostic.C.repair','lost-stub':'gcc.debug.stub.state'}[k];y['commands']=[q for q in y['commands'] if q['id']!=tag]
            else:x['remaining_parents']=[]
            rejected=bool(validate(x,y,pins,external));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=external:errors.append('sources changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
