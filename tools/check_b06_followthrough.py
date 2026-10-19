"""Validate exact original B06 follow-through identities, leaf bindings and scoped evidence."""
from pathlib import Path
import argparse,copy,hashlib,json,sys
ROOT=Path(__file__).resolve().parents[1]
B=Path('research/v5/expansion/b06')
def load(p):return json.loads(p.read_text(encoding='utf8'))
def validate(data,root=ROOT):
 errors=[];gaps={r['id']:r for r in load(root/B/'gap_ledger.json')['gaps']};rows=data.get('updates',[]);ids=[r.get('id') for r in rows];reports={};source_records={r['id']:r for r in load(root/B/'followup_resources.json')['resources']};expected_dims={'teaching','implementation','author_validation','external_review','independent_learner','physical_observation'}
 if len(ids)!=len(set(ids)) or set(ids)!=set(gaps):errors.append('Original77 gap identities not preserved exactly')
 def file(rel):
  p=root/rel
  try:p.resolve().relative_to(root.resolve())
  except ValueError:errors.append('Outside workspace '+rel);return None
  if not p.is_file():errors.append('Missing file '+rel);return None
  return p
 for rel,want in data.get('historical_inputs',{}).items():
  p=file(rel)
  if p and hashlib.sha256(p.read_bytes()).hexdigest()!=want:errors.append('Historical binding changed '+rel)
 for row in rows:
  ident=row.get('id','');dims=row.get('dimensions',{})
  if set(dims)!=expected_dims:errors.append(ident+': six dimensions required')
  for name,v in dims.items():
   if not v.get('state') or not isinstance(v.get('evidence'),list):errors.append(ident+': malformed dimension '+name);continue
   for rel in v['evidence']:file(rel)
   if v['state'] in ['complete','passed'] and not v['evidence']:errors.append(ident+': unsupported completion '+name)
  if dims.get('external_review',{}).get('state')!='not-run' or dims.get('independent_learner',{}).get('state')!='not-assessed':errors.append(ident+': author evidence cannot claim independent assessment')
  if ident.endswith('-TRANSFER'):
   expected=set(gaps[ident]['objective_ids']);bindings=row.get('criterion_bindings',[])
   if len(bindings)!=len(expected) or {x.get('objective') for x in bindings}!=expected:errors.append(ident+': exact per-leaf binding missing')
   if row.get('original_transfer')!=gaps[ident]['description']:errors.append(ident+': original transfer changed')
   for s in row.get('source_records',[]):
    record=source_records.get(s.get('id'));file(s['path'])
    if not record or not s.get('passages') or any(x not in record['locators'] for x in s['passages']):errors.append(ident+': uninspected source locator')
   for binding in bindings:
    for rel in [binding['rubric_path'],binding['shared_contract'],*binding['teaching'],*binding['implementation']]:file(rel)
    rubric=root/binding['rubric_path']
    if rubric.is_file() and ('**'+binding['objective']+'**') not in rubric.read_text(encoding='utf8'):errors.append(ident+': original rubric not found')
    assessment=binding['independent_assessment']
    if assessment.get('state')!='not-assessed' or assessment.get('minimum_days',0)<7 or assessment.get('original_transfer_id')!=ident:errors.append(ident+': independent transfer requirement changed')
    for evidence in binding['author_validation']:
     rel=evidence['path'];p=file(rel)
     if not p:continue
     if rel not in reports:
      report=load(p);reports[rel]=report
      if not report.get('passed'):errors.append('Report did not pass '+rel)
      pins=report.get('input_sha256',report.get('source_sha256',report.get('inspected_paths',{})))
      if not pins:errors.append('Unbound report '+rel)
      for src,want in pins.items():
       q=file(src)
       if q and hashlib.sha256(q.read_bytes()).hexdigest()!=want:errors.append('Stale report '+rel+' -> '+src)
     report=reports[rel];checks=report.get('checks',[]);actual={c.get('label',c.get('kind',c.get('name'))) for c in checks if c.get('passed')};actual.update('profile:'+p['name'] for p in report.get('profiles',[]) if p.get('build_exit')==0 and p.get('execute_exit')==0 and p.get('assembly') and all(p['assembly'].values()) and p.get('raw_samples'));actual.update('no-avx:'+p['name'] for p in report.get('profiles',[]) if p.get('no_avx_emulator',{}).get('passed') and p['no_avx_emulator'].get('exit')==0 and 'avx_usable=0' in p['no_avx_emulator'].get('output',''))
     if not evidence.get('checks') or any(label not in actual for label in evidence['checks']):errors.append(ident+': missing/failed exact report check')
 return list(dict.fromkeys(errors)),len(reports)
def main():
 ap=argparse.ArgumentParser();ap.add_argument('--self-test',action='store_true');a=ap.parse_args();data=load(ROOT/B/'completion_dispositions.json');errors,reports=validate(data);controls=[]
 if a.self_test:
  for name,change in [('missing original ID',lambda d:d['updates'].pop()),('missing leaf criterion',lambda d:d['updates'][0]['criterion_bindings'].pop()),('invented successful check',lambda d:d['updates'][0]['criterion_bindings'][0]['author_validation'][0]['checks'].append('NEVER_EXECUTED')),('invented independent review',lambda d:d['updates'][0]['dimensions']['external_review'].update(state='passed'))]:
   mutant=copy.deepcopy(data);change(mutant);found,_=validate(mutant);controls.append({'name':name,'rejected':bool(found)})
  if not all(c['rejected'] for c in controls):errors.append('Negative structural control survived')
 result={'passed':not errors,'original_ids':len(data['updates']),'leaf_bindings':sum(len(r.get('criterion_bindings',[])) for r in data['updates']),'current_reports':reports,'negative_controls':controls,'errors':errors,'limits':'Author source/identity/evidence binding only; no external review, learner acceptance or physical observation inferred.'};print(json.dumps(result,indent=2));return 0 if not errors else 1
if __name__=='__main__':raise SystemExit(main())
