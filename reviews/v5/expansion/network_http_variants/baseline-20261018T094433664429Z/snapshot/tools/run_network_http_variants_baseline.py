"""Rebind exact original framing, interest and real descriptor-reuse anchors."""
from datetime import datetime,timezone
from pathlib import Path
import json,os,shlex,shutil,subprocess,sys
from common import ROOT,dump,sha
BASE='research/v5/expansion/network_http_variants'
def main():
    files=['tools/run_network_http_variants_baseline.py','tools/common.py','labs/b04_depth/tests/tests.c','labs/b04_depth/include/b04.h','assessor/b04_depth/depth.c','labs/http_depth/tests/tests.c','labs/http_depth/include/http_depth.h','labs/http_depth/tests/socket_adapter.c','assessor/http_depth/http_depth.c','labs/hosted_followthrough/http/models.py']
    before={p:sha(ROOT/p) for p in files};home=ROOT/'reviews/v5/expansion/network_http_variants'/('baseline-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));home.mkdir(parents=True);executions=[];external={};deps=[];profiles=[]
    for rel in before:
        out=home/'snapshot'/rel;out.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(ROOT/rel,out)
    report=dict(passed=False,input_sha256=before,executions=executions,external_input_sha256=external,compiler_dependencies=deps,profiles=profiles,physical='not-run',independent_assessment='not-run')
    def run(cmd,label,expected=0,marker=None):
        cmd=list(map(str,cmd));dep=None
        if cmd[0] in ['gcc','clang'] and '-c' in cmd:dep=home/(label+'.d');cmd+=['-MD','-MF',str(dep)]
        r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90,env={**os.environ,'ASAN_OPTIONS':'detect_leaks=1:halt_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'});text=r.stdout+'\n'+r.stderr;log=home/(label+'.log');log.write_text(text,encoding='utf8');executions.append(dict(command=cmd,exit_code=r.returncode,expected_exit=expected,required_marker=marker,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert r.returncode==expected and (not marker or marker in text),text
        if expected:assert 'AddressSanitizer' not in text and 'runtime error:' not in text
        if dep:
            paths=[Path(v).resolve() for v in shlex.split(dep.read_text().replace('\\\n',' ').split(':',1)[1])];deps.append(dict(execution=len(executions)-1,paths=list(map(str,paths))))
            for p in paths:
                if p.is_relative_to(ROOT):assert p.relative_to(ROOT).as_posix() in before
                else:external[str(p)]=sha(p)
        return r.stdout
    def build(cc,flags,label,sources,defines=()):
        objects=[]
        for i,s in enumerate(sources):
            obj=home/(label+str(i)+'.o');run([cc,*[f for f in flags if f!='-no-pie'],'-Ilabs/b04_depth/include','-Ilabs/http_depth/include',*defines,'-c',ROOT/s,'-o',obj],label+'compile'+str(i));objects.append(obj)
        exe=home/label;run([cc,*flags,*objects,'-o',exe],label+'link');return exe
    try:
        for cc in ['gcc','clang']:
            path=Path(shutil.which(cc)).resolve();external[str(path)]=sha(path);run([cc,'--version'],cc+'version')
            for name,extra in [('debug',['-O0','-g']),('release',['-O3']),('asan',['-O1','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie'])]:
                label=cc+'-'+name;flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror',*extra]
                framed=build(cc,flags,label+'-frame',['labs/b04_depth/tests/tests.c','assessor/b04_depth/depth.c']);run([framed,'framing'],label+'-frame-run')
                wrong=build(cc,flags,label+'-frame-mutant',['labs/b04_depth/tests/tests.c','assessor/b04_depth/depth.c'],['-DB4_MUTATE_FRAMING']);run([wrong,'framing'],label+'-frame-kill',1,'DEPTH_CHECK:')
                core=build(cc,flags,label+'-core',['labs/http_depth/tests/tests.c','assessor/http_depth/http_depth.c'])
                for case in ['length','date','lifecycle']:run([core,case],label+'-'+case,marker='PASS HTTP depth')
                for macro in ['HD_MUTATE_LIFECYCLE','HD_MUTATE_LIFECYCLE2']:
                    wrong=build(cc,flags,label+'-'+macro,['labs/http_depth/tests/tests.c','assessor/http_depth/http_depth.c'],['-D'+macro]);run([wrong,'lifecycle'],label+'-'+macro+'-kill',1,'HD_CHECK:')
                sock=build(cc,flags,label+'-socket',['labs/http_depth/tests/socket_adapter.c','assessor/http_depth/http_depth.c']);observation=run([sock],label+'-socket-run',marker='local socketpair: explicit flags, controlled fd reuse, stale event rejected, owned descriptors closed')
                profiles.append(dict(profile=label,framing='original CL6/zero-chunk5 and unchanged conflict output',anchors=['length','date','lifecycle'],socket=observation,controls=['B4_MUTATE_FRAMING','HD_MUTATE_LIFECYCLE','HD_MUTATE_LIFECYCLE2']));print(label+' baseline passed',flush=True)
        report['models']=run([sys.executable,ROOT/'labs/hosted_followthrough/http/models.py'],'local-models',marker='HTTP_MODELS_PASS:')
        assert all(sha(ROOT/p)==h for p,h in before.items());report.update(passed=True,source_stable=True)
    except Exception as error:report['failure']=repr(error)
    report['artifact_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in home.rglob('*') if p.is_file()};dump(home/'report.json',report);dump(ROOT/BASE/'baseline_validation.json',report);print(dict(passed=report['passed'],commands=len(executions),failure=report.get('failure')));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
