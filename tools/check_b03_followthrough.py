"""Source-bound exact B03 guard; no inventory or canonical mutation."""
import argparse,copy,hashlib,importlib.util,json,re,subprocess,sys
from pathlib import Path
from build_b03_followthrough_support import BASE,TOPICS,DIMS,PROFILES,SCOPES,ROOT,aggregate,parse_brief,read,selectors,sha,variants,write
def require(ok,reason):
 if not ok:raise ValueError(reason)
def local_pins(pins):
 for path,digest in pins.items():require((ROOT/path).is_file() and sha(path)==digest,'stale file '+path)
def evidence(support,mapping,campaign,briefs):
 require(campaign.get('passed') and not campaign.get('pilot'),'campaign not complete')
 require(not support['remaining_author_work'],'author gap not closed')
 ids=[r['id'] for r in support['updates']];require(len(ids)==27 and len(set(ids))==27,'wrong original ID set')
 rows={r['id']:r for r in support['updates']};checks={r['label']:r for r in campaign['checks']}
 require(len(checks)==len(campaign['checks'])==207,'missing campaign checks')
 for label,r in checks.items():
  require(r['passed'],'failed check '+label)
  if 'raw' not in r:continue
  require(sha(r['raw'])==r['raw_sha256'],'stale raw check '+label);body=(ROOT/r['raw']).read_text(errors='replace')
  require(r['returncode']==r['expected'],'incorrect exit '+label)
  require(not r['marker'] or r['marker'] in body,'missing semantic marker '+label)
  require(not any(x in body for x in ['runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer']),'sanitizer result '+label)
 for binding in mapping['roots']:
  topic=int(binding['exercise'].split('-')[1][1:]);meta,body=briefs[binding['exercise']]
  require(binding['criteria']==meta['rubric'],'changed original criterion object')
  require(binding['variants']==variants(body),'changed ordinary/delayed variants')
  require(binding['original_body_sha256']==hashlib.sha256(body.encode()).hexdigest(),'changed original exercise body')
  require(binding['prerequisites']==meta['prerequisite_exercises'],'changed prerequisites')
  require(binding['minimum_days']==meta['delayed_transfer']['minimum_days']==7,'changed delayed interval')
  children=[]
  for c in meta['rubric']:
   row=rows[c['id']];key=c['id'].rsplit('-',1)[1];require(row['original_criterion']==c,'changed criterion binding');children.append(row)
   require(row['required_checks']==selectors(topic,key),'criterion check selection changed');require(row['closure_rationale']==SCOPES[topic][key],'criterion mechanism changed')
  delayed=rows[meta['id']+'-DELAYED'];children.append(delayed)
  require(delayed['original_variants']==variants(body) and delayed['minimum_days']==7,'delayed task mismatch')
  require(delayed['required_checks']==selectors(topic,'DELAYED'),'delayed check selection changed')
  root=rows[meta['id']];require(root['required_criteria']==[c['id'] for c in meta['rubric']] and root['required_variants']==meta['delayed_transfer']['variant_ids'],'root drops child')
  require(root['dimensions']==aggregate(children),'root not exact constituent union')
  require(meta.get('b03_followthrough_author_support',{}).get('support')==BASE+'support.json','missing additive card hook')
 for row in rows.values():
  require(set(row['dimensions'])==set(DIMS),'wrong dimension keys')
  for name,d in row['dimensions'].items():
   for path in d['evidence']:require((ROOT/path).is_file(),'evidence not concrete file')
   if name in ('teaching','implementation','author_validation'):require(bool(d['evidence']),'missing author dimension')
  require(row['dimensions']['external_review']['state']=='not-run','invented external review')
  require(row['dimensions']['independent_learner']['state']=='not-assessed','invented delayed learner')
  require(row['dimensions']['physical_observation']['state']=='not-claimed-separate-target-obligation','invented physical claim')
  for label in row.get('required_checks',[]):require(label in checks and checks[label]['passed'],'missing exact mechanism check')
 for label,count in [('spsc',6),('claim',122),('claim-sc',122)]:
  r=checks['GenMC '+label];body=(ROOT/r['raw']).read_text();require(r['expected']==0 and r['returncode']==0 and 'Number of complete executions explored: '+str(count) in body,'wrong RC11 positive bounds')
 for label in ['spsc-relaxed','claim-no-publication','claim-no-reuse','original-two-producers']:
  r=checks['GenMC '+label];require(r['returncode']==r['expected']==42 and r['marker']=='Error: Non-atomic race!','compilation failure substituted for RC11 counterexample')
 for profile in PROFILES:
  for mode in [0,1]:
   r=checks[profile+' threads '+str(mode)+' execute'];body=(ROOT/r['raw']).read_text();require('ATOMIC_SUPPORT size=4 align=4 spsc_lock_free=1 claim_lock_free=1' in body,'missing selected atomic support')
   body=(ROOT/checks[profile+' threads '+str(mode)+' undefined']['raw']).read_text();require('__atomic_' not in body,'unexpected selected host atomic helper')
 # Exact optimized x86 observations, not a portable opcode requirement.
 observed=[]
 for profile in ['gcc-release','clang-release']:
  for mode in [0,1]:
   body=(ROOT/checks[profile+' threads '+str(mode)+' disassembly']['raw']).read_text()
   for symbol in ['b3_publish','b3_take']:
    m=re.search(r'^[0-9a-f]+ <'+symbol+r'>:\n(.*?)(?=^[0-9a-f]+ <|\Z)',body,re.M|re.S);require(m,'missing observed symbol')
    # Register-only xchg %ax,%ax is alignment padding, not a memory exchange.
    exchanges=sum(bool(re.search(r'\bxchg\s+[^\n]*\(',line)) for line in m[1].splitlines())
    require(exchanges==(1 if mode else 0),'changed selected optimized store instruction')
    observed.append(dict(profile=profile,mode='seq_cst' if mode else 'acquire/release',symbol=symbol,xchg=exchanges))
 local_pins(campaign['input_sha256']);local_pins(campaign['outputs_sha256'])
 return dict(rows=len(rows),checks=len(checks),original_roots=5,original_criteria=17,original_delayed=5,observed_codegen=observed,model_complete_executions={'spsc':6,'claim':122,'claim-sc':122})
