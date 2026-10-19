"""Versioned closure of existing depth records; no new observation is created."""
from pathlib import Path
import argparse,copy,functools,hashlib,json
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_interrupt_reconciliation'
SPECS=[('IW-7f84c8c013af4bcb','curriculum/v5/expansion/b03_depth_disposition.json','objectives',226,['handoff'],'T29.06.b'),('IW-b1bca23c5f546ae2','curriculum/v5/expansion/b03_depth_disposition.json','objectives',227,['handoff'],'T29.06.c'),('IW-2fc0e7af025f8fb5','curriculum/v5/expansion/b03_depth_disposition.json','objectives',235,['time'],'T29.09.b'),('LRUN-25','research/v5/expansion/legacy_runtime/gap_ledger.json','gaps',24,['frame','handoff','time'],None)]
WHY=['Actual phase-split Xi publication/release and wrap tests, bounded RC11 exploration, native/target controls and unchanged delayed counterexamples now assess the exact leaf.','Equivalent-work target timer windows, instrumentation comparisons, bounded interrupt schedules and complete physical acquisition protocol now exist; virtual timing is not physical worst-case evidence.','Actual bounded coupled-counter proof and reduced-width ABA refusal, separate SC software clock, lost-event accounting, independent arithmetic oracles and target interrupt observations now assess the exact leaf.','Frame/basic/extended/lazy-FP variants, priority/mask nesting, actual queue ownership and coherent sampling execute on available MPS2 targets; full physical capture and changed-condition analysis packages exist. Physical latency/tail chaining/device ordering remain unobserved.']
@functools.lru_cache(None)
def content(p,n,t):return p.read_bytes()
def data(p):s=p.stat();return content(p,s.st_size,s.st_mtime_ns)
def sha(p):return hashlib.sha256(data(p)).hexdigest()
def read(p):return json.loads(data(p).decode('utf8'))
def child(n):return 'research/v5/expansion/legacy_advanced_'+n+'/completion_dispositions.json'
def expected():
    files={Path(__file__).relative_to(ROOT).as_posix()};reports=[];rows=[]
    for (ident,origin,key,index,names,objective),why in zip(SPECS,WHY):
        files.add(origin);original=read(ROOT/origin)[key][index];selected=[];artifacts=[];local_reports=[]
        for name in names:
            path=child(name);d=read(ROOT/path);files.add(path);files.update(d['source_sha256']);local_reports+=d['current_reports'];artifacts.append(path)
            selected += [r for r in d['updates'] if objective is None or r['objective']==objective]
        assert selected
        artifacts=list(dict.fromkeys([*artifacts,*[p for r in selected for p in r['artifacts']]]));reports+=local_reports
        dims={k:dict(state=s,evidence=e) for k,s,e in [('teaching','complete',artifacts),('implementation','complete',artifacts),('author_validation','passed',local_reports),('external_review','not-run',[]),('independent_learner','not-assessed',[]),('physical_observation','not-run',[])]}
        rows.append(dict(id=ident,original_origin=dict(path=origin,pointer='/'+key+'/'+str(index)),original_record=original,objective=objective,dimensions=dims,artifacts=artifacts,author_validation=dict(reports=local_reports,criterion_ids=[r['id'] for r in selected]),closure_rationale=why,remaining_author_work=[],evidence_limits=['Aggregation of source-bound author packages creates no new experiment.','Physical target qualification and capture, non-author review, independent learner and delayed observations remain pending.','LRUN-23 DMA/cache targets and LRUN-24 diagnostic/Ethernet obligations remain separate; no unrelated closure inferred.']))
    return dict(schema_version=1,scope='Three existing T29 residual leaf records and original mixed LRUN-25',source_sha256={p:sha(ROOT/p) for p in sorted(files)},current_reports=list(dict.fromkeys(reports)),updates=rows,limits=['Historical source records unchanged.','Author implementation and available-target execution complete only for the exact linked scope; physical and learner evidence separate.'])
def validate(d):
    errors=[]
    if d!=expected():errors.append('original identities, scoped children or dimensions changed')
    for name in ['frame','handoff','time']:
        c=read(ROOT/child(name))
        for p,h in c['source_sha256'].items():
            if not (ROOT/p).is_file() or sha(ROOT/p)!=h:errors.append('stale child '+p)
        for p in c['current_reports']:
            r=read(ROOT/p)
            if r.get('passed') is not True:errors.append('failed report '+p)
            for key in ['input_sha256','outputs_sha256']:
                for f,h in r.get(key,{}).items():
                    if not (ROOT/f).is_file() or sha(ROOT/f)!=h:errors.append('stale report '+f)
    return errors
def main():
    p=argparse.ArgumentParser();p.add_argument('--write',action='store_true');p.add_argument('--self-test',action='store_true');p.add_argument('--report',required=True);a=p.parse_args();HERE.mkdir(parents=True,exist_ok=True);path=HERE/'completion_dispositions.json'
    if a.write:path.write_text(json.dumps(expected(),ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
    d=read(path);errors=validate(d);controls=[]
    if a.self_test and not errors:
        for kind in ['omit-origin','rewrite-origin','wrong-child','inflate-review','inflate-learner','inflate-physical','stale-pin','erase-limit']:
            x=copy.deepcopy(d)
            if kind=='omit-origin':x['updates'].pop()
            elif kind=='rewrite-origin':x['updates'][0]['original_record']['remaining']='easier task'
            elif kind=='wrong-child':x['updates'][0]['author_validation']['criterion_ids']=[]
            elif kind.startswith('inflate-'):x['updates'][0]['dimensions'][{'inflate-review':'external_review','inflate-learner':'independent_learner','inflate-physical':'physical_observation'}[kind]]['state']='passed'
            elif kind=='stale-pin':x['source_sha256'][next(iter(x['source_sha256']))]='0'*64
            else:x['updates'][0]['evidence_limits']=[]
            rejected=bool(validate(x));controls.append(dict(control=kind,rejected=rejected))
            if not rejected:errors.append('accepted '+kind)
    out=dict(passed=not errors,updates=len(d['updates']),errors=errors,negative_controls=controls,input_sha256={p:sha(ROOT/p) for p in [path.relative_to(ROOT).as_posix(),Path(__file__).relative_to(ROOT).as_posix()]});(ROOT/a.report).write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=not errors,updates=len(d['updates']),controls=len(controls),errors=errors));return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
