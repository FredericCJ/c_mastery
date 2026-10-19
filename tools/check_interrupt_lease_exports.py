"""Actual PR-T29-04 CLI export, public stub rejection and explicit reference imports."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_advanced_lease';REF=ROOT/'assessor/interrupt_depth/lease';DMA=ROOT/'assessor/peripheral_depth/dma/dma.c';CACHE=ROOT/'assessor/firmware_lifecycle/targets/h745_cache.c';HELP=ROOT/'assessor/execution/support.c'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    base=ROOT/'build/interrupt-lease-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[];pins={Path(__file__),ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',DMA,CACHE,HELP,ROOT/'assessor/peripheral_depth/dma/target.c'}
    def run(label,cmd,expected=0,marker=None):
        cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=120);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=bool(ok)));return ok
    project='PR-T29-04';dest=base/project;attachment=HERE/('attachment_'+project+'.json');a=json.loads(attachment.read_text());source=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins|={attachment,source};pins.update(ROOT/p for p in a['supplied_artifacts'])
    if not run('actual init',[sys.executable,'-m','cmastery','init',project,dest]):errors.append('init failed')
    else:
        for p in a['supplied_artifacts']:
            if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
        private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
        if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
        if json.loads(source.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('criteria changed')
        lab=dest/'labs/interrupt_depth/lease'
        for reference in [False,True]:
            cmd=[sys.executable,lab/'run.py','--build',dest/('native-'+str(reference))]
            if reference:cmd+=['--source',REF/'lease.c']
            run('native '+str(reference),cmd,0 if reference else 1,'XD_PASS' if reference else 'XD_CHECK:')
            target=dest/('target-'+str(reference));cmd=[sys.executable,lab/'target_build.py','--build',target,'--runtime',HELP,'--dma',DMA,'--cache',CACHE,'--composition',REF/'composition.c']
            if reference:cmd+=['--source',REF/'lease.c','--bridge',REF/'bridge.c','--order',REF/'order.c','--driver',REF/'target.c']
            ok=run('target build '+str(reference),cmd,marker='XD_TARGET_BUILD_PASS objects=9')
            if ok:run('target execute '+str(reference),['qemu-system-arm','-M','mps2-an386','-display','none','-monitor','none','-serial','none','-semihosting-config','enable=on,target=native','-icount','shift=0,align=off,sleep=off','-kernel',target/'image.elf'],0 if reference else 1,'XD_TARGET_PASS' if reference else 'XD_TARGET_UNIMPLEMENTED')
        for board in ['F439','H745']:
            run('exported '+board+' port build',[sys.executable,lab/'board_build.py','--build',dest/board,'--board',board,'--port',ROOT/'assessor/peripheral_depth/dma/target.c','--dma',DMA,'--cache',CACHE,'--runtime',HELP],marker='XD_BOARD_BUILD_PASS '+board)
        run('exported capture checker',[sys.executable,dest/'tools/check_interrupt_lease_capture.py','--self-test'],marker='"physical_claim": false')
    pins.update(p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
    from envelope_export_inputs import bind_project_exports
    bind_project_exports(ROOT,pins,rows,errors)
    d=dict(passed=not errors and len(rows)==10 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Reference implementations explicitly selected outside learner tree; no assessor leakage or automatic fallback.','Actual exported native/target/port tools; model execution is separate from physical target behavior.','No independent learner or physical evidence.'])
    (ROOT/'reviews/v5/expansion/legacy_advanced_lease/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
