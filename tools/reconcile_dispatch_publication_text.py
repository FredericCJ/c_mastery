"""Rebind active authored readiness after exact versioned correction execution."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def put(p,d):(ROOT/p).write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def main():
 issue='research/v5/modern/dispatch_publication_text_issue.json';history='research/v5/modern/history/dispatch_encoding_20261018/';correction=read(history+'correction.json');report='reviews/v5/expansion/modern_completion/dispatch_publication_correction.json';r=read(report);assert r['passed'] and r['export_count']==14 and len(r['commands'])==21
 for p,h in r['input_sha256'].items():assert sha(p)==h,p
 assert all(c['passed'] and c['returncode']==c['expected'] for c in r['commands'])
 provider=correction['provider'];campaign='reviews/v5/expansion/modern_completion/dispatch_campaign.json';allowed={provider:correction['prior_sha256'],campaign:correction['snapshots'][campaign]['sha256']};changes=[]
 for family in ['dispatch','dod']:
  path='research/v5/modern/'+family+'_objective_readiness.json';d=read(path)
  for row in d['criteria']:
   for p,h in list(row.get('pins',{}).items()):
    now=sha(p)
    if h!=now:
     assert allowed.get(p)==h,(path,p,'unrelated stale input');row['pins'][p]=now;changes.append(dict(record=path,criterion=row['id'],path=p,before=h,after=now))
  # These are active authored mappings, not execution receipts. Preserve their
  # pre-correction snapshots, retain every original claim, and identify the run
  # authorizing only these two changed bindings.
  d['publication_correction']=dict(record=history+'correction.json',evidence=report,scope='Numeric-range encoding only; original criteria/claims and non-author dimensions unchanged.');put(path,d)
 assert len(changes) in (0,92),len(changes)
 d=read(issue);d.update(status='author-repair-executed-external-evidence-unchanged',repair_executed=True,correction_record=history+'correction.json',validation_report=report)
 d['required_work'][0].update(state='author-repair-and-current-validation-complete',closure='Eight exact lines/13 ranges corrected against actual bounds; current dispatch and authority producers and14 actual exports passed, preserving historical snapshots. Final whole-repository publication verification remains required.')
 put(issue,d)
 artifacts=[provider,history+'correction.json','research/v5/modern/dispatch_objective_readiness.json','research/v5/modern/dod_objective_readiness.json',issue,report,'tools/reconcile_dispatch_publication_text.py'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',[provider]),('implementation','not-applicable-text-correction',[history+'correction.json']),('author_validation','passed',[report]),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-applicable-text-correction',[history+'correction.json'])]}
 put('research/v5/modern/dispatch_publication_completion_dispositions.json',dict(schema_version=1,updates=[dict(id=d['id'],dimensions=dims,closure_rationale=d['required_work'][0]['closure'])],current_reports=[report],input_sha256={p:sha(p) for p in artifacts},changed_active_bindings=changes,historical_policy='Original provider and live pre-correction reports are retained byte-for-byte in the correction snapshot; accepted historical receipts are untouched.'))
 print(dict(passed=True,changed_bindings=len(changes)))
if __name__=='__main__':main()
