"""Versioned eight-line range-encoding repair; retained reports stay historical."""
from pathlib import Path
import argparse,hashlib,json,re
ROOT=Path(__file__).resolve().parents[1]
ISSUE='research/v5/modern/dispatch_publication_text_issue.json'
HISTORY='research/v5/modern/history/dispatch_encoding_20261018'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--apply',action='store_true');a=p.parse_args();d=json.loads((ROOT/ISSUE).read_text(encoding='utf8'));provider=ROOT/d['provider'];old=provider.read_bytes();text=old.decode('utf8');rows=[]
 for row in d['exact_affected_lines']:
  before=row['exact_text'];after,n=re.subn(r'(?<=\d)[^\x00-\x7f]+(?=\d)',' through ',before)
  assert n and before!=after;rows.append(dict(line=row['line'],before=before,after=after,range_count=n))
 if not a.apply:print(json.dumps(rows,ensure_ascii=False,indent=2));return
 history=ROOT/HISTORY;record=history/'correction.json'
 if record.exists():
  current=json.loads(record.read_text());assert sha(provider)==current['corrected_sha256'];print('Versioned correction already applied');return
 assert sha(provider)==d['provider_sha256']
 for row in rows:
  assert text.splitlines()[row['line']-1]==row['before'] and text.count(row['before'])==1;text=text.replace(row['before'],row['after'])
 paths=[d['provider'],ISSUE,'research/v5/modern/dispatch_objective_readiness.json','research/v5/modern/dod_objective_readiness.json','reviews/v5/expansion/modern_completion/dispatch_campaign.json']
 paths+=['research/v5/modern/ownership_authority_boundary_'+suffix+'.json' for suffix in ['resources','research_gate','validation','investigation_validation','readiness','completion_dispositions']]
 snapshots={}
 for name in paths:
  target=history/name;target.parent.mkdir(parents=True,exist_ok=True);assert not target.exists();target.write_bytes((ROOT/name).read_bytes());snapshots[name]=dict(path=target.relative_to(ROOT).as_posix(),sha256=sha(target))
 # Preserve all original newline bytes outside these exact replacements.
 provider.write_bytes(text.encode('utf8'))
 record.write_text(json.dumps(dict(id=d['id'],provider=d['provider'],prior_sha256=hashlib.sha256(old).hexdigest(),corrected_sha256=sha(provider),changes=rows,snapshots=snapshots,implementation_semantics_changed=False,validation='pending current producer execution and consumer reconciliation',inspected_implementation=['assessor/dispatch/dispatch.c:ds_fold,capability,loaded,host functions','assessor/dispatch/capture.c:counted allocator,capture,typed functions','labs/dispatch/tests/plugin.c:apply,migrate']),ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
 print(dict(corrected_lines=len(rows),ranges=sum(r['range_count'] for r in rows),record=record.relative_to(ROOT).as_posix()))
if __name__=='__main__':main()
