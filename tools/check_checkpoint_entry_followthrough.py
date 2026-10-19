"""Exact P2 entry tasks, actual input sensitivity and strict ELF closure guard."""
import argparse,copy,functools,hashlib,json,pathlib,re
from close_checkpoint_entry_followthrough import ROOT,HERE,CARD,ARTIFACTS,REMAINING,original,rows,attach
import run_checkpoint_entry_followthrough as runner
@functools.lru_cache(None)
def cache(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cache(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,ext):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    c=read(ROOT/CARD);old=original();need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original task/baseline identity');need(attach(copy.deepcopy(c))==c,'idempotent attachment');need(d.get('updates')==rows(r) and len(d.get('updates',[]))==6,'six exact ordinary/delayed tasks');need(d.get('remaining_parents')==REMAINING,'remaining scope')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need((ROOT/n).is_file() and n in d.get('source_sha256',{}),'bound artifact '+n)
    for owner,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for key in keys:
            need(bool(owner.get(key)),'nonempty '+key)
            for n,h in owner.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact source inputs');need(r.get('external_sha256')==ext,'actual tool identities');need({'tools/check_checkpoint_entry_followthrough.py','tools/close_checkpoint_entry_followthrough.py',CARD,'research/v5/expansion/legacy_checkpoints/author_supplements.py'}<=set(d.get('source_sha256',{})),'canonical producer/guard identity')
    generator=data(ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py').decode('utf8');need('attach_entry_followthrough(attach_memory_followthrough(attach_successor_support(record)))' in generator,'maintained canonical attachment hook')
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact executed command inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'actual result '+n)
    def cmd(n,code,marker=None):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def args(n):return commands.get(n,{}).get('command',[])
    def packet(n,suffix):
        p=next((pathlib.Path(x) for x in args(n) if x.endswith(suffix)),None);need(p is not None and p.is_file() and p.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'actual retained packet '+n);return read(p) if p and p.is_file() else {}
    for cc in ['gcc','clang']:
        for opt in ['O0','O3']:
            for shape in ['fixed','pie']:
                t=cc+'.'+opt+'.'+shape;pie=shape=='pie'
                for unit in ['image','writer','helpers','aggregate']:need(all(x in args(t+'.'+unit+'.compile') for x in ['-'+opt,'-std=c17','-ffreestanding','-fno-builtin','-fno-stack-protector','-fPIE' if pie else '-fno-pie']),'strict object profile '+t+unit)
                for unit in ['entry','image','writer','helpers','aggregate']:
                    cmd(t+'.'+unit+'.object',0,'CE_ELF_PASS');obj=packet(t+'.'+unit+'.object','.object.json');need(obj.get('kind')==1,'actual relocatable object '+t+unit)
                    if unit=='aggregate':need('memcpy' in obj.get('undefined',''),'actual compiler-emitted helper '+t)
                    if unit=='image':need(any(s['name']=='.bss' and s['type']==8 and s['size']>=131072 for s in obj.get('sections',[])) and any(s['name']=='.data' and s['size']>=4 for s in obj.get('sections',[])),'actual BSS/data object sections '+t)
                need(all(x in args(t+'.link') for x in ['-nostdlib','-nostartfiles','-nodefaultlibs','-static-pie' if pie else '-static']),'explicit strict link '+t);cmd(t+'.audit',0,'CE_ELF_PASS');elf=packet(t+'.audit','.elf.json');need(elf.get('kind')==(3 if pie else 2) and elf.get('undefined')=='' and any(s['memory_bytes']>s['file_bytes'] for s in elf.get('loads',[])),'actual final image closure '+t)
                cmd(t+'.execute',0,'"cases": 8');q=packet(t+'.execute','.execution.json');obs=q.get('observations',[]);need(q.get('status')=='pass' and q.get('pie')==pie and len(obs)==8,'actual full changed input packet '+t)
                if len(obs)==8:
                    need([x['exit_code'] for x in obs]==[0,0,0,0,0,71,70,72],'actual admission failures '+t);need(obs[3]['arguments']==['AB','changed'] and obs[3]['stdout']=='ABchanged' and obs[3]['argc']==3 and obs[1]['test']==66 and obs[0]['test']==65,'changed inputs consumed '+t)
                for index,v in enumerate(obs):
                    trace=pathlib.Path(v.get('trace','missing'));need(trace.is_file() and trace.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'bound actual syscall trace '+t);text=data(trace).decode('utf8') if trace.is_file() else ''
                    if v.get('exit_code')==0:
                        bias=v.get('load_bias');need(isinstance(bias,int) and bias>=0 and bias%4096==0 and (bias!=0 if pie else bias==0),'actual load bias '+t);symbols=q.get('symbols',{});need(v.get('runtime_data',-1)-symbols.get('cpw_data',0)==bias and v.get('runtime_entry',-1)-symbols.get('_start',0)==bias,'two independent symbol bias derivations '+t);need(v.get('stdout')==''.join(v['arguments']) and v.get('argc')==len(v['arguments'])+1 and v.get('envc')==len(v['environment']),'actual argc/env/output effect '+t);need('EBADF' in text and 'exit(0)' in text,'actual syscall/error/exit '+t)
                    else:need(not v.get('stdout') and not v.get('stderr') and 'exit('+str(v['exit_code'])+')' in text,'rejection before publication '+t)
                assembly=logs.get(t+'.assembly','');start=re.search(r'<_start>:\n(.*?)(?=\n\n)',assembly,re.S);need(start is not None and all(x in start.group(1) for x in ['%rsp,%rdi','call','syscall']),'actual entry bridge '+t)
                cmd(t+'.missing-helper',1,'memcpy');cmd(t+'.forbidden.audit',41,'CE_AUDIT:forbidden resolved provider');cmd(t+'.bad-entry.audit',41,'CE_AUDIT:entry outside executable PT_LOAD');cmd(t+'.bad-extent.audit',41,'CE_AUDIT:invalid load extent or writable executable');cmd(t+'.learner.reject',93)
                if pie:cmd(t+'.relocation.audit',41,'CE_AUDIT:unhandled relocation');need('-DCE_BAD_RELOCATION' in args(t+'.relocation.compile'),'actual relocation source control '+t)
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==5 and all(q.get('passed') is True for q in e['commands']),'actual isolated exported entry rejection')
    for key in ['source_sha256','outputs_sha256']:
        for n,h in e.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    for s in read(HERE/'resources.json')['resources']:
        i=s['inspection'];need(i.get('scope')=='; '.join(s['locators']) and i.get('passages')==s['locators'] and i.get('method'),'actual source inspection');cap=i.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'captured primary bytes')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'actual inspected input '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();errors=validate(d,r,pins,ext);controls=[]
    if a.self_test and not errors:
        for k in ['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-check','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','lost-object','lost-execution','lost-entry','lost-pie','lost-strict','lost-helper','lost-forbidden','lost-relocation','lost-bad-entry','lost-bad-extent','lost-learner','lost-remaining']:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='ignored placeholder'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-check':x['updates'][0]['author_validation']['checks']=['gcc.version']
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key='source_sha256' if k=='stale-source' else 'outputs_sha256';y[key][next(iter(y[key]))]='0'*64
            elif k=='lost-strict':next(q for q in y['commands'] if q['id']=='gcc.O0.fixed.link')['command'].remove('-nodefaultlibs')
            elif k=='lost-pie':next(q for q in y['commands'] if q['id']=='gcc.O0.pie.link')['command'].remove('-static-pie')
            elif k=='lost-remaining':x['remaining_parents']=[]
            else:
                tags={'lost-object':'gcc.O0.fixed.image.object','lost-execution':'gcc.O0.fixed.execute','lost-entry':'gcc.O0.fixed.assembly','lost-helper':'gcc.O0.fixed.missing-helper','lost-forbidden':'gcc.O0.fixed.forbidden.audit','lost-relocation':'gcc.O0.pie.relocation.audit','lost-bad-entry':'gcc.O0.fixed.bad-entry.audit','lost-bad-extent':'gcc.O0.fixed.bad-extent.audit','lost-learner':'gcc.O0.fixed.learner.reject'};y['commands']=[q for q in y['commands'] if q['id']!=tags[k]]
            rejected=bool(validate(x,y,pins,ext));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=ext:errors.append('source changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
