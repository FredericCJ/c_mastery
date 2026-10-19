from pathlib import Path
import os,sys,json,hashlib,re
from collections import deque
ROOT=Path.cwd();sys.path.insert(0,str(ROOT/'tools'))
from audit_active_disposition_pins import record_hash
from audit_current_envelopes import historical
from check_residual_followthrough import pins,local
OUT=ROOT/'reviews/v5/expansion/integrated/final_author_run/html_streaming_fix/dependency_review'
TARGETS={'tools/check_html.py','tools/check_html_streaming.py','tools/build_v5.py'}
def sha(p):
 h=hashlib.sha256()
 with p.open('rb') as f:
  for part in iter(lambda:f.read(1048576),b''):h.update(part)
 return h.hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
canonical='curriculum/v5/expansion/implementation_updates.json';active=read(ROOT/canonical);rows={r['id']:record_hash(r) for r in active['updates']};canonical_sha=sha(ROOT/canonical)
prior_path='reviews/v5/expansion/integrated/final_author_run/publisher_scope_fix/active_provider_dependencies_02.json';prior=read(ROOT/prior_path)
providers={};errors=[]
for area in ['research/v5/expansion','research/v5/modern']:
 for directory,dirs,files in os.walk(ROOT/area):
  dirs[:]=[d for d in dirs if not historical(d)]
  for name in files:
   if not name.endswith('.json') or historical(name):continue
   p=Path(directory)/name
   try:d=read(p)
   except (OSError,ValueError,UnicodeError) as e:errors.append({'path':p.relative_to(ROOT).as_posix(),'error':str(e)});continue
   if not isinstance(d,dict) or not isinstance(d.get('updates'),list):continue
   selected=[r['id'] for r in d['updates'] if isinstance(r,dict) and r.get('id') in rows and record_hash(r)==rows[r['id']]]
   if selected:providers[p.relative_to(ROOT).as_posix()]={'active_ids':selected,'sha256':sha(p)}
queue=deque(sorted(set(providers)|set(prior['document_sha256'])));seen=set();documents={};hits=[];mentions=[];missing=[];edges=0
hexsha=re.compile('[0-9a-f]{64}')
def references(value,location=''):
 if isinstance(value,dict):
  for key,item in value.items():
   if key in TARGETS:yield {'field':location+'/'+key,'path':key,'expected':item if isinstance(item,str) and hexsha.fullmatch(item) else None,'kind':'target-key'}
   if isinstance(item,str) and item in TARGETS:
    expected=value.get('sha256');yield {'field':location+'/'+key,'path':item,'expected':expected if isinstance(expected,str) and hexsha.fullmatch(expected) else None,'kind':'target-value'}
   yield from references(item,location+'/'+str(key))
 elif isinstance(value,list):
  for i,item in enumerate(value):yield from references(item,location+'/'+str(i))
while queue:
 name=queue.popleft()
 if name in seen:continue
 seen.add(name);p=ROOT/name
 if not p.is_file():missing.append(name);continue
 try:
  raw=p.read_bytes();d=json.loads(raw.decode('utf-8-sig'));documents[name]=hashlib.sha256(raw).hexdigest()
 except (OSError,ValueError,UnicodeError) as e:errors.append({'path':name,'error':str(e)});continue
 for pointer,target,expected in pins(d):
  try:normalized=local(target)
  except (ValueError,TypeError):continue
  if normalized in TARGETS:hits.append({'document':name,'field':pointer,'path':normalized,'expected':expected,'direct_active_provider':name in providers})
  if normalized and normalized.endswith('.json'):
   edges+=1
   if (ROOT/normalized).is_file() and normalized not in seen:queue.append(normalized)
 for item in references(d):mentions.append({'document':name,'direct_active_provider':name in providers,**item})
stable=sha(ROOT/canonical)==canonical_sha and all(sha(ROOT/n)==r['sha256'] for n,r in providers.items())
result={'scope':'Read-only exact active-row provider discovery plus prior reachable JSON closure and newly recognized current edges. Generic target-key/value scan additionally detects whole-source file mappings; preserved reachable receipts are not implicitly current.','active_rows':len(rows),'canonical':{'path':canonical,'sha256':canonical_sha},'provider_count':len(providers),'providers':providers,'previous_traversal':{'path':prior_path,'sha256':sha(ROOT/prior_path)},'documents':len(documents),'document_sha256':documents,'edges':edges,'recognized_target_pins':hits,'all_target_mentions':mentions,'target_sources_at_read':{n:sha(ROOT/n) if (ROOT/n).is_file() else None for n in sorted(TARGETS)},'missing_reachable_json':missing,'errors':errors,'provider_inputs_stable':stable,'original_execution_reports_rewritten':False,'new_c_execution':False}
(OUT/'audit_01.json').write_text(json.dumps(result,indent=2),encoding='utf8')
print(json.dumps({k:result[k] for k in ['active_rows','provider_count','documents','provider_inputs_stable','recognized_target_pins','all_target_mentions','errors','missing_reachable_json']},indent=2))
