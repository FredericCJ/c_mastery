"""Actual selected-project exports, unsolved build and explicit assessor run."""
import hashlib,json,subprocess,sys
from datetime import datetime,timezone
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'reviews/v5/expansion/integrated/final_author_run/t43_cgroup'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    out=BASE/('exports_'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));out.mkdir(parents=True,exist_ok=False);work=out/'learner';commands=[]
    def run(name,cmd,expected):
        cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=180);log=out/(name+'.txt');log.write_text(r.stdout+r.stderr);commands.append({'label':name,'command':cmd,'returncode':r.returncode,'expected_exit_code':expected,'log':str(log.relative_to(ROOT)),'log_sha256':sha(log)});assert r.returncode==expected,(name,r.stdout[-1000:],r.stderr[-1000:]);return r.stdout+r.stderr
    run('init',[sys.executable,'-B','-m','cmastery','init','PR-T43-10',work],0)
    assert not any('assessor' in p.relative_to(work).parts for p in work.rglob('*'))
    supplied=json.loads((ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T43-10.json').read_text())['supplied_artifacts'];assert all((work/p).read_bytes()==(ROOT/p).read_bytes() for p in supplied)
    args=[sys.executable,'-B',work/'labs/performance_delegated/build.py','--cc','gcc','--run','--kernel','/home/frede/cmastery-os-follow-tools/linux-source-7.0.0/arch/x86/boot/bzImage','--busybox','/home/frede/cmastery-b06-tools/usr/bin/busybox']
    failed=run('learner',[*args,'--work',out/'learner-build'],1);assert 'PC_TODO:' in failed and 'PC_PROBE_EXIT=1' in failed
    passed=run('explicit-reference',[*args,'--work',out/'reference-build','--source',ROOT/'assessor/performance_delegated/delegated.c'],0);assert 'PC_PASS:T43.11.a-DELAYED:' in passed and 'PC_RESTORED max 100000' in passed and 'PC_CLEANUP removed=3' in passed
    paths=set(supplied)|{'curriculum/projects/PR-T43-10.md','curriculum/v5/expansion/legacy_assessments/PR-T43-10.json','cmastery/cli.py','cmastery/assessments.py','cmastery/exercises.py','cmastery/learning.py','cmastery/project_assets.py','tools/run_performance_delegated_exports.py','assessor/performance_delegated/delegated.c'}
    result={'passed':True,'commands':commands,'project':'PR-T43-10','selected_assessment_sha256':sha(ROOT/'curriculum/v5/expansion/legacy_assessments/PR-T43-10.json'),'source_sha256':{p:sha(ROOT/p) for p in sorted(paths)},'exported_files':{str(p.relative_to(work)):sha(p) for p in work.rglob('*') if p.is_file()},'workspace':str(work.relative_to(ROOT)),'limits':['No private source exported. Explicit reference source supplied from assessor only for author run.','No learner completion or physical PMU claim.']};dest=out/'exports.json';dest.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':True,'commands':len(commands),'report':str(dest.relative_to(ROOT))}))
if __name__=='__main__':main()
