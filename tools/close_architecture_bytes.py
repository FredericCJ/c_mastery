"""Package the exact six T37.03/T37.04 criterion/changed-condition pairs."""
import argparse,copy,hashlib,json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT));HERE=ROOT/'research/v5/expansion/legacy_architecture_bytes'
from research.v5.expansion.legacy_architecture_bytes.author_support import attach
PROJECT='PR-T37-02';RECEIPT='research/v5/expansion/legacy_architecture_bytes/validation.json';SUPPORT='research/v5/expansion/legacy_architecture_bytes/completion_dispositions.json'
ARTIFACTS=['labs/architecture_bytes/CONTRACTS.md','assessor/architecture_bytes/ANALYSIS.md','assessor/architecture_bytes/bytes.c','assessor/architecture_bytes/x86.S','assessor/architecture_bytes/thumb.S','assessor/architecture_bytes/oracle.py','labs/architecture_bytes/tests.c','labs/architecture_bytes/native.c','labs/architecture_bytes/placement.c','labs/architecture_bytes/placement.ld']
RATIONALES={'T37.03.a':'Actual native partial/full writes and M4 MOVS, equal-contract original assembly loops,124 retained timing samples/profile with migrations/confounds and independently checked observable values.','T37.03.b':'Checked whole numeric extents before pointer formation,5472 independent arithmetic packets, explicit LE/BE wire adapter and signed extrema; no wire-layout casts.','T37.03.c':'Actual typed struct aw_record pool, representation copy and initialized malloc storage; invalid declared-byte-array cast is compile/analysis-only; actual F439 CCM/SRAM typed objects linked with independently checked intervals and inspected bus exclusion.','T37.04.a':'Original bounded C decoder preserves output on truncation/unsupported forms; complete65536 Thumb first-halfword classification, selected CMP fields and explicitly gated newly inspected CMP AL/B T1 variants.','T37.04.b':'7936 independently modeled signed PC-relative packets across relocated streams, actual original JA bytes, selected Thumb PC+4 convention and intended current-PC mutant rejection.','T37.04.c':'Actual assembled/disassembled directive fixture plus GNU AT&T/Intel mnemonic equivalence, operand-width counterexample and explicit shorter-immediate assembler choice; no arbitrary-byte execution.'}
REMAINING=['PR-T37-03 through PR-T37-08 original and changed mechanisms remain separately required.','External review, independent seven-day learner, physical observation and exact final C17/C23 remain unclaimed.']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def canon():return ROOT/('curriculum/v5/expansion/legacy_assessments/'+PROJECT+'.json')
def evidence(commands,objective):
 patterns={'T37.03.a':['.registers','.timing','.disassembly','thumb.'],'T37.03.b':['.bytes','.storage','.mutation.ENDIAN','.mutation.EXTENT','thumb.'],'T37.03.c':['.storage','.allocated','.bad_cast','placement.','thumb.'],'T37.04.a':['.decode','.mutation.EXTENSION','.encoding.','encoding.oracle','thumb.'],'T37.04.b':['.branches','.mutation.PC','.encoding.','encoding.oracle','thumb.'],'T37.04.c':['.encoding.','encoding.oracle','.decode','.disassembly']}
 return dict(report=RECEIPT,checks=[n for n in commands if any(p in n for p in patterns[objective])])
def dimensions():return {k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',ARTIFACTS[:1]),('implementation','complete',ARTIFACTS),('author_validation','passed',[RECEIPT]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
def integrate():
 d=read(canon());snapshot=HERE/(PROJECT+'.before-bytes.json')
 if not snapshot.exists():snapshot.write_bytes(canon().read_bytes())
 public=sorted(p.relative_to(ROOT).as_posix() for p in (ROOT/'labs/architecture_bytes').rglob('*') if p.is_file() and '__pycache__' not in p.parts)+['labs/algorithm_wrappers/wrappers.h','labs/containers_depth/containers.h','labs/containers_depth/byte_helpers.c']
 write(HERE/('attachment_'+PROJECT+'.json'),dict(project=PROJECT,domain='T37',supplied_artifacts=public,shared_contracts=[ARTIFACTS[0]],source_register='research/v5/expansion/legacy_architecture_bytes/resources.json',support=SUPPORT,criterion_ids=[c['id'] for c in d['criteria']],scope='Six exact ordinary/changed register, byte, storage, instruction and relocation mechanisms.'))
 after=attach(copy.deepcopy(d));assert after['criteria']==d['criteria'] and after['baseline_binding']==d['baseline_binding'] and attach(copy.deepcopy(after))==after;write(canon(),after);print('PASS additive byte/register attachment')
def package():
 reports=[read(HERE/n) for n in ['validation.json','exports.json']];files={pathlib.Path(__file__),ROOT/'tools/check_architecture_bytes.py'}|{p for p in HERE.iterdir() if p.is_file() and p.name!='completion_dispositions.json' and not p.name.startswith('attempt') and not p.name.startswith('root_')}
 for data in reports:
  assert data['status']=='pass' and data['commands'] and all(x['passed'] for x in data['commands'])
  for key in ['source_sha256','outputs_sha256']:
   for n,h in data[key].items():assert sha(ROOT/n)==h,('stale',n)
  files|={ROOT/n for n in data['source_sha256']}
 d=read(canon());before=read(HERE/(PROJECT+'.before-bytes.json'));assert d['criteria']==before['criteria'] and d['baseline_binding']==before['baseline_binding'] and attach(copy.deepcopy(d))==d;files.add(canon());r,e=reports;commands=[c['id'] for c in r['commands']];rows=[]
 for c in d['criteria']:
  v=c['variants'][0]
  for delayed in [False,True]:rows.append(dict(id=v['id'] if delayed else c['id'],parent_project=PROJECT,objective=c['objective'],original_task=v['task'] if delayed else c['task'],original_delayed_variant=v['id'],minimum_days=v['minimum_days'],dimensions=dimensions(),artifacts=ARTIFACTS,author_validation=evidence(commands,c['objective']),closure_rationale=RATIONALES[c['objective']],remaining_author_work=[],evidence_limits=r['limits']))
 write(HERE/'completion_dispositions.json',dict(schema_version=1,scope='Exact six original T37.03/T37.04 criteria and six original V1 identities',source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(files)},current_reports=[RECEIPT,'research/v5/expansion/legacy_architecture_bytes/exports.json'],updates=rows,remaining_parents=REMAINING,summary=dict(criteria=6,delayed=6,commands=len(r['commands']),export_commands=len(e['commands']))));print('PASS12 exact byte/register dispositions')
def main():
 p=argparse.ArgumentParser();p.add_argument('--integrate',action='store_true');a=p.parse_args();integrate() if a.integrate else package()
if __name__=='__main__':main()
