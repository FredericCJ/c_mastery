"""Maintain additive original-ID dispositions for T71/T72, never historical acceptance."""
from pathlib import Path
import argparse,copy,hashlib,json,re
ROOT=Path(__file__).resolve().parents[1]
REPORT='research/v5/modern/'
def read(path):return json.loads((ROOT/path).read_text(encoding='utf8'))
def metadata(path):return json.loads(re.search(r'```json\s*(.*?)\s*```',(ROOT/path).read_text(encoding='utf8'),re.S)[1])
def h(path):return hashlib.sha256((ROOT/path).read_bytes()).hexdigest()
def report(name,pattern):
 path=REPORT+name+'_validation.json';r=read(path);checks=[c['label'] for c in r['checks'] if re.search(pattern,c['label'])]
 if not checks:raise ValueError('No actual matching checks: '+path+' '+pattern)
 return dict(path=path,checks=checks,scope='Actual named author commands only; not independent learner or external review')
def dimensions(teaching,implementation,validation,physical=False):
 return dict(teaching=dict(state='complete',evidence=teaching),implementation=dict(state='complete',evidence=implementation),author_validation=dict(state='passed',evidence=[x['path'] for x in validation]),external_review=dict(state='not-run',evidence=[]),independent_learner=dict(state='not-assessed',evidence=[]),physical_observation=dict(state='pending' if physical else 'not-applicable-host-or-model-contract',evidence=teaching[-1:]))
