"""Exact Linux-capstone task/evidence guard, including deliberate false packets."""
import argparse,copy,functools,hashlib,json,pathlib
from close_capstone_linux import ROOT,HERE,PROJECTS,card,original,rows,REMAINING,attach
import run_capstone_linux as runner
@functools.lru_cache(None)
def cached(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cached(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,external):
    errors=[]
    def need(ok,message):
        if not ok:errors.append(message)
    need(d.get('updates')==rows(r) and len(d.get('updates',[]))==24,'exact twelve ordinary/delayed pairs');need(d.get('remaining_parents')==REMAINING,'remaining evidence limits')
    for p in PROJECTS:
        c=read(ROOT/card(p));old=original(p);need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'preserved '+p);need(attach(copy.deepcopy(c))==c,'idempotent '+p)
    for x,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for k in keys:
            need(bool(x.get(k)),'nonempty '+k)
            for n,h in x.get(k,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact scoped source inputs');need(r.get('external_sha256')==external,'actual tool identities')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need(n in d.get('source_sha256',{}),'bound row artifact '+n)
    cmds={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(cmds)==len(r.get('commands',[])) and set(cmds)==runner.expected_commands() and all(q.get('passed') is True for q in cmds.values()),'exact executed command inventory')
    logs={}
    for n,q in cmds.items():
        p=ROOT/q.get('output','missing');t=data(p).decode('utf8') if p.is_file() else '';logs[n]=t;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in t),'actual result '+n)
    def args(n):return cmds.get(n,{}).get('command',[])
    def result(n,code,marker=None):q=cmds.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    for cc,mode,flags in runner.PROFILES:
        t=cc+'.'+mode
        for kind in ['core','modern','linux','compose']:need(all(f in args(t+'.'+kind+'.build') for f in flags+['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic']),'actual profile '+t+kind)
        for name,kind,macro,case,marker in runner.MUTANTS:need(macro in args(t+'.mutant.'+name+'.build'),'actual mutant '+t+name);result(t+'.mutant.'+name,1,marker)
        for name,marker in [('stub','CAP_ASSERT:cs_init'),('stub-modern','CAP_MODERN:p.next==cursors[i]'),('stub-linux','CAP_LINUX:!csl_open')]:result(t+'.'+name,1,marker);need(any(s.endswith(('learner.c','modern_stub.c','linux_stub.c')) for s in args(t+'.'+name+'.build')),'compiled learner '+name)
        need('framing-splits=891' in logs.get(t+'.core.framing',''),'all framing splits '+t);need('backing=252' in logs.get(t+'.core.pressure',''),'separate backing '+t)
        for name,trace,code in [('authority','U',41),('retained','ABLX',42)]:
            result(t+'.minimize.'+name,0,'CAP_MINIMIZED '+name);p=next((pathlib.Path(v) for v in args(t+'.minimize.'+name) if v.endswith('.json')),None);need(p is not None and p.is_file(),'minimized packet '+t+name)
            if p is not None and p.is_file():
                z=read(p);need(z.get('passed') is True and z.get('minimal')==trace and z.get('minimality')=='single-command deletion for this property, not global proof' and any(o.get('trace')==trace and o.get('exit_code')==code for o in z.get('observations',[])),'actual property minimality '+t+name)
        p=next((pathlib.Path(v) for v in args(t+'.fuzz') if v.endswith('.json')),None);need(p is not None and p.is_file(),'independent oracle packet '+t)
        if p is not None and p.is_file():need(read(p).get('passed') is True and len(read(p).get('observations',[]))==516,'516 bounded oracle cells '+t)
    for cc in ['gcc','clang']:
        for n in ['modern','compose','race']:need('-fsanitize=thread' in args(cc+'.tsan.'+n+'.build'),'actual TSan compile '+cc+n)
        result(cc+'.tsan.race',66,'WARNING: ThreadSanitizer: data race')
        for n in ['modern','compose']:need('WARNING: ThreadSanitizer' not in logs.get(cc+'.tsan.'+n,''),'synchronized target '+cc+n)
        for shared in ['static','shared']:
            t=cc+'.package.'+shared;need('-DCAP_BUILD_ADAPTER=OFF' in args(t+'.configure') and '-DCAP_BUILD_EXAMPLES=OFF' in args(t+'.configure'),'minimal source export '+t);result(t+'.private-reject',1,'release_active');result(t+'.generated-header-reject',1,'private_generated_not_installed.h');result(t+'.relocated',0,'CAP_INSTALLED cases=68')
            if shared=='shared':need('-Wl,-rpath,$ORIGIN/../lib' in args(t+'.consumer.build') and 'libcapcore.so.1' in logs.get(t+'.dependencies',''),'explicit shared runtime '+t)
        for opt in ['O0','O3']:
            t=cc+'.target.'+opt;need(all(f in args(t+'.link') for f in ['-nostdlib','-nostartfiles','-nodefaultlibs']),'strict link '+t);need(not logs.get(t+'.undefined','').strip(),'empty undefined '+t);result(t+'.run',0,'CAP_TARGET_PASS');need(any('loader,file=' in a for a in args(t+'.run')),'actual poisoned startup '+t);need(any(a.endswith('/target_helpers.c') for a in args(t+'.copyhelper.compile')),'explicit compiler helper '+t);result(t+'.leaked-header',1,'sys/epoll.h')
    for row in d.get('updates',[]):need(set(row.get('author_validation',{}).get('checks',[]))<=set(cmds),'objective-specific executed checks')
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==36 and all(q.get('passed') is True for q in e['commands']),'four actual learner exports')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','actual source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    for s in read(HERE/'resources.json')['resources']:
        i=s['inspection'];need(i.get('passages')==s['locators'] and i.get('scope')=='; '.join(s['locators']),'actual exact source scope');need(sha(ROOT/i['capture']['path'])==i['capture']['sha256'],'captured source')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'inspected input '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        for name in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-checks','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','stale-tool','lost-authority','lost-release','lost-fuzz','lost-installed','lost-target','lost-tsan','wrong-tsan','wrong-mutation','lost-remaining','lost-coro','lost-real-worker']:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if name=='missing-criterion':x['updates'].pop(0)
            elif name=='missing-delayed':x['updates'].pop()
            elif name=='changed-task':x['updates'][0]['original_task']='placeholder'
            elif name=='changed-delay':x['updates'][1]['minimum_days']=0
            elif name=='wrong-checks':x['updates'][0]['author_validation']['checks']=[]
            elif name.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[name]]['state']='passed'
            elif name.startswith('stale-'):k={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-tool':'external_sha256'}[name];y[k][next(iter(y[k]))]='0'*64
            elif name=='wrong-tsan':next(q for q in y['commands'] if q['id']=='gcc.tsan.modern.build')['command'].remove('-fsanitize=thread')
            elif name=='wrong-mutation':next(q for q in y['commands'] if q['id']=='gcc.O0.mutant.authority.build')['command'].remove('-DCS_MUTANT=1')
            elif name=='lost-remaining':x['remaining_parents']=[]
            else:
                missing={'lost-authority':'gcc.O0.minimize.authority','lost-release':'gcc.O0.minimize.retained','lost-fuzz':'gcc.O0.fuzz','lost-installed':'gcc.package.static.relocated','lost-target':'gcc.target.O0.run','lost-tsan':'gcc.tsan.race','lost-coro':'gcc.O0.compose.coroutine','lost-real-worker':'gcc.O0.compose.p5'}[name];y['commands']=[q for q in y['commands'] if q['id']!=missing]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=name,rejected=rejected))
            if not rejected:errors.append(name+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('inputs changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
