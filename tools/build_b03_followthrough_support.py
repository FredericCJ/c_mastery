"""Exact additive original X support; never merges or regenerates global catalogs."""
import copy,hashlib,json,re,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from cmastery.exercises import parse_brief
BASE='research/v5/expansion/b03_followthrough/';REPORT='reviews/v5/expansion/b03_followthrough/campaign_01.json'
TOPICS=(23,24,25,28,30);DIMS=('teaching','implementation','author_validation','external_review','independent_learner','physical_observation')
PROFILES=('gcc-debug','gcc-release','clang-debug','clang-release','gcc-asan','clang-asan')
SCOPES={
23:{'R1':'Every script prefix, exact confirmed byte output, zero progress, oversized transfer and terminal idempotence.', 'R2':'Synthetic interruption/would-block quotas plus actual two-writer round-robin under returning-callback premise.', 'R3':'Confirmed-prefix limits and visible partial application with request identity, exact payload matching and retained progress.', 'DELAYED':'Same-ID partial-message resumption and lost-final-reply receipt; conflicting/stale/exhausted IDs and atomic effect/progress premise.'},
24:{'R1':'Current/stale/future generation words, three nonreused IDs and owned-start rejection.', 'R2':'Actual requested/queued/final states; rejection retains ownership and queued ACK cannot reclaim.', 'R3':'Repeated timeout/rejection plus completion/final-quiescence orderings, exactly one report.', 'DELAYED':'Queued-acceptance ACK is separated from final quiescence; every bounded event-word prefix checked independently.'},
25:{'R1':'Actual original SPSC publication and reverse-reuse reads-from/happens-before arguments and bounded RC11 runs.', 'R2':'Actual relaxed-order and two-producer RC11 race counterexamples, with ordinary payload conflicts analyzed without native UB.', 'R3':'Separate atomic support size/alignment/is_lock_free/helper observations, native stress, RC11 bounds and algorithm progress limits.', 'R4':'Actual equal-work acquire/release and seq_cst handoffs; exact two-token provider translation, selected compiler assembly and bounded RC11.', 'R5':'Payload ownership/value graphs, native monotonic SPSC and two-producer uniqueness/order, explicit claim states.', 'DELAYED':'Original two-producer ownership counterexample and actual CAS claim redesign with publication/reuse, stalled-owner and failure semantics.'},
28:{'R1':'Logical four-event fields and reserved high-bit W0C command policy separated from physical bus/struct layout.', 'R2':'W1C access counts, consuming data reads, clear-on-read debugger witness and W0C one-write/no-read encoding.', 'R3':'All pre/post arrival event sets, selected/unselected survival and unavoidable same-bit coalescence.', 'DELAYED':'Actual W0C command255 XOR mask, reserved bits1, independent event-set oracle and wrong-encoding mutant.'},
30:{'R1':'Modular monotonic units/sampling, rational upward rounding and retained127ms horizon at16-bit/0.5ms resolution.', 'R2':'All16777216 original inputs: admitted due, detectable ambiguity and invalid delays with output preservation.', 'R3':'Unwrapped-duration oracle and indistinguishable10/266-tick histories; coherent sampling cannot detect missed full wraps.', 'DELAYED':'Actual bounded high-low-high adapter, all65536 initial values and64 inter-read schedules, instability/output preservation and explicit hardware-atomic-update premise.'}}
LIMITS=['Author reference and finite validation only; no independent review or learner delayed result.','No physical device/timer, actual OS cancellation, durable service or exact inaccessible published C17 claim.','Native stress, RC11 selected executions, synthetic state models and source arguments retain their distinct scopes.']
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def write(p,v):
 path=ROOT/p;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def variants(body):return re.search(r'^## Variants\s*\n(.*?)(?=^## |\Z)',body,re.M|re.S)[1].strip()
def selectors(topic,key):
 def profiles(*suffixes):return [p+' '+s for p in PROFILES for s in suffixes]
 if topic==23:
  return profiles(*({'R1':['base io'],'R2':['base io','variant scheduler'],'R3':['base io','oracle message'],'DELAYED':['oracle message']}[key]))
 if topic==24:return profiles(*(['base cancel'] if key=='R1' else ['base cancel','oracle cancel']))
 if topic==25:
  data={'R1':['GenMC spsc','GenMC claim'],'R2':['GenMC spsc-relaxed','GenMC claim-no-publication','GenMC claim-no-reuse','GenMC original-two-producers'],'R3':['GenMC spsc','GenMC claim'],'R4':['GenMC claim-sc','SC exact source translation'],'R5':['GenMC spsc','GenMC claim'],'DELAYED':['GenMC original-two-producers','GenMC claim','GenMC claim-no-publication','GenMC claim-no-reuse']}
  return data[key]+profiles('threads 0 execute','threads 1 execute','threads 0 undefined','threads 1 undefined','threads 0 disassembly','threads 1 disassembly')
 if topic==28:return profiles('base mmio','oracle device')
 return profiles(*(['variant clock'] if key=='DELAYED' else ['base deadline','variant clock']))
