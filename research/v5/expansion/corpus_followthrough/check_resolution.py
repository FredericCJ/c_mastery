"""Exact finite-screen audit; source mapping is not a new execution receipt."""
import argparse,copy,hashlib,json,pathlib,re
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
def read(p):return json.loads(p.read_text(encoding='utf-8-sig'))
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def check(data,screen):
 errors=[];origins=read(HERE/'finite84_origins.json');expected={r['id']:r for r in origins['origins']};rows=data.get('resolutions',[])
 if len(rows)!=84 or {r['id'] for r in rows}!=set(expected):errors.append('exact84 identities')
 if data.get('input_metadata_sha256')!=origins['input_metadata_sha256'] or screen.get('inputs')!=origins['input_metadata_sha256']:errors.append('metadata identity')
 elements=screen.get('elements',[])
 if len(elements)!=1083 or {r['id'] for r in elements if r['metadata_description_read']}!=set(expected):errors.append('screen boundary')
 if sum(not r['metadata_description_read'] for r in elements)!=999:errors.append('unread count')
 for r in elements:
  if r['id'] in expected:
   if r.get('description_identity_sha256')!=expected[r['id']]['description_identity_sha256']:errors.append('description identity')
   if r.get('resolution_id')!=r['id'] or r.get('status')!='bounded-mechanism-implemented':errors.append('unresolved selected description')
   if r.get('source_passages_inspected') is not False:errors.append('metadata promoted to passage inspection')
  elif r.get('status')!='scope-routing-not-content-inspection':errors.append('unread element promoted')
 for row in rows:
  if row.get('status')!='bounded-mechanism-implemented' or not row.get('mechanism') or not row.get('limits') or not row.get('inspection_provenance'):errors.append('missing mechanism/limits')
  evidence=row.get('evidence',[]);pins=row.get('evidence_sha256',{})
  if set(evidence)!=set(pins) or not evidence:errors.append('evidence pin scope')
  if not any(n.startswith('assessor/') and n.endswith('.c') for n in evidence):errors.append('no actual implementation')
  for n,h in pins.items():
   p=ROOT/n
   if not p.is_file() or sha(p)!=h:errors.append('changed evidence '+n)
  ids=set()
  for n in evidence:
   p=ROOT/n
   if n.startswith('curriculum/v5/exercises/X-') and p.is_file():
    meta=json.loads(re.search(r'```json\s*(.*?)```',p.read_text(encoding='utf8'),re.S)[1]);ids|={r['id'] for r in meta['rubric']}
   elif n.startswith('curriculum/v5/expansion/legacy_assessments/') and p.is_file():ids|={r['id'] for r in read(p)['criteria']}
  if not row.get('criteria') or not set(row['criteria'])<=ids:errors.append('missing exact criterion')
  for dimension in ['external_review','independent_learner','physical_observation']:
   if row.get('dimensions',{}).get(dimension)!='not established by this mapping':errors.append('inflated evidence dimension')
  if row.get('dimensions',{}).get('author_validation')!='provider-specific evidence; not rerun by this mapping':errors.append('invented rerun')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--report');a=p.parse_args();data=read(HERE/'mechanism_resolutions.json');screen=read(HERE/'screening_map.json');errors=check(data,screen);controls=[]
 if a.self_test:
  cases=[('omitted-description',lambda d,s:d['resolutions'].pop()),('repeated-description',lambda d,s:d['resolutions'].__setitem__(1,copy.deepcopy(d['resolutions'][0]))),('changed-description',lambda d,s:s['elements'].__setitem__(next(i for i,r in enumerate(s['elements']) if r['metadata_description_read']),dict(next(r for r in s['elements'] if r['metadata_description_read']),description_identity_sha256='0'*64))),('false-source-inspection',lambda d,s:next(r for r in s['elements'] if r['metadata_description_read']).update(source_passages_inspected=True)),('unread-promoted',lambda d,s:next(r for r in s['elements'] if not r['metadata_description_read']).update(status='bounded-mechanism-implemented')),('missing-criterion',lambda d,s:d['resolutions'][0].update(criteria=['invented'])),('missing-limits',lambda d,s:d['resolutions'][0].update(limits='')),('changed-source',lambda d,s:d['resolutions'][0]['evidence_sha256'].update({d['resolutions'][0]['evidence'][0]:'0'*64})),('invented-rerun',lambda d,s:d['resolutions'][0]['dimensions'].update(author_validation='passed current global campaign'))]
  for k in ['external_review','independent_learner','physical_observation']:cases.append(('inflated-'+k,lambda d,s,k=k:d['resolutions'][0]['dimensions'].update({k:'passed'})))
  for name,fn in cases:
   d=copy.deepcopy(data);s=copy.deepcopy(screen);fn(d,s);controls.append(dict(control=name,rejected=bool(check(d,s))))
  errors+=['vacuous '+r['control'] for r in controls if not r['rejected']]
 files=[HERE/n for n in ['author_resolutions.py','build_screening.py','check_resolution.py','finite84_origins.json','mechanism_resolutions.json','screening_map.json','README.md']]
 report=dict(passed=not errors,resolved_descriptions=84,metadata_only_descriptions=999,negative_controls=controls,errors=errors,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limits='Exact mechanism/reference mapping; provider executions retain their own scopes and final integrated validation remains separate.')
 if a.report:
  out=ROOT/a.report;out.parent.mkdir(parents=True,exist_ok=True);out.write_text(json.dumps(report,indent=2)+'\n',encoding='utf8')
 print(json.dumps(report,indent=2));raise SystemExit(bool(errors))
if __name__=='__main__':main()
