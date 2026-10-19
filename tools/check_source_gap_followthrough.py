"""Read-only guard for exact source scopes and the selected optimized RC11 run."""
import argparse,copy,json,re,subprocess,sys
from pathlib import Path
from build_source_gap_followthrough import ROOT,BASE,PRIVATE,REVIEW,REPORTS,PROVIDERS,REMAINDER,read,sha,write,selected,successors

DIMS={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'}
ORIGINS={'G-LDEPTH-API-SOURCES':'research/v5/expansion/legacy_api/gap_ledger.json','G-LDEPTH-DIAGNOSTICS-SOURCES':'research/v5/expansion/legacy_diagnostics/gap_ledger.json','XI-GENMC-OPT-IR':'research/v5/expansion/legacy_advanced_handoff/source_access.json'}

def require(ok,message):
 if not ok:raise ValueError(message)

def pins(values):
 for p,h in values.items():require((ROOT/p).is_file() and sha(p)==h,'stale local evidence '+p)

def raw(row):
 require(sha(row['raw'])==row['raw_sha256'],'changed command log')
 return (ROOT/row['raw']).read_text(encoding='utf8',errors='replace')

def path_arg(value):
 value=value.split('=',1)[-1]
 prefix='/mnt/f/Learning/Curricula/c_mastery/'
 require(value.startswith(prefix),'unexpected generated artifact location')
 return value[len(prefix):]

def evidence(s,a,v,c,t,old,capture,expected_rows):
 require(set(r['id'] for r in s['updates'])==set(ORIGINS) and len(s['updates'])==3,'exact origins missing')
 for row in s['updates']:
  original=next(x for x in read(ORIGINS[row['id']])['gaps'] if x['id']==row['id'])
  require(row['original_source']==ORIGINS[row['id']] and row['original_record']==original,'original record changed')
  require(set(row['dimensions'])==DIMS,'dimension set changed')
  for name,d in row['dimensions'].items():
   for p in d['evidence']:require((ROOT/p).is_file(),'non-file evidence')
   if name in ('teaching','implementation','author_validation'):require(d['evidence'],'empty author evidence')
  require(row['dimensions']['external_review']['state']=='not-run','invented external review')
  require(row['dimensions']['independent_learner']['state']=='not-assessed','invented learner work')
  require(row['dimensions']['physical_observation']['state']=='not-claimed','invented physical observation')
  if row['id'].startswith('XI-'):
   require(row['dimensions']['implementation']['state']=='partial-selected-optimized-profile-supported-default-O1-still-unsupported','unsupported optimized scope erased')
   require(row['remaining_scope']==['Default O1 memory-intrinsic promotion and whole service/frame translation unit remain unsupported here.'],'default route gap erased')
   require(len(row.get('external_blockers',[]))==1 and row['external_blockers'][0]['affected_ids']==['XI-GENMC-OPT-IR'] and row['external_blockers'][0]['attempts']==REPORTS[:2],'exact tool blocker evidence missing')
   require('LLVM 21.1.8' in row.get('required_environment','') and 'race detection enabled' in row['required_environment'],'tool environment lost')
  else:
   require(row['dimensions']['implementation']['state']=='not-applicable-source-access-disposition-ISO-gap-retained' and row['remaining_scope']==REMAINDER,'source gap erased')
 require(s['remaining_external_work']==REMAINDER and v['unresolved']==REMAINDER,'normative source limits erased')
 require(a['assignments']==expected_rows and a['topics']==13 and a['resources']==66,'assignment/locator/inspection mismatch')
 require(v['assigned_readings']==len(expected_rows) and v['assigned_resources']==66,'assignment count mismatch')
 require(v['passed'] and v['no_whole_book_claim'] is True,'whole-book claim')
 require(v['successor_inspections']==successors(),'successor source passage missing or changed')
 require(v['successor_providers']==[dict(path=p,sha256=sha(p),child_ids=ids) for p,ids in PROVIDERS.items()],'successor child identity mismatch')
 for p,ids in PROVIDERS.items():
  provider=read(p)
  for ident in ids:require(any(r['id']==ident for r in provider['updates']),'missing named successor child')
  for row in v['successor_inspections']:
   if row['registry'] in provider['source_sha256']:require(provider['source_sha256'][row['registry']]==row['registry_sha256'],'source successor registry stale')
 for row in v['successor_inspections']:
  cap=row['entry']['inspection'].get('capture')
  if cap:require(sha(cap['path'])==cap['sha256'],'successor primary capture stale')
 require({r['resource'] for r in v['fresh_passages']}=={'R-modern-c-3','R-hanson-cii','R-kernighan-ritchie-2','R-parnas-modules-1972','R-dijkstra-ewd249','R-doxygen-contract-docs','R-KER-MULTIBOOT2'},'fresh scope expanded or dropped')
 fresh={r['resource']:r for r in v['fresh_passages']}
 require(fresh['R-hanson-cii']['locators']==['3.1 Interface','3.2 Implementation','6.1 Interface'] and fresh['R-hanson-cii']['pages']=='PDF48-56 and105-107','Hanson exact scope changed')
 require(fresh['R-KER-MULTIBOOT2']['locators']==['3.3 I386 machine state','3.6.1 Boot information format','3.6.2 Basic tags structure','3.6.6 Modules','3.6.8 Memory map'],'boot scope expanded')
 require(v['held_identities']==[{k:x for k,x in r.items() if k!='path'} for r in capture['held']] and len(capture['held'])==14,'held identities mismatch')
 require(v['renewed_public_sources']==[{k:x for k,x in r.items() if k!='path'} for r in capture['web']],'public capture summary mismatch')
 web={r['name']:r for r in capture['web']}
 require(sum(r.get('status')==200 for r in web.values())==5 and len(web)==6,'capture disposition changed')
 require(any('multiboot.html' in r['url'] and r.get('status')==200 for r in web.values()),'GNU source not actually captured')
 require(any('404' in r.get('error','') for r in web.values()),'failed alternate URL erased')
 for public in [s,a,v]:
  body=json.dumps(public)
  require('F:\\\\Knowledge' not in body and 'F:/Knowledge' not in body,'private held path in public metadata')
 require(c['passed'] and len(c['checks'])==28 and all(r['passed'] for r in c['checks']),'current campaign incomplete')
 checks={r['label']:r for r in c['checks']};require(len(checks)==28,'duplicate campaign check')
 observations=[]
 for opt in [0,1]:
  for mutant in [0,1,7,8]:
   name=f'O{opt}-mutant{mutant}';r=checks[name];cmd=r['command'];body=raw(r)
   require(r['returncode']==r['expected']==(42 if mutant else 0),'wrong semantic result')
   marker='Error: Non-atomic race!' if mutant else 'Number of complete executions explored: 20'
   require(r['marker']==marker and marker in body,'missing semantic result')
   before=next(x for x in cmd if x.startswith('--output-llvm-before='));after=next(x for x in cmd if x.startswith('--output-llvm-after='))
   expected=[cmd[0],'-rc11','-mode=verify','-disable-mm-detector','-disable-estimation',before,after,'--','-std=c17',f'-O{opt}','-I/mnt/f/Learning/Curricula/c_mastery/labs/interrupt_depth/frame','-I/mnt/f/Learning/Curricula/c_mastery/labs/interrupt_depth/handoff',f'-DXI_MUTANT={mutant}']+(['-fno-builtin'] if opt else [])+[cmd[-1]]
   require(cmd==expected,'verification semantics/optimization flags changed')
   model=(ROOT/path_arg(cmd[-1])).read_text();require(model=='#include "/mnt/f/Learning/Curricula/c_mastery/assessor/interrupt_depth/handoff/queue.c"\n#include "/mnt/f/Learning/Curricula/c_mastery/assessor/interrupt_depth/handoff/genmc.c"\n','source replacement or altered harness')
   for stage,arg in [('before',before),('after',after)]:
    ir=path_arg(arg);rverify=checks[name+'-verify-'+stage]
    require(rverify['returncode']==rverify['expected']==0 and rverify['command'][1:3]==['-passes=verify','-disable-output'] and path_arg(rverify['command'][-1])==ir,'missing LLVM verification')
    require(raw(rverify)=='','unexpected LLVM verifier diagnostic')
   if mutant==0:
    ir=(ROOT/path_arg(before)).read_text();loads=re.findall(r'load atomic i32,.*? (monotonic|acquire|seq_cst),',ir);stores=re.findall(r'store atomic i32 .*? (release|seq_cst),',ir)
    require(not re.search(r'\bcall\b[^\n]*@llvm\.mem',ir),'unhandled memory intrinsic in selected input')
    require(loads.count('monotonic')==loads.count('acquire')==(6 if opt else 2) and 'seq_cst' not in loads,'atomic loads strengthened or missing')
    require(stores==['release']*(6 if opt else 2),'atomic stores strengthened or missing')
    observations.append(dict(profile=f'O{opt}'+(' -fno-builtin' if opt else ''),complete_executions=20,monotonic_loads=loads.count('monotonic'),acquire_loads=loads.count('acquire'),release_stores=len(stores)))
 require(checks['genmc-version']['marker']=='GenMC v0.19.0 (commit #9f6c4c0)' and checks['genmc-version']['marker'] in raw(checks['genmc-version']),'wrong current tool identity')
 for name in ['clang-help','genmc-help','genmc-version']:
  r=checks[name];require(r['returncode']==r['expected']==0 and r['marker'] in raw(r),'tool help/version failed')
 require(t['source_stable'] and len(t['attempts'])==6,'failed attempt history missing')
 attempts={r['label']:r for r in t['attempts']};failed=attempts['o1']
 require(failed['returncode']==-4 and '-O1' in failed['command'] and '-fno-builtin' not in failed['command'] and 'Memory intrinsic found! Attempting to promote it...' in raw(failed),'unsupported default O1 erased')
 for name in ['o1-no-builtin','o1-scalar','o1-no-idiom']:
  r=attempts[name];require(r['returncode']==0 and 'Number of complete executions explored: 20' in raw(r),'supported attempt missing')
 require(old['passed'] is False and [r['label'] for r in old['checks'] if not r['passed']]==['genmc-version'],'historical failed envelope rewritten')
 require(sha(BASE+'history/run_genmc_ir_validation_01.py')==old['input_sha256']['tools/run_genmc_ir_validation.py'],'historical producer bytes lost')
 historic=read('research/v5/expansion/legacy_advanced_handoff/optimized_model_attempt.json')
 require(historic['returncode']==-4 and historic['validation_credit'] is False,'original unsupported attempt changed')
 for p in ['assessor/interrupt_depth/handoff/queue.c','assessor/interrupt_depth/handoff/genmc.c','labs/interrupt_depth/handoff/handoff.h','labs/interrupt_depth/frame/frame.h']:
  require(c['input_sha256'][p]==t['input_sha256'][p]==historic['input_sha256'][p]==sha(p),'original queue input changed')
 return observations

def external(values):
 script='import json,hashlib,sys,pathlib; v=json.load(sys.stdin); bad=[p for p,h in v.items() if not pathlib.Path(p).is_file() or hashlib.sha256(pathlib.Path(p).read_bytes()).hexdigest()!=h]; print(json.dumps({"identities":len(v),"mismatches":len(bad)})); sys.exit(bool(bad))'
 command=['wsl','python3','-c',script] if sys.platform=='win32' else [sys.executable,'-c',script]
 r=subprocess.run(command,input=json.dumps(values),text=True,capture_output=True,timeout=60);require(r.returncode==0,'external tool/source identity mismatch')

def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);args=p.parse_args();require(not (ROOT/args.report).exists(),'report must be a new path')
 s=read(BASE+'support.json');a=read(BASE+'assigned_inspection.json');v=read(BASE+'source_validation.json');c=read(REVIEW+'genmc_validation_02.json');t=read(REVIEW+'genmc_attempts_01.json');old=read(REVIEW+'genmc_validation_01.json');latest=read(PRIVATE+'LATEST.json');capture=read(latest['receipt']);expected,unused=selected()
 pins(s['source_sha256']);require(sha(latest['receipt'])==latest['sha256']==v['private_receipt_sha256'] and v['private_receipt']==latest['receipt'],'capture identity mismatch');require(sha(PRIVATE+'capture.py')==capture['producer_sha256'],'capture producer changed')
 for i,(path,h) in enumerate(capture['library_inputs'].items()):require(Path(path).is_file() and sha(path)==h,'library input mismatch '+str(i))
 for i,row in enumerate(capture['held']):require(Path(row['path']).is_file() and sha(row['path'])==row['sha256'],'held payload mismatch '+str(i))
 for row in capture['extracts']:require(sha(row['extract'])==row['extract_sha256'],'private extract mismatch')
 for row in capture['web']:
  if row.get('path'):require(sha(row['path'])==row['sha256'],'private web capture mismatch')
 for report in [c,t]:pins(report['input_sha256']);pins(report['outputs_sha256']);external(report['external_input_sha256'])
 pins(old['outputs_sha256']);observations=evidence(s,a,v,c,t,old,capture,expected);controls=[]
 history=read(BASE+'history/source_packet_01/manifest.json')
 for original,row in history.items():require(sha(row['archive'])==row['sha256'],'first source packet bytes lost')
 require(history[BASE+'support.json']['sha256']=='f0ec5b4c3d349b654b9d87627666b748ded4fd873bf311e98264fe11142b0cdc','wrong first source packet')
 if args.self_test:
  for case in range(19):
   ss,aa,vv,cc,tt,oo=copy.deepcopy((s,a,v,c,t,old))
   if case==0:ss['updates'].pop()
   elif case==1:ss['updates'][0]['original_record']['description']='generic replacement'
   elif case==2:aa['assignments'].pop()
   elif case==3:aa['assignments'][0]['assignment']['locator']='whole publication'
   elif case==4:vv['unresolved']=[]
   elif case==5:vv['no_whole_book_claim']=False
   elif case==6:next(r for r in cc['checks'] if r['label']=='O1-mutant0')['command'][1]='-sc'
   elif case==7:next(r for r in cc['checks'] if r['label']=='O1-mutant0')['command'].insert(2,'-disable-race-detection')
   elif case==8:next(r for r in cc['checks'] if r['label']=='O1-mutant7').update(returncode=1,expected=1)
   elif case==9:oo['passed']=True
   elif case==10:next(r for r in cc['checks'] if r['label']=='O1-mutant0')['command'].remove('-fno-builtin')
   elif case==11:ss['updates'][0]['dimensions']['external_review']['state']='passed'
   elif case==12:ss['updates'][0]['dimensions']['independent_learner']['state']='passed'
   elif case==13:ss['updates'][0]['dimensions']['physical_observation']['state']='passed'
   elif case==14:next(r for r in tt['attempts'] if r['label']=='o1')['returncode']=0
   elif case==15:vv['fresh_passages'][-1]['locators'].append('whole specification')
   elif case==16:vv['successor_inspections'].pop()
   elif case==17:vv['unresolved'][1]='Actual POSIX2024 runtime observed'
   else:next(r for r in ss['updates'] if r['id']=='XI-GENMC-OPT-IR')['external_blockers']=[]
   try:evidence(ss,aa,vv,cc,tt,oo,capture,expected)
   except (ValueError,KeyError,StopIteration):controls.append(dict(case=case,rejected=True))
   else:raise ValueError('negative control accepted '+str(case))
 pins(s['source_sha256']);result=dict(passed=True,status='passed',source_sha256=sha(BASE+'support.json'),source_files=len(s['source_sha256']),updates=3,assigned_readings=len(expected),assigned_resources=66,fresh_resource_scopes=7,integrator_source_scopes=6,held_payload_identities=14,command_checks=28,observed_IR=observations,negative_controls=controls,current_reports=s['current_reports'],remaining=s['remaining_external_work']+s['updates'][-1]['remaining_scope']);write(args.report,result);print(json.dumps(result))

if __name__=='__main__':main()