def dimensions(origin,topic,validation):
 impl=['assessor/b03_depth/depth.c','assessor/b03_followthrough/ANALYSIS.md','labs/b03_followthrough/follow.h','labs/b03_followthrough/tests.c']
 impl+=['assessor/b03_followthrough/atomics.c','assessor/b03_followthrough/model.c','assessor/b03_followthrough/threads.c','assessor/b03_followthrough/sc_translation.py'] if topic==25 else ['assessor/b03_followthrough/transfers.c','assessor/b03_followthrough/base_cases.c','assessor/b03_followthrough/variants.c','assessor/b03_followthrough/oracle.py']
 return dict(teaching=dict(state='complete-exact-original-and-changed-condition',evidence=[origin,'labs/b03_depth/CONTRACTS.md','labs/b03_followthrough/CONTRACTS.md','labs/b03_followthrough/README.md']),implementation=dict(state='implemented-exact-bounded-mechanisms',evidence=impl),author_validation=dict(state='passed-source-bound-exact-author-support',evidence=validation),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='not-claimed-separate-target-obligation',evidence=['labs/b03_followthrough/CONTRACTS.md']))
def aggregate(rows):return {k:dict(state=rows[0]['dimensions'][k]['state'],evidence=sorted({p for r in rows for p in r['dimensions'][k]['evidence']})) for k in DIMS}
def build(report=REPORT):
 campaign=read(report);assert campaign['passed'] and not campaign['pilot'];rows=[];bindings=[];pins=set(campaign['input_sha256']);pins.add(report)
 for topic in TOPICS:
  origin=f'curriculum/v5/exercises/X-T{topic}-002.md';meta,body=parse_brief(ROOT/origin);children=[]
  for c in meta['rubric']:
   key=c['id'].rsplit('-',1)[1];row=dict(id=c['id'],original_source=origin,original_criterion=copy.deepcopy(c),closure_rationale=SCOPES[topic][key],required_checks=selectors(topic,key),dimensions=dimensions(origin,topic,[report]),evidence_limits=LIMITS);children.append(row);rows.append(row)
  delayed=meta['delayed_transfer'];assert delayed['variant_ids']==[meta['id']+'-DELAYED']
  row=dict(id=delayed['variant_ids'][0],original_source=origin,original_variants=variants(body),minimum_days=delayed['minimum_days'],closure_rationale=SCOPES[topic]['DELAYED'],required_checks=selectors(topic,'DELAYED'),dimensions=dimensions(origin,topic,[report]),evidence_limits=LIMITS);rows.append(row);children.append(row)
  rows.append(dict(id=meta['id'],original_source=origin,required_criteria=[c['id'] for c in meta['rubric']],required_variants=delayed['variant_ids'],dimensions=aggregate(children),closure_rationale='Exact constituent criteria and both ordinary/delayed changed conditions; no domain-wide inference.',evidence_limits=LIMITS))
  bindings.append(dict(exercise=meta['id'],source=origin,criteria=meta['rubric'],variants=variants(body),minimum_days=delayed['minimum_days'],prerequisites=meta['prerequisite_exercises'],original_body_sha256=hashlib.sha256(body.encode()).hexdigest()))
 write(BASE+'evidence_map.json',dict(schema_version=1,roots=bindings,current_reports=[report]))
 for row in rows:
  for d in row['dimensions'].values():pins.update(d['evidence'])
 pins.update([BASE+'evidence_map.json',BASE+'author_support.py','tools/build_b03_followthrough_support.py','tools/check_b03_followthrough.py','cmastery/exercises.py'])
 result=dict(schema_version=1,required=True,updates=sorted(rows,key=lambda r:r['id']),source_sha256={p:sha(p) for p in sorted(pins)},current_reports=[report],remaining_author_work=[],remaining_external_work=LIMITS,scope='Five original B03 roots,17 exact rubric objects,five original delayed clauses; concrete new companions and exact source-bound campaign.')
 write(BASE+'support.json',result);print(json.dumps({'updates':len(rows),'source_pins':len(pins),'campaign':report}))
if __name__=='__main__':
 import argparse
 p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();build(a.report)