def external(pins):
 script='import json,hashlib,sys,pathlib; p=json.load(sys.stdin); bad=[k for k,v in p.items() if not pathlib.Path(k).is_file() or hashlib.sha256(pathlib.Path(k).read_bytes()).hexdigest()!=v]; print(json.dumps(bad)); sys.exit(bool(bad))'
 cmd=[sys.executable,'-c',script] if sys.platform!='win32' else ['wsl','python3','-c',script]
 p=subprocess.run(cmd,input=json.dumps(pins),text=True,capture_output=True,timeout=60);require(p.returncode==0,'external identity changed '+p.stdout+p.stderr)
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);a=p.parse_args();s=read(BASE+'support.json');m=read(BASE+'evidence_map.json');require(len(s['current_reports'])==1,'wrong report set');report=s['current_reports'][0];c=read(report)
 b={f'X-T{n}-002':parse_brief(ROOT/f'curriculum/v5/exercises/X-T{n}-002.md') for n in TOPICS};local_pins(s['source_sha256']);result=evidence(s,m,c,b);external(c['external_input_sha256']);controls=[]
 if a.self_test:
  for case in range(12):
   ss,mm,cc,bb=copy.deepcopy((s,m,c,b))
   if case==0:ss['updates'].pop()
   elif case==1:next(r for r in ss['updates'] if 'original_criterion' in r)['original_criterion']['criterion']='generic topic'
   elif case==2:next(r for r in ss['updates'] if r['id'].endswith('DELAYED'))['original_variants']='generic transfer'
   elif case==3:ss['updates'][0]['dimensions']['independent_learner']['state']='passed'
   elif case==4:ss['updates'][0]['dimensions']['physical_observation']['state']='passed'
   elif case==5:cc['checks']=[r for r in cc['checks'] if r['label']!='GenMC claim-no-reuse']
   elif case==6:next(r for r in cc['checks'] if r['label']=='GenMC original-two-producers').update(expected=1,returncode=1)
   elif case==7:next(r for r in ss['updates'] if 'required_checks' in r)['required_checks']=['gcc-debug public execute']
   elif case==8:mm['roots'][0]['criteria'][0]['criterion']='changed original'
   elif case==9:cc['input_sha256'][next(iter(cc['input_sha256']))]='0'*64
   elif case==10:cc['pilot']=True
   else:bb['X-T23-002'][0].pop('b03_followthrough_author_support')
   try:evidence(ss,mm,cc,bb)
   except (ValueError,KeyError):controls.append(dict(case=case,rejected=True))
   else:raise ValueError('negative guard control accepted '+str(case))
 local_pins(s['source_sha256']);result.update(passed=True,status='passed',source_sha256=sha(BASE+'support.json'),source_files=len(s['source_sha256']),output_files=len(c['outputs_sha256']),external_identities=len(c['external_input_sha256']),negative_controls=controls,current_reports=[report],limits=s['remaining_external_work']);write(a.report,result);print(json.dumps(result))
if __name__=='__main__':main()
