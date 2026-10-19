"""Validate exact original T71/T72 gap/rubric/delayed bindings and current author receipts."""
from pathlib import Path
import argparse,copy,hashlib,json,re,sys
ROOT=Path(__file__).resolve().parents[1]
DIMENSIONS={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'}
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def file(p):
 if not isinstance(p,str) or not p or Path(p).is_absolute():raise ValueError('Invalid relative evidence path')
 candidate=(ROOT/p).resolve();candidate.relative_to(ROOT.resolve())
 if not candidate.is_file():raise ValueError('Missing evidence '+p)
 return candidate
def sha(p):return hashlib.sha256(file(p).read_bytes()).hexdigest()
def metadata(path):return json.loads(re.search(r'```json\s*(.*?)\s*```',file(path).read_text(encoding='utf8'),re.S)[1])
def validate(data,family,receipts=True):
 domain=data['domain'];old=read(f'research/v5/modern/{family}_gap_ledger.json');updates={x['id']:x for x in data['updates']}
 if len(updates)!=len(data['updates']):raise ValueError('Duplicate original update ID')
 bindings={x['rubric_id']:x for x in data['criterion_bindings']}
 if len(bindings)!=len(data['criterion_bindings']):raise ValueError('Duplicate criterion binding')
 expected_ids={g['id'] for g in old['gaps']};expected_criteria=set()
 for gap in old['gaps']:
  if not gap.get('exercise'):continue
  x=gap['exercise'];m=metadata(f'curriculum/v5/exercises/{x}.md');expected_ids.add(x);expected_ids.update(m['delayed_assessment']['variant_ids'])
  if m.get('delayed_transfer')!=m['delayed_assessment']:raise ValueError('Canonical executable delayed policy differs from original assessment')
  for r in m['rubric']:
   expected_ids.add(r['id']);expected_criteria.add(r['id']);b=bindings.get(r['id'])
   if not b or (b['objective'],b['exercise'],b['criterion'])!=(r['objective'],x,r['criterion']):raise ValueError('Missing or changed exact criterion '+r['id'])
   if b['independent_assessment']!={'state':'not-assessed',**m['delayed_assessment']}:raise ValueError('Independent/delayed evidence overstated')
   for facet in ['teaching','implementation','assessor_analysis']:
    if not b.get(facet):raise ValueError('Missing facet '+facet)
    for p in b[facet]:file(p)
   if not b['author_validation']:raise ValueError('Missing author checks')
  for contract in [f'labs/{family}/followthrough/CONTRACTS.md',f'labs/{family}/followthrough/ADAPTERS.md']:
   if contract not in m['shared_contracts']:raise ValueError('New mechanism contract not attached to original exercise')
  if x+'-DELAYED' not in file(f'labs/{family}/followthrough/CONTRACTS.md').read_text(encoding='utf8'):raise ValueError('Delayed variant missing from concrete contract')
 if set(updates)!=expected_ids or set(bindings)!=expected_criteria:raise ValueError('Original ID reconciliation mismatch')
 for row in updates.values():
  if set(row['dimensions'])!=DIMENSIONS:raise ValueError('Missing evidence dimension')
  if row['dimensions']['external_review']['state']!='not-run':raise ValueError('False external review claim')
  if row['dimensions']['independent_learner']['state'] not in ['not-assessed','pending-actual-learner-imports']:raise ValueError('False learner claim')
  for d in row['dimensions'].values():
   if not d.get('state') or 'evidence' not in d:raise ValueError('Malformed dimension')
   for p in d['evidence']:file(p)
  for p in row['artifacts']:file(p)
 for p,digest in data['input_sha256'].items():
  if sha(p)!=digest:raise ValueError('Stale disposition input '+p)
 gate=read(f'research/v5/modern/{family}_followthrough_research_gate.json');resources=read(f'research/v5/modern/{family}_followthrough_resources.json')['resources']
 if gate['status']!='passage-inspected' or set(gate['resource_ids'])!={r['id'] for r in resources}:raise ValueError('Source gate IDs mismatch')
 for p,digest in gate['research_input_sha256'].items():
  if sha(p)!=digest:raise ValueError('Stale source gate '+p)
 for r in resources:
  if not r.get('locators') or not all(r['inspection'].get(k) for k in ['date','method','scope','limits']):raise ValueError('Missing bounded source inspection')
 reports={}
 if receipts:
  for p in data['current_reports']:
   report=read(p);reports[p]=report
   if report.get('passed') is not True or not report.get('checks') or not all(c.get('passed') is True for c in report['checks']):raise ValueError('Author report failed or empty '+p)
   pins=report.get('input_sha256')
   if not pins:raise ValueError('Unbound report '+p)
   for source,digest in pins.items():
    if sha(source)!=digest:raise ValueError('Stale report '+p+' input '+source)
  for b in bindings.values():
   for selected in b['author_validation']:
    if selected['path'] not in reports:raise ValueError('Unregistered criterion receipt')
    actual={r['label'] for r in reports[selected['path']]['checks']}
    if not selected['checks'] or not set(selected['checks'])<=actual:raise ValueError('Invented named command')
 return dict(domain=domain,original_gaps=len(old['gaps']),updates=len(updates),criteria=len(bindings),reports=len(data['current_reports']))
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');ap.add_argument('--structure-only',action='store_true');args=ap.parse_args();results=[];controls=[]
 for family in ['protocols','coroutines']:
  data=read(f'research/v5/modern/{family}_completion_dispositions.json');results.append(validate(data,family,not args.structure_only))
  if args.self_test:
   for mutation in ['missing-gap','missing-criterion','false-learner','false-objective']:
    bad=copy.deepcopy(data)
    if mutation=='missing-gap':bad['updates'].pop()
    elif mutation=='missing-criterion':bad['criterion_bindings'].pop()
    elif mutation=='false-learner':bad['updates'][0]['dimensions']['independent_learner']['state']='passed'
    else:bad['criterion_bindings'][0]['objective']='T00.00.a'
    try:validate(bad,family,False)
    except (ValueError,KeyError):controls.append(family+':'+mutation)
    else:raise ValueError('Negative control escaped '+mutation)
 print(json.dumps(dict(passed=True,scope='structure-only' if args.structure_only else 'current-source-bound-author-evidence',domains=results,negative_controls=controls,limits=['No independent learner, external review or physical completion inferred']),indent=2))
if __name__=='__main__':
 try:main()
 except (ValueError,KeyError,OSError) as e:print(str(e),file=sys.stderr);raise SystemExit(1)
