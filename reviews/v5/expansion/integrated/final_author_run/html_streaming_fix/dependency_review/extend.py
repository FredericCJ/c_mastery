from pathlib import Path
import sys,json,hashlib,re
ROOT=Path.cwd();sys.path.insert(0,str(ROOT/'tools'))
from check_residual_followthrough import pins,local
OUT=ROOT/'reviews/v5/expansion/integrated/final_author_run/html_streaming_fix/dependency_review'
TARGETS={'tools/check_html.py','tools/check_html_streaming.py','tools/build_v5.py','site/conf.py','tools/check_navigation_v5.py'}
prior=json.loads((OUT/'audit_01.json').read_text(encoding='utf8'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
mentions=[];hits=[];changed=[];failures=[];hashes={};pat=re.compile('[0-9a-f]{64}')
def walk(v,location=''):
 if isinstance(v,dict):
  for k,x in v.items():
   if k in TARGETS:yield {'field':location+'/'+k,'path':k,'expected':x if isinstance(x,str) and pat.fullmatch(x) else None,'kind':'target-key'}
   if isinstance(x,str) and x in TARGETS:
    h=v.get('sha256');yield {'field':location+'/'+k,'path':x,'expected':h if isinstance(h,str) and pat.fullmatch(h) else None,'kind':'target-value'}
   elif isinstance(x,(dict,list)):yield from walk(x,location+'/'+k)
 elif isinstance(v,list):
  for i,x in enumerate(v):
   if isinstance(x,str) and x in TARGETS:yield {'field':location+'/'+str(i),'path':x,'expected':None,'kind':'unhashed-list-reference'}
   else:yield from walk(x,location+'/'+str(i))
for name,before in prior['document_sha256'].items():
 try:
  raw=(ROOT/name).read_bytes();h=hashlib.sha256(raw).hexdigest();hashes[name]=h
  if h!=before:changed.append(name)
  d=json.loads(raw.decode('utf-8-sig'))
  for pointer,target,expected in pins(d):
   normalized=local(target)
   if normalized in TARGETS:hits.append({'document':name,'field':pointer,'path':normalized,'expected':expected,'direct_active_provider':name in prior['providers']})
  for r in walk(d):mentions.append({'document':name,'direct_active_provider':name in prior['providers'],**r})
 except (OSError,ValueError,UnicodeError) as e:failures.append({'path':name,'error':str(e)})
provider_stable=all(sha(ROOT/p)==r['sha256'] for p,r in prior['providers'].items()) and sha(ROOT/prior['canonical']['path'])==prior['canonical']['sha256']
result={'scope':'Five-target extension against all current and previously reachable documents discovered by audit_01; exact recursive SHA-map recognition plus generic target-map and unhashed-reference recognition. Current source hash changes still require global validation; zero current hash dependencies do not make old whole-source reports current.','targets':{n:sha(ROOT/n) if (ROOT/n).is_file() else None for n in sorted(TARGETS)},'active_rows':prior['active_rows'],'candidate_exact_row_providers':len(prior['providers']),'documents':len(hashes),'document_sha256':hashes,'recognized_target_pins':hits,'all_target_mentions':mentions,'changed_documents_since_first_pass':changed,'provider_inputs_stable':provider_stable,'errors':failures,'original_execution_reports_rewritten':False,'new_c_execution':False}
(OUT/'audit_02.json').write_text(json.dumps(result,indent=2),encoding='utf8')
print(json.dumps({k:result[k] for k in ['active_rows','candidate_exact_row_providers','documents','recognized_target_pins','all_target_mentions','changed_documents_since_first_pass','provider_inputs_stable','errors']},indent=2))
