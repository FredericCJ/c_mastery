"""Regression for lossless named changed-condition extraction in the inventory."""
from pathlib import Path
import hashlib,json
from reconcile_implementation_v5 import ROOT,variant_requirement,parse,extract
def main():
 rows=[]
 def require(ok,name):
  rows.append(dict(check=name,passed=bool(ok)))
  if not ok:raise AssertionError(name)
 body='## Variants\n\nV1: Change ownership. V2: Change failure atomicity.\n\nAttempt after seven days.\n\n## Security\nV2: unrelated.'
 require(variant_requirement(body,'V1',['V1','V2'])=='V1: Change ownership.','inline first excludes second')
 require(variant_requirement(body,'V2',['V1','V2'])=='V2: Change failure atomicity.','inline second exact task')
 require(variant_requirement(body.replace(' V2:','\nV2:'),'V2',['V1','V2'])=='V2: Change failure atomicity.','separate line remains exact')
 require(variant_requirement(body,'V3',['V1','V2','V3']) is None,'missing task never invented')
 require(variant_requirement('## Variants\n\nV10: unrelated. V1: exact.\n','V1',['V1','V10'])=='V1: exact.','local label token boundary')
 pins={Path(__file__),ROOT/'tools/reconcile_implementation_v5.py'}
 for name,wanted in [('X-T36-001','V2: Stream edges from untrusted bytes and preserve checks under truncation.'),('X-T48-001','V2: Define a partial-copy count/fault API and compare caller recovery obligations.'),('X-T42-002','V2: integrate the parser into a16-bit-size_t model with the same wire limit, without storing admitted body lengths above1024 in size_t.')]:
  path=ROOT/'curriculum/v5/exercises'/(name+'.md');pins.add(path);data,body=parse(path);tasks={ident:row for ident,_,row,_ in extract(data,path.relative_to(ROOT).as_posix())};require(tasks[name+'-V2']['requirement']==wanted,'actual '+name+' V2');require('V2:' not in tasks[name+'-V1']['requirement'],'actual '+name+' separated V1');require(tasks[name+'-V2']['original_local_id']=='V2','original local identity preserved')
 result=dict(passed=all(x['passed'] for x in rows),checks=rows,input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in sorted(pins)},limits=['Corrects derived task extraction, not exercise semantics or author/learner completion.']);p=ROOT/'reviews/v5/expansion/followthrough/inventory_variants.json';p.write_text(json.dumps(result,indent=2)+'\n');print(dict(passed=result['passed'],checks=len(rows)))
if __name__=='__main__':main()
