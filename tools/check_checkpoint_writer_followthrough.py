"""Exact task, source, backend, mutation, closure and non-inflation controls."""
import argparse,copy,functools,hashlib,json,pathlib,re
from close_checkpoint_writer_followthrough import ROOT,HERE,CARD,ARTIFACTS,REMAINING,original,rows,attach
import run_checkpoint_writer_followthrough as runner
@functools.lru_cache(None)
def cache(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cache(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,ext):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    card=read(ROOT/CARD);old=original();need(card['criteria']==old['criteria'] and card['baseline_binding']==old['baseline_binding'],'original criterion/baseline');need(attach(copy.deepcopy(card))==card,'idempotent attachment');need(d.get('updates')==rows(r) and len(d.get('updates',[]))==8,'eight exact ordinary/delayed rows');need(d.get('remaining_parents')==REMAINING,'remaining scope')
    for x,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for k in keys:
            need(bool(x.get(k)),'nonempty '+k)
            for n,h in x.get(k,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact campaign sources');need(r.get('external_sha256')==ext,'actual compiler tools')
    need({'tools/check_checkpoint_writer_followthrough.py','tools/close_checkpoint_writer_followthrough.py',CARD,'research/v5/expansion/legacy_checkpoints/author_supplements.py'}<=set(d.get('source_sha256',{})),'canonical producers bound')
    generator=data(ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py').decode();need('attach_writer_followthrough(' in generator and 'legacy_checkpoint_writer.author_support' in generator,'maintained hook')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need(n in d.get('source_sha256',{}),'bound artifact '+n)
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact complete executed inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'actual result '+n)
    def cmd(n,code,marker=None):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def args(n):return commands.get(n,{}).get('command',[])
    for cc,mode,flags in runner.PROFILES:
        t=cc+'.'+mode
        for n in ['build','progress.build','log.build']:
            need(all(f in args(t+'.'+n) for f in flags+['-std=c17','-ffunction-sections','-Wl,--gc-sections']),'actual profile '+t+n)
        for n,marker in [('byte','CWF_BYTE_PASS'),('span','CWF_SPAN_PASS'),('progress','CWF_PROGRESS_PASS'),('log','log_records=1')]:cmd(t+'.'+n,0,marker)
        need('CWF_SINK:2:65:66' in logs.get(t+'.byte',''),'exact struct in_transfer byte sink '+t)
        for n in ['progress','log']:
            text=logs.get(t+'.'+n,'');need(all('CWF_OS raw='+str(raw)+' cycles=3 callbacks=7 accepted=9 cause=9' in text for raw in [0,1]),'actual alternating OS backends '+t+n);need('callbacks=4 adapter_payload_copies=0 log_records='+str(n=='log' and 1 or 0) in text,'equal product/logger cost '+t+n)
            match=re.search(r'CWF_COST product=(\d+) result=(\d+) adapter=(\d+) core=(\d+)',text);need(match is not None and all(int(v)>0 for v in match.groups()),'actual product sizes '+t+n)
        need('-DCWF_LOG' in args(t+'.log.build') and '-DCWF_LOG' not in args(t+'.progress.build'),'separate logger removal '+t)
        for m,(case,macros,marker) in runner.MUTANTS.items():
            cmd(t+'.mutant.'+m,41,marker);need(all(v in args(t+'.mutant.'+m+'.build') for v in macros),'actual semantic mutant '+t+m)
        cmd(t+'.stub.byte',41,'CWF_ASSERT:');cmd(t+'.stub.span',41,'CWF_ASSERT:');need(any(x.endswith('/learner.c') for x in args(t+'.stub.build')),'actual unsolved scaffold '+t)
        shape=r.get('shapes',{}).get(t,{});need(set(shape)=={'cpw_step','cpx_progress_step','er_transfer','cwf_error'} and all(isinstance(s.get('static_conditional_branch_instructions'),int) and s['static_conditional_branch_instructions']>=0 for s in shape.values()),'actual generated branch sites '+t)
        for symbol in shape:
            match=re.search(r'<'+symbol+r'>:\n(.*?)(?=\n\n)',logs.get(t+'.shapes',''),re.S);need(match is not None,'actual symbol '+t+symbol)
            if match:need(sum(j not in ['jmp','jmpq'] for j in re.findall(r'\t(j[a-z]+)\s',match.group(1)))==shape[symbol]['static_conditional_branch_instructions'],'branch count attribution '+t+symbol)
        if mode!='asan':
            cmd(t+'.strict.run',0);need(all(x in args(t+'.strict.build') for x in ['-ffreestanding','-fno-builtin','-nostdlib','-nostartfiles','-nodefaultlibs','-static']),'strict dependency policy '+t);names=logs.get(t+'.strict.nm','');need(' in_drain' in names and ' cwf_byte_step' in names and not re.search(r'\bU\b|\b(malloc|free|printf|puts|write|__libc_start_main)\b',names),'actual strict closure '+t)
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==5 and all(q.get('passed') is True for q in e['commands']),'actual isolated export')
    for k in ['source_sha256','outputs_sha256']:
        for n,h in e.get(k,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    g=read(HERE/'research_gate.json');need(g.get('status')=='passage-inspected','source gate')
    for n,h in g['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    for s in read(HERE/'resources.json')['resources']:
        i=s['inspection'];need(i.get('scope')=='; '.join(s['locators']) and i.get('passages')==s['locators'] and i.get('method'),'actual inspection');cap=i.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'captured primary')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'inspected input '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-byte','lost-span','lost-raw-libc','lost-log','lost-strict','lost-closure','lost-stub','lost-requeue','lost-suffix','lost-short','lost-retry','lost-cause','lost-product','wrong-branches','lost-remaining']:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='placeholder'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k=='wrong-branches':y['shapes']['gcc.O0']['cpw_step']['static_conditional_branch_instructions']+=1
            elif k=='lost-remaining':x['remaining_parents']=[]
            else:
                tag={'lost-byte':'byte','lost-span':'span','lost-raw-libc':'progress','lost-log':'log','lost-strict':'strict.run','lost-closure':'strict.nm','lost-stub':'stub.span'}.get(k,'mutant.'+k.removeprefix('lost-'));y['commands']=[q for q in y['commands'] if q['id']!='gcc.O0.'+tag]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('source changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
