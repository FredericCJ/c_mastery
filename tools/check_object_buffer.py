"""Reject missing exact tasks, stale evidence and UB diagnostic/mastery conflation."""
import argparse,copy,functools,hashlib,json,pathlib,re
from close_object_buffer import ROOT,HERE,CARD,ARTIFACTS,REMAINING,original,rows,attach
import run_object_buffer as runner
@functools.lru_cache(None)
def cache(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cache(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,ext):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    c=read(ROOT/CARD);old=original();need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original criteria/baseline unchanged');need(attach(copy.deepcopy(c))==c,'idempotent pure attachment');need(ARTIFACTS[0] in c['shared_contracts'],'actual shared contract');need(d.get('updates')==rows(r) and len(d.get('updates',[]))==18,'exact tasks/delays/dimensions');need(d.get('remaining_parents')==REMAINING,'remaining scope')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'bound artifact '+n)
    for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for key in keys:
            need(bool(owner.get(key)),'nonempty '+key)
            for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact execution sources');need(r.get('external_sha256')==ext,'actual tool/runtime identities');need({'tools/check_object_buffer.py','tools/close_object_buffer.py',CARD,'research/v5/expansion/legacy_object_buffer/research_gate.json'}<=set(d.get('source_sha256',{})),'current guard/canonical/source bindings')
    deps=r.get('dependency_sha256',{});need(len(deps)>10 and any('/sysroot/usr/include/' in n for n in deps),'actual i386 headers')
    for n,h in deps.items():p=pathlib.Path(n);need(p.is_file() and sha(p)==h,'dependency '+n)
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact successful command inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual output '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'observed result '+n)
    def cmd(n,code,marker):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def args(n):return commands.get(n,{}).get('command',[])
    def inputs(n,paths):
        for p in paths:need(any(str(x).endswith('/'+p) for x in args(n)),'actual input '+n+' '+p)
    def packet(n,suffix):
        p=next((pathlib.Path(x) for x in args(n) if x.endswith(suffix)),None);need(p is not None and p.is_file() and p.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'actual retained packet '+n);return read(p) if p and p.is_file() else {}
    def oracle(t,k,count):
        n=t+'.'+k+'.oracle';cmd(n,0,'"observations": '+str(count));inputs(n,['assessor/object_followthrough/buffer/oracle.py']);need(k in args(n),'named independent model '+n);q=json.loads(logs.get(n,'{}'));need(q.get('kind')==k and q.get('observations')==count,'actual oracle result '+n);q=packet(n,'.packets.json');need(q.get('returncode')==0 and len(q.get('stdout','').splitlines())==count,'actual raw oracle '+n)
    for cc in ['gcc','clang']:
        for p,flags in runner.PROFILES:
            t=cc+'.'+p;need(all(x in args(t+'.compile') for x in ['-std=c17','-Werror','-Wpedantic',*flags]),'actual compiler profile '+t);inputs(t+'.compile',['assessor/object_followthrough/buffer/buffer.c','assessor/object_followthrough/buffer/workspace.c']);inputs(t+'.stub.compile',['labs/object_followthrough/buffer/learner.c'])
            for case in runner.CASES:cmd(t+'.'+case,0,'OB_'+case.upper()+'_PASS');cmd(t+'.stub.'+case,41,'OB_ASSERT:')
            oracle(t,'size',320);oracle(t,'history',2500)
            for m,(_,marker) in runner.MUTANTS.items():cmd(t+'.mutant.'+m,41,'OB_ASSERT:'+marker);need('-DOB_MUTANT_'+m in args(t+'.mutant.'+m+'.compile'),'actual intended mutant '+t+m)
        need(all(x in args(cc+'.fallback.compile') for x in ['-DOB_NO_VLA','-Wvla','-Werror']),'actual VLA-free policy '+cc);cmd(cc+'.fallback.workspace',0,'OB_WORKSPACE_PASS');need('evaluations=0' in logs.get(cc+'.fallback.workspace',''),'fallback has no VLA evaluation '+cc);cmd(cc+'.vla.forbidden',1,'error:');need('-Wvla' in args(cc+'.vla.forbidden'),'actual VLA diagnostic policy '+cc)
        for edition in ['c99','c11','c17','c23']:
            n=cc+'.'+edition+'.vla';need('-std='+edition in args(n+'.compile'),'actual edition mode '+n);inputs(n+'.compile',['assessor/object_followthrough/buffer/workspace.c','labs/object_followthrough/buffer/vla_probe.c']);cmd(n,0,'OB_EDITION:');need('evaluations=1' in logs.get(n,''),'actual local bound evaluation '+n)
        for p in ['O0','O3']:
            inputs(cc+'.warning.'+p,['labs/object_followthrough/buffer/warning.c']);need('-'+p in args(cc+'.warning.'+p) and '-c' in args(cc+'.warning.'+p),'warning source compile-only '+cc+p)
        for p in ['debug','optimized','sanitized']:
            n=cc+'.diagnostic.'+p+'.compile';inputs(n,['labs/object_followthrough/buffer/diagnostic.c']);need(('-O0' if p=='debug' else '-O3' if p=='optimized' else '-fsanitize=address,undefined') in args(n),'actual diagnostic instrumentation '+n)
            if p!='sanitized':cmd(cc+'.diagnostic.'+p+'.assembly',0,'<main>:')
        n=cc+'.diagnostics.observe';cmd(n,0,'"observations": 6');inputs(n,['assessor/object_followthrough/buffer/diagnostics.py']);q=packet(n,'.diagnostics.json');obs=q.get('observations',[]);need(q.get('status')=='pass' and len(obs)==6 and len(q.get('limits',[]))>=3,'actual bounded diagnostic observations '+cc)
        need({(v.get('profile'),v.get('branch')) for v in obs}=={(p,b) for p in ['debug','optimized','sanitized'] for b in ['initialized','uninitialized']},'full optimization/branch diagnostic cells '+cc)
        for v in obs:
            text=v.get('stdout','')+v.get('stderr','');need(v.get('uninitialized_report')==('uninitialised' in text) and v.get('asan_report')==('AddressSanitizer' in text) and v.get('ubsan_report')==('runtime error:' in text),'actual diagnostic classification '+cc)
            need(bool(v.get('command')) and v['command'][-1]==('1' if v['branch']=='initialized' else '0'),'actual branch argument '+cc)
            if v['profile']!='sanitized':need(v['command'][0]=='valgrind' and '--track-origins=yes' in v['command'] and '--error-exitcode=99' in v['command'],'actual Memcheck observation '+cc)
            if v['branch']=='initialized':need(v['exit_code']==0 and not(v['uninitialized_report'] or v['asan_report'] or v['ubsan_report']),'defined initialized positive '+cc)
            elif v['profile']=='debug':need(v['exit_code']==99 and 'Conditional jump or move depends on uninitialised value' in text and 'diagnostic.c' in text,'actual named uninitialized sensitivity '+cc)
        for case in ['initialization','relocation','handles']:
            n=cc+'.debug.memcheck.'+case;cmd(n,0,'ERROR SUMMARY: 0 errors');need(args(n) and args(n)[0]=='valgrind' and '--leak-check=full' in args(n),'actual clean positive Memcheck '+n)
    # Exact selected-version observation supports the documented optimizer counterexample.
    clang=packet('clang.diagnostics.observe','.diagnostics.json');selected=[q for q in clang.get('observations',[]) if q.get('profile')=='optimized' and q.get('branch')=='uninitialized'];need(len(selected)==1 and selected[0]['exit_code']==0 and not selected[0]['uninitialized_report'],'actual selected Clang optimized false negative')
    assembly=logs.get('clang.diagnostic.optimized.assembly','');main=re.search(r'<main>:(.*?)(?:\n\n|$)',assembly,re.S);need(main is not None and 'ob_diagnostic_sink' in main.group(1) and not re.search(r'\bcall\b',main.group(1)),'actual optimized missing access explained by assembly')
    need('numerical_address_reused=1' in logs.get('gcc.debug.relocation',''),'actual same-address realloc observation');need('padded_size=16,padded_offset=9' in logs.get('gcc.debug.sizing','') and 'padded_size=12,padded_offset=9' in logs.get('i386.sizing',''),'actual changed tail-padding layouts')
    cmd('i386.elf',0,'elf32-i386');need('-m32' in args('i386.compile') and '--sysroot='+str(runner.SYSROOT) in args('i386.compile'),'actual narrower ABI')
    for case in runner.CASES:cmd('i386.'+case,0,'OB_'+case.upper()+'_PASS');need(args('i386.'+case) and args('i386.'+case)[0]==str(runner.LOADER),'actual narrower loader '+case)
    oracle('i386','size',320);oracle('i386','history',2500)
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==8 and all(q.get('passed') is True for q in e['commands']),'actual isolated learner export rejection')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    records=read(HERE/'resources.json')['resources'];need({'R-OB-N1570','R-OB-DR400','R-OB-DR451','R-OB-N3096','R-OB-MEMCHECK','R-OB-GCC-WARN'}<={q['id'] for q in records},'named source authorities')
    for s in records:
        i=s['inspection'];need(i.get('scope')=='; '.join(s['locators']) and i.get('passages')==s['locators'] and i.get('method'),'bounded source inspection');cap=i.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'captured primary bytes')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected source '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-i386','lost-loader','lost-dependencies','lost-size','lost-history','lost-vla-fallback','lost-vla-diagnostic','lost-edition','lost-memcheck','lost-diagnostic-observation','lost-diagnostic-assembly','lost-diagnostic-scope','lost-remaining',*['mutant-'+m for m in runner.MUTANTS]]:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='easier task'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k.startswith('mutant-'):y['commands']=[q for q in y['commands'] if q['id']!='gcc.debug.mutant.'+k[7:]]
            elif k=='lost-i386':next(q for q in y['commands'] if q['id']=='i386.compile')['command'].remove('-m32')
            elif k=='lost-loader':next(q for q in y['commands'] if q['id']=='i386.handles')['command'][0]='host-model'
            elif k=='lost-dependencies':y['dependency_sha256']={}
            elif k=='lost-vla-fallback':next(q for q in y['commands'] if q['id']=='gcc.fallback.compile')['command'].remove('-DOB_NO_VLA')
            elif k=='lost-edition':next(q for q in y['commands'] if q['id']=='clang.c23.vla.compile')['command'].remove('-std=c23')
            elif k=='lost-diagnostic-scope':x['updates'][4]['author_validation']['diagnostic_scope']='Undefined output proves semantics'
            elif k=='lost-remaining':x['remaining_parents']=[]
            else:
                tags={'lost-size':'gcc.debug.size.oracle','lost-history':'gcc.debug.history.oracle','lost-vla-diagnostic':'gcc.vla.forbidden','lost-memcheck':'gcc.debug.memcheck.initialization','lost-diagnostic-observation':'clang.diagnostics.observe','lost-diagnostic-assembly':'clang.diagnostic.optimized.assembly'};y['commands']=[q for q in y['commands'] if q['id']!=tags[k]]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('source changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
