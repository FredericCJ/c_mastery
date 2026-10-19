"""Bind all eighteen exact P3 ordinary/delayed identities to specific retained/new evidence."""
import argparse,copy,hashlib,json,pathlib,subprocess,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_checkpoint_protocol';PROJECTS=['PR-P3-01','PR-P3-02'];REPORT='research/v5/expansion/legacy_checkpoint_protocol/validation.json';SUPPORT='research/v5/expansion/legacy_checkpoint_protocol/completion_dispositions.json'
from research.v5.expansion.legacy_checkpoint_protocol.author_support import attach
import run_checkpoint_protocol_followthrough as runner
CONTRACT='labs/checkpoint_followthrough/protocol/CONTRACTS.md';ANALYSIS='assessor/checkpoint_followthrough/protocol_analysis.md'
EVIDENCE={'T04.06.b':(['codec'],['protocol'],[]),'T15.12.b':(['codec'],['protocol','protocol16'],['gcc.O0.policy','clang.O3.policy']),'T15.12.c':(['codec'],['auth'],[]),'T18.01.b':(['codec'],['protocol','protocol16'],[]),'T15.13.b':(['stream'],['protocol','protocol16'],['gcc.O0.replay']),'T15.13.c':(['stream'],['protocol','protocol16'],['gcc.O0.replay','gcc.O0.policy']),'T21.05.b':(['stream'],['durable'],[]),'T32.06.a':(['stream'],['protocol','protocol16'],[]),'T32.06.c':(['stream'],[],['gcc.O0.replay','clang.O3.replay','clang.asan.replay'])}
WHY={'T04.06.b':'Exact offset fixture/byte conversion and independent swapped-host model, with no executed big-endian claim.','T15.12.b':'Original length/version admission and actual optional-field/CRC compatibility, plus explicit configured-v1 policy counterexample.','T15.12.c':'Actual bitwise CRC/zlib comparison and tampering counterexample, then fully bound synthetic HMAC/replay protocol using its permitted primitive.','T18.01.b':'Complete original partitions/sentinel publication checks and explicitly restricted in-place overlap implementation.','T15.13.b':'All original split compositions and actual retained message/backpressure under changed finite refusal schedules.','T15.13.c':'Nested candidate recovery, configurable16/64 storage/work bounds and changed-capacity normal-exit missing-frame counterexample.','T21.05.b':'Actual copied queue commit with clear-on-full control, followed by actual receipt/store held/written/durable/ack composition under74 cuts.','T32.06.a':'Deterministic bounded original and configured/refusal byte corpora compared against independent whole-buffer zlib oracle.','T32.06.c':'Actual original minimizer witness and retained property, with new unchanged-witness replay across chunks and actual occupancy limits1/2.'}
REMAINING=['Independent delayed learner attempts and external review remain required and unclaimed.','Physical target/power observations and final published ISO text remain separate evidence dimensions.']
def card(project):return 'curriculum/v5/expansion/legacy_assessments/'+project+'.json'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def original(project):
    d=json.loads(subprocess.check_output(['git','show','HEAD:'+card(project)],cwd=ROOT,text=True,encoding='utf8'))
    for c in d['criteria']:c['successor_support']=dict(path='research/v5/expansion/legacy_completion/checkpoint_support.json',id=c['id'])
    return d
def evidence(objective):
    core,transfer,new=EVIDENCE[objective];out=[dict(report=runner.RETAINED[0],anchors=['checkpoint_depth.'+n for n in core],profiles=8)]
    if transfer:out.append(dict(report=runner.RETAINED[1],cases=transfer,profiles=6))
    if objective=='T32.06.c':out.append(dict(report=runner.RETAINED[2],property='missing synthetic sequence7',checks=['single_byte_deletion_minimal','comparisons']))
    if new:out.append(dict(report=REPORT,checks=new))
    return out
def artifacts(objective):
    core,transfer,new=EVIDENCE[objective];a=[CONTRACT,'labs/checkpoint_depth/include/checkpoint_depth.h','labs/checkpoint_depth/tests/tests.c','assessor/checkpoint_depth/checkpoint_depth.c',ANALYSIS]
    for name in transfer:
        name='protocol' if name=='protocol16' else name;a += ['labs/checkpoint_depth/transfer/'+name+'.h','labs/checkpoint_depth/transfer/'+name+'_tests.c','assessor/checkpoint_depth/transfer/'+name+'.c']
    if objective=='T32.06.a':a+=['labs/checkpoint_depth/tests/check_stream.py','labs/checkpoint_depth/transfer/check_protocol.py']
    if objective in ['T15.13.b','T15.13.c','T32.06.c']:a+=['labs/checkpoint_followthrough/protocol/admission.h','labs/checkpoint_followthrough/protocol/replay.c','labs/checkpoint_followthrough/protocol/check_replay.py','assessor/checkpoint_followthrough/protocol/admission.c']
    if objective in ['T15.12.b','T15.13.c']:a+=['labs/checkpoint_followthrough/protocol/policy.c']
    if objective=='T15.12.c':a+=['labs/checkpoint_depth/transfer/openssl_mac.c']
    if objective=='T32.06.c':a+=['tools/minimize_checkpoint_stream.py',runner.RETAINED[2]]
    return list(dict.fromkeys(a))
