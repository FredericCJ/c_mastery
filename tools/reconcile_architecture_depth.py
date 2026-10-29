"""Versioned aggregation of ten original T37 depth records, never new evidence."""
import argparse,copy,functools,hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_architecture_reconciliation';ORIGIN='research/v5/expansion/b045_legacy/depth_dispositions.json'
SPECS=[('IW-62b31ef93b0a4f88',10,'commit'),('IW-1e33e491aaa67e97',11,'alu'),('IW-4dfb467dcc1f7a43',12,'bytes'),('IW-2122cfa8a833aaad',13,'bytes'),('IW-bb8159568aac3df1',14,'commit'),('IW-050f0d4a210301e9',15,'pipeline'),('IW-0d7464f4ffc9f133',16,'cache'),('IW-8837152ccb5d7ff9',17,'bus'),('IW-2233582b62fe7c85',18,'frame'),('IW-d07e2af614bc5578',19,'performance')]
WHY=['Actual x86 SUB/ZF and MPS2 SUB/Z observations are paired with the unchanged original toy transition model; physical clocks remain a distinct evidence class.','Original C add/sub, Z/N and operation admission plus changed wire dependency controls now exist and execute.','Typed record/object extent adapter and actual selected x86/Thumb instruction observations are linked.','Original bounded C decoder is checked against actual selected assembler/disassembler bytes, not only a lookup fixture.','Multi-tick load stall/cancel, ordered external-input injection and deferred speculative effects are executable bounded models; arbitrary asynchronous hardware remains outside their stated scope.','Alternate load/branch/register-file/port configurations and renamed out-of-order scheduling execute; native commercial pipeline equivalence is deliberately unclaimed, not inferred from certificates.','Layout/cache, dirty ownership and exact dimensional performance work is authored and executed; independent learner evidence remains open.','Exact RM0090 Rev21 passages, actual B06 linker/map, typed-buffer ledger and unchanged struct pd_transfer ownership are composed with stricter selected-bank admission.','Actual frozen reset/vector/frame/priority producers execute in MPS2 including basic/extended/lazy-FP cases; F439 boot/physical latency remains separate.','Actual equivalent-contract checked C/assembly kernels measured31 samples per condition, all1488 raw samples retained across six compiler profiles; no speedup or physical Cortex timing claim.']
@functools.lru_cache(None)
def content(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return content(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def write(p,d):p.write_text(json.dumps(d,indent=2)+'\n',encoding='utf8')
def children():return {name:'research/v5/expansion/legacy_architecture_'+name+'/completion_dispositions.json' for _,_,name in SPECS}
def rows():
 original=read(ROOT/ORIGIN)['subtopics'];out=[]
 for (ident,index,name),why in zip(SPECS,WHY):
  source=original[index];path=children()[name];child=read(ROOT/path);selected=[r for r in child['updates'] if r.get('objective','').startswith(source['topic']+'.')];assert selected,(name,source)
  artifacts=list(dict.fromkeys([path,*[p for r in selected for p in r['artifacts']]]));reports=child['current_reports'];dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts),('implementation','complete',artifacts),('author_validation','passed',reports),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
  out.append(dict(id=ident,original_origin=dict(path=ORIGIN,pointer='/subtopics/'+str(index)),original_record=source,dimensions=dims,artifacts=artifacts,author_validation=dict(reports=reports,criterion_ids=[r['id'] for r in selected]),closure_rationale=why,remaining_author_work=[],evidence_limits=['Aggregation adds no experiment or independent review.','Original record and historical receipts preserved; linked scoped results retain their limits.','Physical and independent learner dimensions remain open.']))
 return out
def expected():
 files={ORIGIN,pathlib.Path(__file__).relative_to(ROOT).as_posix()};reports=[]
 for path in children().values():
  child=read(ROOT/path);files.add(path);files.update(child['source_sha256']);reports.extend(child['current_reports'])
 return dict(schema_version=1,scope='Ten existing T37 depth-disposition IDs; successor aggregation only',source_sha256={n:sha(ROOT/n) for n in sorted(files)},current_reports=list(dict.fromkeys(reports)),updates=rows(),limits=['No new experiment; no global inventory rewrite.','All original project/criterion/delay identities remain bound by their child packages.','No absolute mastery, physical behavior, independent review or learner success claim.'])
def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 e=expected();need(d==e,'exact original IDs, requirements, child criterion bindings and six dimensions')
 for path in children().values():
  child=read(ROOT/path)
  for n,h in child['source_sha256'].items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale child source '+n)
  for path in child['current_reports']:
   report=read(ROOT/path);need(report.get('status')=='pass' and report.get('commands') and all(c.get('passed') is True for c in report['commands']),'actual passing child report '+path)
   for key in ['source_sha256','outputs_sha256']:
    for n,h in report.get(key,{}).items():need((ROOT/n).is_file() and sha(ROOT/n)==h,'stale child report '+n)
 return errors
def main():
 p=argparse.ArgumentParser();p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');a=p.parse_args();HERE.mkdir(parents=True,exist_ok=True);path=HERE/'completion_dispositions.json'
 if a.write:write(path,expected())
 d=read(path);errors=validate(d);controls=[]
 if a.self_test and not errors:
  for key in ['omit-origin','rewrite-origin','wrong-child','inflate-review','inflate-learner','inflate-physical','stale-pin','erase-limit']:
   bad=copy.deepcopy(d)
   if key=='omit-origin':bad['updates'].pop()
   elif key=='rewrite-origin':bad['updates'][0]['original_record']['remaining']='easier task'
   elif key=='wrong-child':bad['updates'][0]['author_validation']['criterion_ids']=[]
   elif key.startswith('inflate-'):bad['updates'][0]['dimensions'][{'inflate-review':'external_review','inflate-learner':'independent_learner','inflate-physical':'physical_observation'}[key]]['state']='passed'
   elif key=='stale-pin':bad['source_sha256'][ORIGIN]='0'*64
   else:bad['updates'][0]['evidence_limits']=[]
   rejected=bad!=expected();controls.append(dict(control=key,rejected=rejected))
   if not rejected:errors.append(key+' accepted')
 print(json.dumps(dict(passed=not errors,updates=len(d['updates']),negative_controls=controls,errors=errors),indent=2));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
