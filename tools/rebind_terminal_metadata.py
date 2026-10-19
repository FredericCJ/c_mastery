"""Version terminal metadata after additive CAN attachment; preserve experiment bytes."""
import argparse,copy,json
from common import ROOT,read_json,dump,sha
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--self-test',action='store_true');a=p.parse_args()
 import sys
 sys.path.insert(0,str(ROOT))
 from research.v5.expansion.legacy_posix_terminal.author_support import attach,can_successor
 path=ROOT/'research/v5/expansion/legacy_posix_terminal/support.json';value=read_json(path);old=copy.deepcopy(value);digest=sha(path)
 receipt=ROOT/'reviews/v5/expansion/legacy_posix_terminal/terminal_validation.json';receipt_digest=sha(receipt)
 changed=[]
 for name,h in value['source_sha256'].items():
  if sha(ROOT/name)!=h:
   assert name in ['curriculum/v5/expansion/legacy_assessments/PR-T23-04.json','curriculum/v5/expansion/legacy_assessments/PR-T24-06.json'],name
   current=read_json(ROOT/name);assert attach(copy.deepcopy(current))==current
   value['source_sha256'][name]=sha(ROOT/name);changed.append(name)
 successor=can_successor();assert successor,'CAN evidence must be current before retiring author-only backlog'
 value['superseded_remaining_author_work']=[successor];value['remaining_author_work']=[]
 assert value['updates']==old['updates']
 snapshot=ROOT/'reviews/v5/expansion/legacy_posix_terminal/metadata_before_can'/('support-'+digest+'.json')
 if not snapshot.is_file():snapshot.parent.mkdir(parents=True,exist_ok=True);snapshot.write_bytes(path.read_bytes())
 dump(path,value);assert sha(receipt)==receipt_digest
 controls=[]
 if a.self_test:
  for name in ('PR-T23-04','PR-T24-06'):
   record=read_json(ROOT/'curriculum/v5/expansion/legacy_assessments'/ (name+'.json'));assert attach(copy.deepcopy(record))==record;controls.append(name+' attachment remains idempotent')
  controls+=['original update bodies preserved','original terminal experiment receipt byte-identical']
 correction=dict(schema_version=1,passed=True,kind='metadata-only additive CAN successor; no fabricated rerun',original_support=dict(path=snapshot.relative_to(ROOT).as_posix(),sha256=digest),changed_bindings=changed,negative_or_preservation_controls=controls,unchanged_experiment=dict(path=receipt.relative_to(ROOT).as_posix(),sha256=receipt_digest),input_sha256={n:sha(ROOT/n) for n in ['tools/rebind_terminal_metadata.py','research/v5/expansion/legacy_posix_terminal/author_support.py',path.relative_to(ROOT).as_posix(),'research/v5/expansion/legacy_can/support.json']},physical_limit='Actual serial/CAN electrical observations remain required in their original/source successor gaps')
 dump(ROOT/'research/v5/expansion/legacy_posix_terminal/can_binding_successor.json',correction);print(json.dumps(dict(passed=True,changed_bindings=changed,controls=len(controls))))
if __name__=='__main__':main()