def rows(r):
    out=[]
    for project in PROJECTS:
        for c in read(ROOT/card(project))['criteria']:
            v=c['variants'][0];a=artifacts(c['objective'])
            for delayed in [False,True]:
                dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',[CONTRACT,ANALYSIS]),('implementation','complete',a),('author_validation','passed',[REPORT,'research/v5/expansion/legacy_checkpoint_protocol/exports.json',*[e['report'] for e in evidence(c['objective'])]]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
                out.append(dict(id=v['id'] if delayed else c['id'],parent_project=project,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dims,artifacts=a,author_validation=dict(report=REPORT,specific_evidence=evidence(c['objective']),analysis=ANALYSIS+'#'+c['objective'].lower().replace('.','-')),closure_rationale=WHY[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
    return out
def integrate():
    for project in PROJECTS:
        d=read(ROOT/card(project));old=original(project);assert d['criteria']==old['criteria'] and d['baseline_binding']==old['baseline_binding'];snapshot=HERE/(project+'.before-protocol.json')
        if not snapshot.exists():snapshot.write_bytes((ROOT/card(project)).read_bytes())
        public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/'labs/checkpoint_followthrough/protocol').rglob('*')) if p.is_file() and '__pycache__' not in p.parts]
        public+=['labs/checkpoint_depth/include/checkpoint_depth.h','labs/checkpoint_depth/src/checkpoint_depth.c','labs/checkpoint_depth/tests/check_stream.py','fixtures/legacy_checkpoints/P3.json']
        for name in ['protocol','auth','durable']:public += ['labs/checkpoint_depth/transfer/'+name+'.h','labs/checkpoint_depth/transfer/'+name+'_stub.c','labs/checkpoint_depth/transfer/'+name+'_tests.c']
        public += ['labs/checkpoint_depth/transfer/openssl_mac.c','labs/checkpoint_depth/transfer/check_protocol.py','labs/service_depth/adapters/receipt.h','labs/storage/include/store.h']
        put(HERE/('attachment_'+project+'.json'),dict(project=project,domain='INTEGRATION',key='checkpoint_protocol_followthrough',supplied_artifacts=public,shared_contracts=[CONTRACT,'labs/checkpoint_depth/CONTRACTS.md','labs/checkpoint_depth/transfer/CONTRACTS.md'],source_register='research/v5/expansion/legacy_checkpoint_protocol/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Every original/delayed P3 task with exact retained/new source-bound evidence and versioned configured-admission clarification.'))
        after=attach(copy.deepcopy(d));assert after['criteria']==old['criteria'] and after['baseline_binding']==old['baseline_binding'] and attach(copy.deepcopy(after))==after;put(ROOT/card(project),after)
    put(HERE/'objective_mapping.json',dict(criteria=[dict(criterion=c['id'],objective=c['objective'],task=c['task'],delayed=c['variants'][0],teaching=CONTRACT,assessment=ANALYSIS+'#'+c['objective'].lower().replace('.','-'),specific_evidence=evidence(c['objective'])) for p in PROJECTS for c in read(ROOT/card(p))['criteria']],limits=REMAINING));print('PASS9 exact original/delayed objects preserved; both attachments idempotent')
def package():
    reports=[read(HERE/n) for n in ['validation.json','exports.json']];assert reports[0]['source_sha256']==runner.pins() and reports[0]['retained_reports']==runner.retained();files={pathlib.Path(__file__),ROOT/'tools/check_checkpoint_protocol_followthrough.py',ROOT/'research/v5/expansion/legacy_checkpoints/author_supplements.py',ROOT/ANALYSIS,*[ROOT/card(p) for p in PROJECTS]}
    for r in reports:
        assert r['status']=='pass' and r['commands'] and all(q['passed'] for q in r['commands'])
        for key in ['source_sha256','outputs_sha256']:
            for n,h in r[key].items():assert sha(ROOT/n)==h,('stale',n)
        files|={ROOT/n for n in r['source_sha256']}
    files|={p for p in HERE.iterdir() if p.is_file() and p.name not in ['completion_dispositions.json','support_validation.json'] and not p.name.startswith(('attempt','root_'))}
    put(HERE/'completion_dispositions.json',dict(schema_version=1,scope='All nine ordinary and nine delayed exact P3 tasks',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[REPORT,'research/v5/expansion/legacy_checkpoint_protocol/exports.json',*runner.RETAINED],updates=rows(reports[0]),remaining_parents=REMAINING));print('PASS18 exact P3 dispositions')
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
