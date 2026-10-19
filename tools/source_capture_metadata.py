"""Distributable failed-capture identity; never promote it to source inspection."""
import argparse,copy,hashlib,json,pathlib,tempfile
ROOT=pathlib.Path(__file__).resolve().parents[1]
BASE=ROOT/'research/v5/expansion/source_capture_metadata'
REPORT='research/v5/expansion/source_capture_metadata/n3245_failed_capture.json'
ORIGIN='research/v5/expansion/legacy_portability_followthrough/replacement_research_gate.json'
IDENT='IW-a8b7587eadd8ddb4'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def write(p,d):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def origin():return dict(path=ORIGIN,pointer='/reference_gaps/0',record=read(ROOT/ORIGIN)['reference_gaps'][0])
def metadata_errors(d):
 o=origin();errors=[]
 if d.get('original_origin')!=o:errors.append('changed original failed capture')
 if d.get('status')!='historical-failed-parse-metadata;not-source-inspection':errors.append('promoted source access')
 if d.get('capture_sha256')!=o['record']['sha256'] or d.get('private_capture_path')!=o['record']['path']:errors.append('capture identity')
 if d.get('payload_redistributed') is not False or d.get('new_parse_performed') is not False:errors.append('invented payload/parse')
 if not d.get('limits'):errors.append('missing limits')
 return errors
def require_metadata(expected_origin):
 """Used by the maintained81-row producer; no private payload is required."""
 d=read(ROOT/REPORT);assert expected_origin==origin();assert not metadata_errors(d)
 # If a local retained capture is available it must still be the original bytes.
 p=ROOT/d['private_capture_path']
 if p.is_file():assert sha(p)==d['capture_sha256']
 return REPORT
def expected_row():
 old=read(BASE/'original_row.json');row=copy.deepcopy(old)
 assert row['id']==IDENT and row['original_origins']==[origin()]
 row['dimensions']['author_validation']['evidence']=[REPORT]
 row['artifacts']=sorted({REPORT,ORIGIN})
 return row