def build(domain,family,specs):
 oldpath=REPORT+family+'_gap_ledger.json';old=read(oldpath);gapmap={g.get('exercise'):g for g in old['gaps'] if 'exercise' in g};updates=[];bindings=[];reports=set();sourcepins={oldpath:h(oldpath)}
 for index,(stem,case,anchor,extras) in enumerate(specs,1):
  x=f'X-{domain}-{index:03}';brief=f'curriculum/v5/exercises/{x}.md';m=metadata(brief);contract=f'labs/{family}/followthrough/CONTRACTS.md';analysis=f'assessor/{family}/followthrough/ANALYSIS.md'
  code=[f'labs/{family}/followthrough/{stem}.h',f'labs/{family}/followthrough/{stem}.c',f'assessor/{family}/followthrough/{stem}.c']
  validation=[report(family+'_followthrough',r' '+case+r' execute |'+case+r'.*TSan|'+('CortexM4' if domain=='T71' else 'fiber ELF|fiber context') if case=='fiber' else r' '+case+r' execute |'+case+r'.*TSan'),report(family+'_baseline_depth',r' '+anchor+r' |boundaries execute|independent finite')]
  for name,pattern,paths in extras:validation.append(report(name,pattern));code.extend(paths)
  for row in validation:reports.add(row['path'])
  sourcepins[brief]=h(brief)
  ids=[]
  for rubric in m['rubric']:
   binding=dict(objective=rubric['objective'],exercise=x,rubric_id=rubric['id'],criterion=rubric['criterion'],teaching=[brief,contract],implementation=code,assessor_analysis=[m['assessor_analysis'],analysis],author_validation=validation,independent_assessment=dict(state='not-assessed',minimum_days=m['delayed_assessment']['minimum_days'],variant_ids=m['delayed_assessment']['variant_ids']),evidence_limits=['Each named criterion still requires its own learner reasoning and evidence; this is author material/execution only','R1 written arguments are not certified by test outcomes; non-author review remains unperformed'])
   bindings.append(binding);ids.append(rubric['id'])
  gap=gapmap[x];base=dict(dimensions=dimensions([brief,contract],code,validation,domain=='T71' and index in [5,8]),closure_rationale='The actual original retained mechanism, required alternative and changed-condition implementation are supplied with exact source-bound checks; independent learner and external review dimensions remain open.',artifacts=[brief,contract,*code,analysis],original_gap=gap['id'],original_work=gap['work'],parent_project=m['project'],criterion_ids=ids,source_records=m['source_records'],remaining_author_work=[],evidence_limits=['Finite author executions do not establish universal correctness, external review or independent mastery','Reference-artifact composition is not an actual learner import campaign','Models/build/link evidence do not establish physical device behavior'])
  for ident,kind in [(gap['id'],'original-gap'),(x,'original-exercise'),(x+'-DELAYED','original-delayed-variant'),*[(r['id'],'original-rubric') for r in m['rubric']]]:
   row=copy.deepcopy(base);row['id']=ident;row['kind']=kind
   if kind=='original-rubric':row['criterion_ids']=[ident]
   if kind=='original-delayed-variant':row['delayed_assessment']=m['delayed_assessment']
   updates.append(row)
 for suffix in ['REF','INTEGRATION','HARDWARE']:
  ident=domain+'-'+suffix;gap=next(g for g in old['gaps'] if g['id']==ident);teach=[f'labs/{family}/followthrough/ADAPTERS.md'];validation=[];impl=teach[:]
  if suffix=='INTEGRATION':
   validation=[report('protocols_followthrough',r'system execute|CortexM4')] if domain=='T71' else [report('coroutines_imports',r'integration execute|stream execute')]
   impl += [f'assessor/{family}/followthrough/'+('system_follow.c' if domain=='T71' else 'integration_follow.c')]
  dims=dimensions(teach,impl,validation,suffix=='HARDWARE' or domain=='T71')
  if suffix=='REF':dims['implementation']=dict(state='not-applicable-reference-access',evidence=[]);dims['author_validation']=dict(state='unavailable-exact-published-C17',evidence=[REPORT+family+'_followthrough_sources.md']);dims['teaching']['state']='gap-documented';dims['physical_observation']=dict(state='not-applicable',evidence=[])
  if suffix=='HARDWARE':dims['implementation']=dict(state='not-applicable-to-physical-observation',evidence=teach);dims['author_validation']=dict(state='not-physical-validation',evidence=teach)
  if suffix=='INTEGRATION':dims['independent_learner']=dict(state='pending-actual-learner-imports',evidence=teach)
  reports.update(x['path'] for x in validation)
  updates.append(dict(id=ident,kind='original-gap',original_work=gap['work'],dimensions=dims,artifacts=impl,closure_rationale='Keep exact source access, real learner imports and physical observations separate from author code and host execution.',remaining_author_work=[],external_requirements=[gap['work']] if suffix!='INTEGRATION' else ['Execute these adapters against the learners actual accepted source revisions; author references do not satisfy that personal evidence.'],evidence_limits=['Original gap retained; unavailable evidence is not relabelled inspected or passed']))
 for path in [f'labs/{family}/followthrough/CONTRACTS.md',f'labs/{family}/followthrough/ADAPTERS.md',f'assessor/{family}/followthrough/ANALYSIS.md',REPORT+family+'_followthrough_research_gate.json']:
  sourcepins[path]=h(path)
 return dict(schema_version=1,domain=domain,scope=f'Original{len(old["gaps"])} gaps, {len(specs)} exercise roots, {len(bindings)} exact rubric bindings and {len(specs)} original delayed IDs; additive author follow-through only',updates=updates,criterion_bindings=bindings,current_reports=sorted(reports),input_sha256=sourcepins,summary=dict(original_gaps=len(old['gaps']),exercise_roots=len(specs),rubric_bindings=len(bindings),delayed_variants=len(specs),source_access_gap=True,independent_learner_assessed=False,external_review_performed=False,physical_observation_performed=False),limits=['Reported author checks must still pass with current source hashes','All ready bounded mechanisms supplied; externally supplied exact C17, learner histories and applicable physical measurements remain distinct','Higher-order T72.10/T69.12 is owned by a separate retained follow-through record'])
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--write',action='store_true');args=ap.parse_args()
 pa=['labs/protocols/followthrough/alternatives.h','assessor/protocols/followthrough/alternatives.c'];ca=['labs/coroutines/followthrough/alternatives.h','assessor/coroutines/followthrough/alternatives.c']
 specs71=[('protocol_follow','driver','typestate',[]),('protocol_follow','builder','builder',[('t71_t72_alternatives',r'protocols alternatives|staged wrapper',pa)]),('protocol_follow','session','session',[('t71_t72_alternatives',r'protocols alternatives',pa)]),('event_follow','hsm','hsm',[]),('event_follow','active','active',[('protocols_active_imports',r'execute',['labs/protocols/followthrough/active_import.h','assessor/protocols/followthrough/active_import.c'])]),('protocol_follow','decoder','thread',[('t71_t72_alternatives',r'protocols alternatives',pa)]),('event_follow','reactive','behavior',[]),('system_follow','system','driver',[('protocols_cache_transfer',r'.*',['labs/protocols/followthrough/cache_follow.h','assessor/protocols/followthrough/cache_follow.c'])])]
 specs72=[('owned_stream','mapper','iterator',[('t71_t72_alternatives',r'coroutines iterator|measurements',ca)]),('query_follow','query','dataflow',[('t71_t72_alternatives',r'coroutines push',ca)]),('owned_stream','generator','generator',[('t71_t72_alternatives',r'coroutines generator',ca)]),('fiber_follow','fiber','fiber',[]),('expression_follow','expression','cps',[]),('parser_follow','cut','parser',[('coroutines_imports',r'integration execute',['labs/coroutines/followthrough/integration_follow.h','assessor/coroutines/followthrough/integration_follow.c'])]),('parser_follow','memo','packrat',[]),('async_follow','async','async',[('coroutines_followthrough',r'channel execute|channel TSan',['labs/coroutines/followthrough/channel_follow.h','assessor/coroutines/followthrough/channel_follow.c']),('coroutines_imports',r'integration execute',['assessor/coroutines/followthrough/integration_follow.c'])]),('effect_follow','effect','effect',[('coroutines_imports',r'integration execute|stream execute',['assessor/coroutines/followthrough/integration_follow.c','labs/api_depth/stream/stream.h','assessor/api_depth/stream.c'])])]
 for domain,family,specs in [('T71','protocols',specs71),('T72','coroutines',specs72)]:
  data=build(domain,family,specs);path=ROOT/(REPORT+family+'_completion_dispositions.json');text=json.dumps(data,indent=2)+'\n'
  if args.write:path.write_text(text,encoding='utf8')
  elif not path.exists() or path.read_text(encoding='utf8')!=text:raise SystemExit('Stale disposition: '+str(path))
  print(json.dumps(dict(domain=domain,updates=len(data['updates']),criteria=len(data['criterion_bindings']))))
 stream=dict(schema_version=1,id='T72-IMPORT-STREAM-PRODUCER',original_project='PR-T19-04',original_criterion_ids=['AX-PR-T19-04-T19.08.'+x for x in 'abc'],artifact_identity='io.stream',provenance='Newly authored concrete bounded frame/checksum/short-write producer; historical accepted metadata does not establish these files previously existed',public_artifacts=['labs/api_depth/stream/'+x for x in ['stream.h','stream.c','tests.c','CONTRACTS.md']],assessor_artifacts=['assessor/api_depth/stream.c','assessor/api_depth/stream.md'],consumer=dict(project='PR-T72-09',exercise='X-T72-009',source='assessor/coroutines/followthrough/integration_follow.c',learner_source='labs/coroutines/followthrough/integration_follow.c',contract='labs/coroutines/followthrough/ADAPTERS.md'),author_validation=report('coroutines_imports',r'stream execute|integration execute'),limits=['Supports these original criteria; does not by itself close every original legacy analysis/variant','New producer is not retrospectively accepted, independently reviewed or a learner artifact','No independent source standard inspection is claimed for original C framing choices; actual original PR-T19-04 contract was read'])
 path=ROOT/(REPORT+'stream_producer.json');text=json.dumps(stream,indent=2)+'\n'
 if args.write:path.write_text(text,encoding='utf8')
 elif not path.exists() or path.read_text(encoding='utf8')!=text:raise SystemExit('Stale stream producer record')
if __name__=='__main__':main()
