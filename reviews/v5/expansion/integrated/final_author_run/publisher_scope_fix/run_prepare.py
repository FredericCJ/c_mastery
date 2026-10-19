from pathlib import Path
from datetime import datetime,timezone
import json,hashlib,sys,traceback
from unittest.mock import patch
sys.path.insert(0,'tools')
from common import ROOT,dump as real_dump,read_json,normalize_prose
import publish,publish_v4,publish_v5
from resource_inputs_v5 import discover,sha
out=ROOT/'reviews/v5/expansion/integrated/final_author_run/publisher_scope_fix/prepare'
out.mkdir(parents=True,exist_ok=True)
_,before=discover(ROOT);registry=sha(ROOT/'references/resources.json')
def dump(path,value):
 path=Path(path)
 if path.is_relative_to(ROOT/'reviews'):path=out/'receipts'/path.relative_to(ROOT/'reviews')
 real_dump(path,value)
print('START',datetime.now(timezone.utc).isoformat(),flush=True)
result={'operation':'actual publish_v5.prepare; review receipts redirected only','source_registers_before':before['inputs'],'registry_before':registry}
try:
 with patch.object(publish,'dump',dump),patch.object(publish_v4,'dump',dump):
  source,html=publish_v5.prepare()
 records=read_json(ROOT/'references/resources.json')['resources'];missing=[];mismatch=[]
 for r in records:
  path=source/'references'/(r['id']+'.md')
  if not path.is_file():missing.append(r['id']);continue
  scope=normalize_prose(publish.inspection_scope_text(r['inspection']['scope']))
  if scope not in path.read_text(encoding='utf8'):mismatch.append(r['id'])
 _,after=discover(ROOT)
 result.update(passed=not missing and not mismatch and before['inputs']==after['inputs'] and registry==sha(ROOT/'references/resources.json'),resource_pages=len(records),missing=missing,scope_mismatch=mismatch,all_source_registers_unchanged=before['inputs']==after['inputs'],registry_unchanged=registry==sha(ROOT/'references/resources.json'),generated_source=str(source))
except BaseException as e:
 result.update(passed=False,error=type(e).__name__+': '+str(e));traceback.print_exc()
result['finished']=datetime.now(timezone.utc).isoformat();result['source_sha256']={n:sha(ROOT/n) for n in ['tools/publish.py','tools/check_publication_scopes.py']};real_dump(out/'validation.json',result);print(json.dumps({k:v for k,v in result.items() if k!='source_registers_before'}),flush=True)
sys.exit(0 if result['passed'] else 1)
