"""Guard all eighteen exact tasks, retained mechanism evidence and actual capacity replay."""
import argparse,copy,functools,hashlib,json,pathlib
from close_checkpoint_protocol_followthrough import ROOT,HERE,PROJECTS,card,REMAINING,original,rows,attach
import run_checkpoint_protocol_followthrough as runner
@functools.lru_cache(None)
def cache(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return cache(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def validate(d,r,pins,ext,binding):
    errors=[]
    def need(ok,s):
        if not ok:errors.append(s)
    for project in PROJECTS:
        c=read(ROOT/card(project));old=original(project);need(c['criteria']==old['criteria'] and c['baseline_binding']==old['baseline_binding'],'original tasks/baseline '+project);need(attach(copy.deepcopy(c))==c,'idempotent attachment '+project)
    need(d.get('updates')==rows(r) and len(d.get('updates',[]))==18,'eighteen exact ordinary/delayed rows');need(d.get('remaining_parents')==REMAINING,'remaining scope')
    for x,keys in [(d,['source_sha256']),(r,['source_sha256','outputs_sha256'])]:
        for k in keys:
            need(bool(x.get(k)),'nonempty '+k)
            for n,h in x.get(k,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale '+n)
    need(r.get('source_sha256')==pins and set(pins)<=set(d.get('source_sha256',{})),'exact source inputs');need(r.get('external_sha256')==ext,'actual tools');need(r.get('retained_reports')==binding and set(binding)==set(runner.RETAINED),'exact current retained evidence')
    need({'tools/check_checkpoint_protocol_followthrough.py','tools/close_checkpoint_protocol_followthrough.py','research/v5/expansion/legacy_checkpoints/author_supplements.py',*[card(p) for p in PROJECTS]}<=set(d.get('source_sha256',{})),'canonical producers/guard bound')
    gen=data(ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py').decode();need('attach_protocol_followthrough(' in gen and 'legacy_checkpoint_protocol.author_support' in gen,'maintained attachment hook')
    for row in d.get('updates',[]):
        for n in row.get('artifacts',[]):need(n in d.get('source_sha256',{}),'bound artifact '+n)
    commands={q['id']:q for q in r.get('commands',[])};need(r.get('status')=='pass' and r.get('failure') is None and len(commands)==len(r.get('commands',[])) and set(commands)==runner.expected_commands() and all(q.get('passed') is True for q in commands.values()),'exact executed inventory');logs={}
    for n,q in commands.items():
        p=ROOT/q.get('output','missing');text=data(p).decode('utf8') if p.is_file() else '';logs[n]=text;need(p.is_file() and sha(p)==q.get('output_sha256'),'actual log '+n);need(q.get('exit_code')==q.get('expected_exit') and(q.get('required_marker') is None or q['required_marker'] in text),'actual result '+n)
    def cmd(n,code,marker=None):q=commands.get(n,{});need(q.get('exit_code')==code and q.get('expected_exit')==code and q.get('required_marker')==marker,'intended result '+n)
    def args(n):return commands.get(n,{}).get('command',[])
    cmd('retained.check',0,'CPA_RETAINED_PASS')
    for cc,mode,flags in runner.PROFILES:
        t=cc+'.'+mode
        for variant in ['reference','skip','drop','stub','policy']:need(all(f in args(t+'.'+variant+'.build') for f in flags+['-std=c17','-Wall','-Wextra','-Werror','-Wpedantic']),'actual profile '+t+variant)
        need('-DCP_MUTANT=3' in args(t+'.skip.build') and '-DCPA_MUTANT_DROP' in args(t+'.drop.build'),'actual two distinct faults '+t);need(any(s.endswith('/learner.c') for s in args(t+'.stub.build')),'actual unsolved scaffold '+t)
        cmd(t+'.drop',41,'CPA_ASSERT:');cmd(t+'.stub',41,'CPA_ASSERT:');cmd(t+'.policy',0,'CPA_POLICY_PASS original32/restricted32 refuse; configured64 accepts version1 length33');need('-DCPX_MAX_PAYLOAD=64' in args(t+'.policy.build'),'actual policy configuration '+t);cmd(t+'.replay',0,'CPA_REPLAY_PASS12')
        packet=next((pathlib.Path(s) for s in args(t+'.replay') if s.endswith('.replay.json')),None);need(packet is not None and packet.is_file() and packet.relative_to(ROOT).as_posix() in r.get('outputs_sha256',{}),'bound actual replay packet '+t)
        if packet is not None and packet.is_file():
            p=read(packet);obs=p.get('observations',[]);need(p.get('status')=='pass' and p.get('receipt_sha256')==binding[runner.RETAINED[-1]] and len(obs)==12,'retained exact minimizer identity '+t);need({(q['seed'],q['chunk'],q['capacity']) for q in obs}=={(seed,chunk,cap) for seed in ['original_hex','minimized_hex'] for chunk in [1,7,256] for cap in [1,2]},'all changed-schedule cells '+t)
            for q in obs:
                good=q['reference'];bad=q['faulty'];cap=q['capacity'];prefix=[str(900+i)+':' for i in range(cap)];need(good['exit_code']==bad['exit_code']==0 and good['frames']==q['expected'] and '7:' in good['frames'] and '7:' not in bad['frames'] and good['frames'][:cap]==bad['frames'][:cap]==prefix,'normal-exit missing nested frame '+t);need(good['capacity']==bad['capacity']==good['peak']==bad['peak']==cap and good['refusals']>0 and good['turns']<=good['bound'] and bad['turns']<=bad['bound'],'actual bounded occupancy/refusal '+t)
    e=read(HERE/'exports.json');need(e.get('status')=='pass' and len(e.get('commands',[]))==10 and all(q.get('passed') is True for q in e['commands']),'both actual isolated exports')
    for k in ['source_sha256','outputs_sha256']:
        for n,h in e.get(k,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'current export '+n)
    gate=read(HERE/'research_gate.json');need(gate.get('status')=='passage-inspected','source gate')
    for n,h in gate['research_input_sha256'].items():need(sha(ROOT/n)==h,'source gate '+n)
    for s in read(HERE/'resources.json')['resources']:
        i=s['inspection'];need(i.get('scope')=='; '.join(s['locators']) and i.get('passages')==s['locators'] and i.get('method'),'actual inspection');cap=i.get('capture')
        if cap:need(sha(ROOT/cap['path'])==cap['sha256'],'captured primary')
    for n,h in read(HERE/'source_access.json')['source_sha256'].items():need(sha(ROOT/n)==h,'inspected input '+n)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');a=p.parse_args();d=read(HERE/'completion_dispositions.json');r=read(HERE/'validation.json');pins=runner.pins();ext=runner.external();binding=runner.retained();errors=validate(d,r,pins,ext,binding);controls=[]
    if a.self_test and not errors:
        names=['missing-criterion','missing-delayed','changed-task','changed-delay','wrong-specific-evidence','inflated-review','inflated-learner','inflated-physical','stale-source','stale-output','stale-retained','lost-replay','lost-policy','lost-drop','lost-stub','wrong-skip','wrong-capacity-policy','lost-remaining']
        for k in names:
            x=copy.deepcopy(d);y=copy.deepcopy(r)
            if k=='missing-criterion':x['updates'].pop(0)
            elif k=='missing-delayed':x['updates'].pop()
            elif k=='changed-task':x['updates'][0]['original_task']='placeholder'
            elif k=='changed-delay':x['updates'][1]['minimum_days']=0
            elif k=='wrong-specific-evidence':x['updates'][0]['author_validation']['specific_evidence']=[]
            elif k.startswith('inflated-'):x['updates'][0]['dimensions'][{'inflated-review':'external_review','inflated-learner':'independent_learner','inflated-physical':'physical_observation'}[k]]['state']='passed'
            elif k.startswith('stale-'):key={'stale-source':'source_sha256','stale-output':'outputs_sha256','stale-retained':'retained_reports'}[k];y[key][next(iter(y[key]))]='0'*64
            elif k=='wrong-skip':next(q for q in y['commands'] if q['id']=='gcc.O0.skip.build')['command'].remove('-DCP_MUTANT=3')
            elif k=='wrong-capacity-policy':next(q for q in y['commands'] if q['id']=='gcc.O0.policy.build')['command'].remove('-DCPX_MAX_PAYLOAD=64')
            elif k=='lost-remaining':x['remaining_parents']=[]
            else:y['commands']=[q for q in y['commands'] if q['id']!='gcc.O0.'+k.removeprefix('lost-')]
            rejected=bool(validate(x,y,pins,ext,binding));controls.append(dict(control=k,rejected=rejected))
            if not rejected:errors.append(k+' accepted')
    if runner.pins()!=pins or runner.external()!=ext or runner.retained()!=binding:errors.append('source changed during guard')
    print(json.dumps(dict(passed=not errors,updates=len(d['updates']),commands=len(r['commands']),paired_replay_cells=72,negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
