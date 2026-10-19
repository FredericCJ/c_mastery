"""Exact systems exercise guard, including transitive providers and actual targets."""
import argparse,copy,hashlib,json,re,subprocess,sys
from pathlib import Path
from build_systems_x_followthrough_support import ROOT,BASE,REPORTS,DIMS,PROFILES,LIMITS,CLAUSES,PROVIDERS,aggregate,dims,hook,parse_brief,provider_rows,read,section,selectors,sha,variant,write
import check_residual_followthrough as audit
def require(ok,message):
 if not ok:raise ValueError(message)
def pins(values):
 for p,h in values.items():require((ROOT/p).is_file() and sha(p)==h,'stale source/artifact '+p)
def raw(row):return (ROOT/row['raw']).read_text(encoding='utf8',errors='replace')
def receipt(value,count):
 require(value.get('passed') is True and not value.get('pilot',False),'incomplete campaign')
 rows={r['label']:r for r in value['checks']};require(len(rows)==len(value['checks'])==count,'missing campaign check')
 for label,r in rows.items():
  require(r['passed'] is True,'failed check '+label)
  if 'raw' not in r:continue
  body=raw(r);require(sha(r['raw'])==r['raw_sha256'],'stale raw '+label)
  require(r['returncode']==r['expected'],'wrong exit '+label)
  require(not r['marker'] or r['marker'] in body,'missing semantic marker '+label)
  require(not any(x in body for x in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer']),'sanitizer finding '+label)
 return rows
def inspect(s,m,reports,briefs):
 require(s['current_reports']==m['reports']==REPORTS,'changed current reports')
 require(s['providers']==m['providers']==sorted(PROVIDERS.values()),'wrong exact providers')
 require(s['remaining_author_work']==[] and s['remaining_external_work']==LIMITS,'lost author/external distinction')
 require(set(s['source_sha256']).isdisjoint({'curriculum/v5/expansion/implementation_updates.json','curriculum/v5/expansion/work_inventory.json'}),'mutable global pin')
 expected=set(CLAUSES)|{'MBR-ONCE-HISTORICAL','MBR-TARGET-ADAPTERS'}
 rows={r['id']:r for r in s['updates']};require(len(rows)==len(s['updates'])==89,'wrong exact row count')
 require(len(m['roots'])==16 and {x['exercise'] for x in m['roots']}==set(CLAUSES),'wrong original root set')
 main=receipt(reports[0],367);controls=receipt(reports[1],123);export=receipt(reports[2],4)
 intake={r['id']:r for r in read(BASE+'intake.json')['rows']};criteria=variants=0
 provider_cache={}
 for origin in m['roots']:
  identity=origin['exercise'];meta,body=briefs[identity];historic=BASE+'history/'+identity+'.md';old,oldbody=parse_brief(ROOT/historic)
  require(sha(historic)==intake[identity]['sha256'],'lost historical bytes')
  stripped=copy.deepcopy(meta);stripped.pop('systems_x_followthrough_author_support',None)
  require(stripped==old and body==oldbody,'changed original body/metadata')
  require(meta==hook.attach(old) and hook.attach(meta)==meta,'impure or missing additive hook')
  delayed=meta.get('delayed_transfer',meta.get('delayed_assessment'))
  require(origin['criteria']==meta['rubric']==intake[identity]['original_rubric'],'changed original rubric')
  require(origin['delayed']==delayed==intake[identity]['original_delayed'] and delayed['minimum_days']==7,'changed original delay')
  require(origin['body_sha256']==hashlib.sha256(body.encode()).hexdigest(),'changed original prose')
  require(origin['prerequisites']==meta['prerequisite_exercises'],'changed prerequisites')
  require(origin['variant_section']==section(body,'Variants'),'changed variant section')
  children=[]
  for c in meta['rubric']:
   r=rows[c['id']];require(r['original_criterion']==c,'changed criterion binding');children.append(r);expected.add(c['id']);criteria+=1
  vids=[]
  for local in delayed['variant_ids']:
   key=local.rsplit('-',1)[-1];ident=local if local.startswith('X-') else identity+'-'+local;r=rows[ident];vids.append(ident);children.append(r);expected.add(ident);variants+=1
   require(r['original_local_id']==local and r['minimum_days']==7,'wrong delayed identity/interval')
   require(r['original_variant']==variant(body,key) and r['original_variant_section']==section(body,'Variants'),'merged or lost variant clause')
  for r in children:
   clause=CLAUSES[identity][r['id'].rsplit('-',1)[-1]]
   require(r['original_source']==origin['source'],'wrong original source')
   require(r['closure_rationale']==clause['rationale'] and r['required_checks']==selectors(clause),'unrelated mechanism selection')
   for name in r['required_checks']:require(name in main and main[name]['passed'],'unexecuted exact selected mechanism')
   key=tuple(clause['providers'])
   if key not in provider_cache:provider_cache[key]=provider_rows(clause)
   require(r['provider_bindings']==provider_cache[key],'stale or unrelated provider child')
   require(r['dimensions']==dims(origin['source'],provider_cache[key]),'lost constituent evidence or inflated dimension')
  root=rows[identity]
  require(root['required_criteria']==[c['id'] for c in meta['rubric']] and root['required_variants']==vids,'omitted constituent')
  require(root['dimensions']==aggregate(children),'wrong root union')
  require(root['original_immediate']==(section(body,'Variants') if identity in ['X-T51-002','X-T55-002','X-T55-003'] else None),'lost immediate changed condition')
 require(criteria==42 and variants==29 and set(rows)==expected,'missing exact original obligation')
 modern=read('curriculum/v5/expansion/modern_bridge_depth.json')['remaining']
 parents={'MBR-ONCE-HISTORICAL':['X-T51-002-R1','X-T51-002-R2','X-T51-002-R3'],'MBR-TARGET-ADAPTERS':['X-T51-002-R2','X-T55-002-R2','X-T55-003-R2']}
 for ident,ids in parents.items():
  r=rows[ident];require(r['original_record']==next(v for v in modern if v['id']==ident),'lost original modern parent')
  require(r['required_children']==ids and r['dimensions']==aggregate([rows[i] for i in ids]),'wrong bounded modern parent union')
 for r in rows.values():
  require(set(r['dimensions'])==set(DIMS) and r['evidence_limits']==LIMITS,'lost evidence limits/schema')
  require(r['dimensions']['external_review']['state']=='not-run','invented independent review')
  require(r['dimensions']['independent_learner']['state']=='not-assessed','invented delayed learner')
  require(r['dimensions']['physical_observation']['state']=='not-run-distinct-host-model-emulator-gates','invented physical observation')
  for d in r['dimensions'].values():
   for path in d['evidence']:require((ROOT/path).is_file(),'nonfile evidence '+path)
 for profile in PROFILES:
  for mechanism in ['transfers','once','reclamation','response','authentication']:require(main[profile+' '+mechanism+' execute']['returncode']==0,'missing profile mechanism')
  for mechanism in ['transfers','reclamation','response']:require(main[profile+' '+mechanism+' oracle']['returncode']==0,'missing independent oracle')
  for mechanism in ['MAC oracle','bootstrap','threads execute']:require(main[profile+' '+mechanism]['returncode']==0,'missing independent/native check')
 # Compiler errors and arbitrary crashes never count as semantic detection.
 for cc in ['gcc','clang']:
  for part,count in [('transfers',4),('once',2),('reclamation',3),('response',2),('authentication',2)]:
   for n in range(1,count+1):
    a=main[f'{cc} {part} mutant {n} compile'];b=main[f'{cc} {part} mutant {n} execute'];marker={'transfers':'SX_CHECK:','once':'ONCE_X_CHECK:','reclamation':'LIST_X_CHECK:','response':'RESPONSE_X_CHECK:','authentication':'AUTH_X_CHECK:'}[part];require(a['returncode']==a['expected']==0 and b['returncode']==b['expected']==1 and b['marker']==marker and marker in raw(b),'mutant not semantic rejection')
  require(main[cc+' tsan execute']['expected']==main[cc+' tsan execute']['returncode']==0,'race detector not executed')
  for part in ['transfers','once','reclamation','response','authentication']:
   require(main[f'{cc} export {part} stub']['expected']==main[f'{cc} export {part} stub']['returncode']==1,'exported stub passed')
   require(main[f'{cc} export {part} reference']['expected']==main[f'{cc} export {part} reference']['returncode']==0,'exported reference failed')
 rejection=[r for label,r in controls.items() if label.endswith(' reject')]
 require(len(rejection)==38 and all(r['expected']==r['returncode']==1 and ('DEPTH_CHECK:' in raw(r) or 'MB_CHECK:' in raw(r)) for r in rejection),'missing original semantic controls')
 require('"hazard_SC_orders": 10' in raw(controls['independent modern finite expectations']),'lost independent original schedules')
 for label in ['bootstrap compiling learner semantic rejection','upper-median bootstrap control','wrong-quantile bootstrap control']:
  require(main[label]['expected']==main[label]['returncode']==1,'missing bootstrap control')
 codegen=[]
 for profile in ['gcc-release','clang-release']:
  body=raw(main[profile+' retained benchmark calls'])
  for name in ['sx_sum_a','sx_sum_b']:
   require(re.search(r'\bcall\w*\s+[^\n]*<'+name+r'>',body),'consumed work call removed')
   require(re.search(r'^[0-9a-f]+ <'+name+r'>:\n',body,re.M),'kernel body missing');codegen.append(profile+' '+name)
 targets=[]
 for cc in ['arm-none-eabi-gcc','clang']:
  for opt in ['0','2']:
   r=main[cc+' M4 '+opt];require(r['returncode']==r['expected']==0 and 'LIST_TARGET_PASS' in raw(r),'missing actual target')
   path=r['command'][r['command'].index('--build')+1]+'/target.json';targets.append(audit.local(path))
 require(len(reports[2]['target_reports'])==2,'missing isolated target exports');targets+=reports[2]['target_reports']
 return dict(rows=89,roots=16,original_criteria=42,original_variants=29,bounded_modern_parents=2,checks=494,new_semantic_controls=26,original_semantic_controls=38,observed_codegen=codegen,target_reports=targets)
def target_receipts(paths):
 external={};count=0
 for path in paths:
  report=read(path);require(report['passed'],'target receipt failed');rows=report['rows'];require(len([r for r in rows if r['name'].endswith('-compile')])==11,'target helper/object closure changed')
  by={r['name']:r for r in rows};require(all(r['returncode']==0 for r in rows),'failed target command')
  for r in rows:
   p=audit.local(r['log']);require(p and sha(p)==r['sha256'],'stale target log')
  require('PASS CMX Cortex-M systems-x' in (ROOT/audit.local(by['qemu']['log'])).read_text(),'no actual target completion')
  require(not (ROOT/audit.local(by['undefined']['log'])).read_text().strip(),'unresolved target imports')
  for p,h in report['source_dependencies'].items():
   local=audit.local(p)
   if local:pins({local:h})
   else:external.setdefault(p,set()).add(h)
  count+=len(rows)
 return external,count
def named_tool_pins(documents):
 found={}
 def visit(v):
  if isinstance(v,dict):
   for name,h in v.get('external_tool_sha256',{}).items():found.setdefault(name,set()).add(h)
   for child in v.values():visit(child)
  elif isinstance(v,list):
   for child in v:visit(child)
 for path in documents:visit(read(path))
 names=[p for p in found if '/' not in p and '\\' not in p]
 code='import json,shutil,sys,pathlib; names=json.load(sys.stdin); print(json.dumps({n:str(pathlib.Path(shutil.which(n)).resolve()) for n in names}))'
 cmd=['wsl','--cd','/tmp','python3','-c',code] if sys.platform=='win32' else [sys.executable,'-c',code]
 result=subprocess.run(cmd,input=json.dumps(names),text=True,capture_output=True,timeout=30);require(result.returncode==0,'cannot resolve named external tools '+result.stderr)
 resolved=json.loads(result.stdout);out={}
 for p,hs in found.items():out.setdefault(resolved.get(p,p),set()).update(hs)
 return out
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);a=p.parse_args();require(not (ROOT/a.report).exists(),'preserve prior receipt')
 s=read(BASE+'support.json');m=read(BASE+'evidence_map.json');reports=[read(r) for r in REPORTS];briefs={i:parse_brief(ROOT/f'curriculum/v5/exercises/{i}.md') for i in CLAUSES}
 pins(s['source_sha256']);summary=inspect(s,m,reports,briefs);external={};artifacts=0
 for report in reports:
  pins(report['input_sha256']);pins(report['outputs_sha256']);artifacts+=len(report['outputs_sha256'])
  for k,h in report.get('external_input_sha256',{}).items():external.setdefault(k,set()).add(h)
 target_external,target_commands=target_receipts(summary['target_reports'])
 for k,hs in target_external.items():external.setdefault(k,set()).update(hs)
 # Extend only this audit's recognized schemas. Shared residual validation is unchanged.
 audit.PIN_KEYS.update({'artifacts','exported_artifacts','source_dependencies'})
 providers=audit.evidence_audit(s['providers']);require(not providers['missing'] and not providers['stale'],'stale transitive provider '+str(providers['missing']+providers['stale'])[:2500])
 require(not any(v.get('passed') is False or v.get('status') in ['failed','FAIL'] for v in providers['report_statuses'].values()),'failed retained provider report')
 for k,hs in providers['external'].items():external.setdefault(k,set()).update(hs)
 for k,hs in named_tool_pins(providers['document_sha256']).items():external.setdefault(k,set()).update(hs)
 external_result=audit.external_check({p:sorted(h) for p,h in external.items()});negative=[]
 if a.self_test:
  names=['lost-id','changed-rubric','merged-variant','wrong-delay','lost-immediate','unrelated-check','wrong-provider','missing-target','wrong-target-exit','missing-tsan','false-command-pass','stale-source','invented-review','invented-learner','invented-physical','erased-limit','wrong-parent','changed-original','missing-export','changed-report']
  for case in names:
   ss,mm,rr,bb=copy.deepcopy((s,m,reports,briefs));leaf=next(r for r in ss['updates'] if 'original_criterion' in r);var=next(r for r in ss['updates'] if 'original_variant' in r)
   if case=='lost-id':ss['updates'].pop()
   elif case=='changed-rubric':leaf['original_criterion']['criterion']='unrelated topic'
   elif case=='merged-variant':var['original_variant']+=' V2: both clauses'
   elif case=='wrong-delay':var['minimum_days']=0
   elif case=='lost-immediate':next(r for r in ss['updates'] if r['id']=='X-T51-002')['original_immediate']=None
   elif case=='unrelated-check':leaf['required_checks']=['gcc-debug b04_depth memory']
   elif case=='wrong-provider':next(r for r in ss['updates'] if r.get('provider_bindings'))['provider_bindings'][0]['provider_sha256']='0'*64
   elif case=='missing-target':rr[0]['checks']=[r for r in rr[0]['checks'] if r['label']!='clang M4 2']
   elif case=='wrong-target-exit':next(r for r in rr[0]['checks'] if r['label']=='clang M4 2')['returncode']=1
   elif case=='missing-tsan':rr[0]['checks']=[r for r in rr[0]['checks'] if r['label']!='gcc tsan execute']
   elif case=='false-command-pass':rr[0]['checks'][0]['returncode']=99
   elif case=='stale-source':ss['source_sha256'][next(iter(ss['source_sha256']))]='0'*64
   elif case=='invented-review':ss['updates'][0]['dimensions']['external_review']['state']='passed'
   elif case=='invented-learner':ss['updates'][0]['dimensions']['independent_learner']['state']='passed'
   elif case=='invented-physical':ss['updates'][0]['dimensions']['physical_observation']['state']='passed'
   elif case=='erased-limit':ss['remaining_external_work']=[]
   elif case=='wrong-parent':next(r for r in ss['updates'] if r['id']=='MBR-TARGET-ADAPTERS')['required_children']=[]
   elif case=='changed-original':bb['X-T43-001']=(bb['X-T43-001'][0],bb['X-T43-001'][1]+' Changed prose.')
   elif case=='missing-export':rr[2]['target_reports'].pop()
   elif case=='changed-report':ss['current_reports']=[]
   try:
    if case=='stale-source':pins(ss['source_sha256'])
    else:inspect(ss,mm,rr,bb)
   except (ValueError,KeyError,AssertionError):negative.append(dict(case=case,rejected=True))
   else:raise ValueError('guard accepted negative '+case)
 pins(s['source_sha256']);pins(providers['document_sha256'])
 result=dict(passed=True,status='passed',source_sha256=sha(BASE+'support.json'),mapping_sha256=sha(BASE+'evidence_map.json'),source_files=len(s['source_sha256']),output_files=artifacts,summary=summary,target_commands=target_commands,providers=providers,external_validation=external_result,negative_controls=negative,current_reports=REPORTS,limits=LIMITS)
 write(a.report,result);print(json.dumps({k:result[k] for k in ['passed','source_sha256','source_files','output_files','target_commands']}));print(json.dumps({'checks':summary['checks'],'negative_controls':len(negative),'provider_documents':providers['documents'],'provider_files':providers['files'],'external_files':external_result['files']}))
if __name__=='__main__':main()
