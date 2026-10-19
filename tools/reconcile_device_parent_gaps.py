"""Refine LRUN23/24 using exact guarded producers; preserve physical gaps."""
import argparse,copy,functools,hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_device_parent_reconciliation'
ORIGINAL='research/v5/expansion/legacy_runtime/gap_ledger.json'
NAMES={'LRUN-23':['advanced_dma','advanced_layers','advanced_lease'],'LRUN-24':['advanced_diagnostic','diagnostic_ring','ethernet_phy','ethernet_ring','serial_can','serial_mcp']}
WHY={'LRUN-23':'Actual selected F439/H745 DMA reachability, generation, drain/lease retention, cache-line/peer models and concrete service adapters now have guarded ordinary and changed-condition implementations. Their target images and modeled effects do not establish physical device completion or cache-coherent transfers.','LRUN-24':'Actual original DG codec/frame adapter is now composed with the actual T39 EnRing/EpPhy providers. Separate original CAN codec/error/controller/transceiver models and actual Linux dummy_hcd gadget/usbfs endpoint/cancellation experiments remain distinct routes; no physical bus equivalence or physical Ethernet observation is inferred.'}
LIMITS={'LRUN-23':['Physical selected-board DMA request/drain timing, cache ownership/maintenance and peer observations remain required.','The service-layer and controller execution records retain their explicitly modeled, source-audited or actual-guest evidence classes.'],'LRUN-24':['Physical PHY/RMII clock, cable/frame captures, CAN arbitration/transceiver/ACK/bus-off behavior and USB electrical/board routing remain required.','The diagnostic format is raw length-delimited Ethernet; the separate UDP host route is not silently substituted for its actual EnRing composition.','Linux dummy_hcd is actual kernel/gadget execution in an isolated guest, not a physical USB controller or wire.']}
@functools.lru_cache(None)
def content(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return content(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(n):return json.loads(data(ROOT/n).decode('utf8'))
def package(name):return 'research/v5/expansion/legacy_'+name+'/completion_dispositions.json'
def expected():
 files={ORIGINAL,pathlib.Path(__file__).relative_to(ROOT).as_posix()};reports=[];rows=[]
 for ident,names in NAMES.items():
  parent=next(x for x in read(ORIGINAL)['gaps'] if x['id']==ident);children={};paths=[];childreports=[]
  for name in names:
   path=package(name);d=read(path);files.add(path);files.update(d['source_sha256']);paths.append(path);reports.extend(d['current_reports']);childreports.extend(d['current_reports'])
   for row in d['updates']:children[row['id']]=row
  if ident=='LRUN-23':
   required=set()
   for project in ['PR-T28-06','PR-T29-04']:
    path='curriculum/v5/expansion/legacy_assessments/'+project+'.json';files.add(path)
    for c in read(path)['criteria']:
     if c['objective'].rsplit('.',1)[0] in parent['topics']:required.add(c['id']);required.update(v['id'] for v in c['variants'])
   assert set(children)==required and len(required)==24
  else:
   assert len(read(package('advanced_diagnostic'))['updates'])==6 and len(read(package('diagnostic_ring'))['updates'])==2
   assert any(r.get('objective','').startswith('T38.11.') for r in children.values()) and any(r.get('objective','').startswith('T39.08.') for r in children.values())
  assert all(r['dimensions']['implementation']['state']=='complete' and r['dimensions']['author_validation']['state']=='passed' for r in children.values())
  childreports=list(dict.fromkeys(childreports));dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete-for-declared-author-scope',paths),('implementation','complete-for-declared-author-scope',paths),('author_validation','passed-with-child-scope-limitations',childreports),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
  rows.append(dict(id=ident,original_source=ORIGINAL,original_record=parent,child_ids=sorted(children),child_packages=paths,dimensions=dims,artifacts=paths,closure_rationale=WHY[ident],remaining_author_work=[],remaining_physical_work=LIMITS[ident],evidence_limits=['Aggregation supplies no new experiment, source passage inspection, independent review or learner success.','Original parent text, criteria, variants and historical receipts remain unchanged.','Exact published ISO editions and other source-access gaps retain their separate identities.']+LIMITS[ident]))
 return dict(schema_version=1,scope='Existing LRUN23/24 author-dimension aggregation; physical observations explicitly outstanding',source_sha256={n:sha(ROOT/n) for n in sorted(files)},current_reports=list(dict.fromkeys(reports)),updates=rows)
def validate(d):
 e=[]
 if d!=expected():e.append('exact original parents/children/remaining dimensions differ')
 for name in {n for names in NAMES.values() for n in names}:
  child=read(package(name))
  for n,h in child['source_sha256'].items():
   if not (ROOT/n).is_file() or sha(ROOT/n)!=h:e.append('stale child '+n)
  for report in child['current_reports']:
   r=read(report)
   if not(r.get('passed') is True or r.get('status')=='pass'):e.append('failed child report '+report)
   for q in r.get('commands',r.get('checks',[])):
    # This one retained size-analysis schema records raw tool exits, not a
    # per-command passed flag. It is not interchangeable with test receipts.
    cost=(report=='reviews/v5/expansion/legacy_advanced_diagnostic/cost_analysis.json' and q.get('returncode')==0 and isinstance(q.get('stdout'),str) and bool(q.get('command')))
    if q.get('passed') is not True and not cost:e.append('failed observation '+report)
   for key in ['source_sha256','input_sha256','outputs_sha256']:
    for n,h in r.get(key,{}).items():
     if not (ROOT/n).is_file() or sha(ROOT/n)!=h:e.append('stale observation '+n)
 return e
def main():
 p=argparse.ArgumentParser();p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');p.add_argument('--report',type=pathlib.Path);a=p.parse_args();HERE.mkdir(parents=True,exist_ok=True);out=HERE/'completion_dispositions.json'
 if a.write:out.write_text(json.dumps(expected(),indent=2)+'\n',encoding='utf8')
 d=read(out.relative_to(ROOT).as_posix());errors=validate(d);controls=[]
 if a.self_test and not errors:
  for k in ['omit-parent','omit-child','omit-ring','omit-CAN','omit-USB','rewrite-original','stale-input','inflate-review','inflate-learner','inflate-physical','erase-physical-limits']:
   bad=copy.deepcopy(d)
   if k=='omit-parent':bad['updates'].pop()
   elif k=='omit-child':bad['updates'][0]['child_ids'].pop()
   elif k in ['omit-ring','omit-CAN','omit-USB']:bad['updates'][1]['child_packages'].remove(package({'omit-ring':'diagnostic_ring','omit-CAN':'serial_can','omit-USB':'advanced_diagnostic'}[k]))
   elif k=='rewrite-original':bad['updates'][0]['original_record']['status']='complete'
   elif k=='stale-input':bad['source_sha256'][ORIGINAL]='0'*64
   elif k.startswith('inflate-'):bad['updates'][0]['dimensions'][{'inflate-review':'external_review','inflate-learner':'independent_learner','inflate-physical':'physical_observation'}[k]]['state']='passed'
   else:bad['updates'][0]['remaining_physical_work']=[]
   rejected=bool(validate(bad));controls.append(dict(control=k,rejected=rejected))
   if not rejected:errors.append(k+' accepted')
 result=dict(passed=not errors,updates=len(d['updates']),negative_controls=controls,errors=errors,
             source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in [out,pathlib.Path(__file__)]})
 if a.report:a.report.parent.mkdir(parents=True,exist_ok=True);a.report.write_text(json.dumps(result,indent=2)+'\n',encoding='utf8')
 print(json.dumps(result,indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
