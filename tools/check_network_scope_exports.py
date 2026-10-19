"""Execute the actual assessment export without implicit assessor fallback."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
from run_network_scope_depth import PROVIDERS,target_args,emulator
ROOT=Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_network_scope';REF=ROOT/'assessor/network_followthrough/scope'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    base=ROOT/'build/network-scope-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');rows=[];errors=[];pins={Path(__file__),ROOT/'tools/run_network_scope_depth.py',ROOT/'cmastery/cli.py',ROOT/'cmastery/assessments.py',*[ROOT/p for p in PROVIDERS.values()]}
    def run(label,cmd,expected=0,marker=None):
        cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);ok=r.returncode==expected and (not marker or marker in r.stdout+r.stderr);rows.append(dict(label=label,command=cmd,returncode=r.returncode,expected=expected,marker=marker,stdout=r.stdout,stderr=r.stderr,passed=bool(ok)));return ok
    project='PR-T41-01';dest=base/project;attachment=HERE/('attachment_'+project+'.json');a=json.loads(attachment.read_text());source=ROOT/('curriculum/v5/expansion/legacy_assessments/'+project+'.json');pins|={attachment,source};pins.update(ROOT/p for p in a['supplied_artifacts'])
    if not run('actual init',[sys.executable,'-m','cmastery','init',project,dest]):errors.append('init failed')
    else:
        for p in a['supplied_artifacts']:
            if not (dest/p).is_file() or sha(dest/p)!=sha(ROOT/p):errors.append('missing/changed public '+p)
        private={sha(p) for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts}
        if any('assessor' in p.relative_to(dest).parts or sha(p) in private for p in dest.rglob('*') if p.is_file()):errors.append('assessor leakage')
        if json.loads(source.read_text())['criteria']!=json.loads((dest/'ASSESSMENT.json').read_text())['criteria']:errors.append('criteria changed')
        lab=dest/'labs/network_followthrough/scope'
        for reference in [False,True]:
            cmd=[sys.executable,lab/'run.py','--build',dest/('native-'+str(reference))]
            if reference:cmd+=['--source',REF/'scope.c']
            run('native '+str(reference),cmd,0 if reference else 1,'NW_PASS' if reference else 'NW_CHECK')
            target=dest/('target-'+str(reference));cmd=target_args(target,'arm-none-eabi-gcc','0',stub=not reference);cmd[1]=lab/'target_build.py'
            if not reference:cmd[cmd.index('--source')+1]=lab/'scope.c'
            ok=run('target build '+str(reference),cmd,marker='NW_TARGET_BUILD_PASS explicit_objects=12 original_runtime')
            if ok:run('target execute '+str(reference),emulator(target/'image.elf'),0 if reference else 1,'NW_TARGET_PASS' if reference else 'NW_TARGET_CHECK')
        includes=['-I'+str(lab),'-I'+str(dest/'labs/communications/include')];providers=[ROOT/PROVIDERS[k] for k in ['http','net']]
        for variant in ['pair','stream','wire']:
            exe=dest/('stub-'+variant)
            sources=[lab/'pair.c',REF/'pair_test.c'] if variant=='pair' else [REF/'scope.c',(lab if variant=='stream' else REF)/'stream.c',(lab if variant=='wire' else REF)/'wire.c',REF/'integration.c']
            ok=run(variant+' stub compile',['gcc','-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror',*includes,*sources,*providers,'-o',exe])
            if ok:run(variant+' stub execute',[exe],1,'NW_PAIR' if variant=='pair' else 'NW_INTEGRATE')
    pins.update(p for p in REF.rglob('*') if p.is_file() and '__pycache__' not in p.parts)
    from envelope_export_inputs import bind_project_exports
    bind_project_exports(ROOT,pins,rows,errors)
    d=dict(passed=not errors and len(rows)==13 and all(r['passed'] for r in rows),errors=errors,commands=rows,input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(pins)},limits=['Actual exported native/target tools and all four compiling incomplete sources; author providers explicitly selected outside learner tree.','No implicit private fallback; complete reference imports remain author execution, not learner integration.','No independent learner, physical observation or external review.'])
    (ROOT/'reviews/v5/expansion/legacy_network_scope/exports.json').write_text(json.dumps(d,indent=2)+'\n');print(dict(passed=d['passed'],commands=len(rows),errors=errors,failed=[r['label'] for r in rows if not r['passed']]));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