def check(packet):
 errors=metadata_errors(read(ROOT/REPORT))
 if packet.get('updates')!=[expected_row()]:errors.append('exact source-only successor')
 for n,h in packet.get('source_sha256',{}).items():
  if not (ROOT/n).is_file() or sha(ROOT/n)!=h:errors.append('stale metadata '+n)
 if not packet.get('source_sha256'):errors.append('missing pins')
 if any(n.startswith('build/') for n in packet.get('source_sha256',{})):errors.append('private payload dependency')
 for row in packet.get('updates',[]):
  if any(e.startswith('build/') for dim in row['dimensions'].values() for e in dim['evidence']):errors.append('undistributable dimension')
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');p.add_argument('--report');a=p.parse_args()
 if a.write:
  old=next(r for r in read(ROOT/'research/v5/expansion/inventory_alias_followthrough/support.json')['updates'] if r['id']==IDENT)
  prior=BASE/'original_row.json'
  if prior.exists():assert read(prior)==old or old==expected_row()
  else:write(prior,old)
  o=origin();capture=ROOT/o['record']['path'];assert capture.is_file() and sha(capture)==o['record']['sha256']
  write(ROOT/REPORT,dict(schema_version=1,status='historical-failed-parse-metadata;not-source-inspection',original_origin=o,private_capture_path=o['record']['path'],capture_sha256=o['record']['sha256'],retained_capture_bytes=capture.stat().st_size,payload_redistributed=False,new_parse_performed=False,observation='The original gate records PyMuPDF1.28.2 zero pages/corrupt object stream128. This successor verifies identity and records that existing failed attempt; it does not repeat parsing.',limits=['No source passage was inspected from this capture.','Original local bytes and historical reports remain unchanged; distributed metadata preserves exact digest/status without redistributing an uninspected PDF.','A future different capture or published document requires separate access, inspection and attribution.']))
  assert not metadata_errors(read(ROOT/REPORT))
  files=[pathlib.Path(__file__),ROOT/REPORT,ROOT/ORIGIN,prior,ROOT/'tools/build_inventory_aliases.py']
  validation=ROOT/'reviews/v5/expansion/source_capture_metadata/metadata_validation.json'
  write(validation,dict(passed=True,status='passed',scope='Original failed capture identity and successor metadata only; no new parsing/source inspection.',source_sha256={q.relative_to(ROOT).as_posix():sha(q) for q in files},checks=['original origin and digest unchanged','local retained bytes digest agrees','no payload redistribution','failure status remains not-inspected','active dimensions reference distributable metadata']))
  files.append(validation)
  write(BASE/'completion_dispositions.json',dict(schema_version=1,scope='One versioned failed-capture metadata successor; original identity/status retained.',source_sha256={q.relative_to(ROOT).as_posix():sha(q) for q in files},current_reports=[validation.relative_to(ROOT).as_posix()],updates=[expected_row()],evidence_limits=['Metadata validation is not successful document parsing or source inspection.','Maintained alias producer uses the same report; no post-regeneration manual patch required.']))
 data=read(BASE/'completion_dispositions.json');errors=check(data);controls=[];positive=[]
 if a.self_test:
  for name,fn in [('missing-row',lambda d:d['updates'].clear()),('promoted-inspection',lambda d:d['updates'][0]['dimensions']['teaching'].update(state='complete-source-inspected')),('payload-evidence',lambda d:d['updates'][0]['dimensions']['author_validation'].update(evidence=['build/portability-sources/n3245.pdf'])),('changed-origin',lambda d:d['updates'][0]['original_origins'][0]['record'].update(status='read')),('stale-source',lambda d:d['source_sha256'].update({REPORT:'0'*64}))]:
   d=copy.deepcopy(data);fn(d);controls.append(dict(control=name,rejected=bool(check(d))))
  for name,fn in [('invented-parse',lambda d:d.update(new_parse_performed=True)),('redistributed',lambda d:d.update(payload_redistributed=True)),('changed-capture',lambda d:d.update(capture_sha256='0'*64))]:
   d=read(ROOT/REPORT);fn(d);controls.append(dict(control=name,rejected=bool(metadata_errors(d))))
  # Exercise the maintained producer on the exact source-only intake. The80
  # unrelated mechanism rows need no regeneration to prove this branch.
  import build_inventory_aliases as producer
  intake=read(ROOT/'research/v5/expansion/inventory_alias_followthrough/inspection_input.json');one=dict(intake,records=[r for r in intake['records'] if r['id']==IDENT])
  rows,bindings,sources,providers=producer.make_rows(one,read(ROOT/'research/v5/expansion/inventory_alias_followthrough/clause_map.json'))
  positive.append(dict(control='maintained-producer-exact-successor',passed=rows==[expected_row()] and not any(s.startswith('build/') for s in sources)))
  # A source archive intentionally lacks private captures. Verify that exact
  # metadata is sufficient there, while any present changed bytes still fail.
  original_root=ROOT;expected_origin=origin()
  with tempfile.TemporaryDirectory(prefix='capture metadata export ') as tmp:
   exported=pathlib.Path(tmp)
   for name in [ORIGIN,REPORT]:
    p=exported/name;p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes((ROOT/name).read_bytes())
   globals()['ROOT']=exported
   try:
    positive.append(dict(control='archive-without-private-payload',passed=require_metadata(expected_origin)==REPORT and not (exported/expected_origin['record']['path']).exists()))
    p=exported/expected_origin['record']['path'];p.parent.mkdir(parents=True,exist_ok=True);p.write_bytes(b'changed capture')
    try:require_metadata(expected_origin);rejected=False
    except AssertionError:rejected=True
    controls.append(dict(control='present-wrong-private-payload',rejected=rejected))
   finally:globals()['ROOT']=original_root
  errors+=['vacuous '+r['control'] for r in controls if not r['rejected']]
  errors+=['failed '+r['control'] for r in positive if not r['passed']]
 result=dict(passed=not errors,updates=1,negative_controls=controls,positive_controls=positive,errors=errors,scope='Metadata successor and negative controls; historical failed parse remains failed.')
 if a.report:write(ROOT/a.report,result)
 print(json.dumps(result,indent=2));raise SystemExit(bool(errors))
if __name__=='__main__':main()
