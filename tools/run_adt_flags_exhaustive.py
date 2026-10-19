"""Run all2**32 actual constructor inputs in separate-TU optimized executables."""
import argparse
from datetime import datetime,timezone
import json
from pathlib import Path
import shutil
import subprocess
import tempfile
import time
from common import ROOT,dump,sha
REPORT='research/v5/modern/adt_flags_exhaustive_validation.json'
def run(report=REPORT):
    source=['labs/adt/flags_negotiation/exhaustive.c','labs/adt/include/adt.h','assessor/adt/adt.c','tools/run_adt_flags_exhaustive.py','tools/common.py']
    before={s:sha(ROOT/s) for s in source};external={};records=[];observations=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('adt-flags-exhaustive-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label):
        start=time.monotonic();cmd=list(map(str,cmd))
        try:p=subprocess.run(cmd,cwd=ROOT,text=True,capture_output=True,timeout=180);code=p.returncode;out=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:code=None;out=str(e)
        log=folder/(str(len(records)).zfill(3)+'-'+label+'.log');log.write_text(out,encoding='utf8');records.append(dict(command=cmd,exit_code=code,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));return code,out
    passed=True
    with tempfile.TemporaryDirectory(prefix='cmastery-raw-flags-') as tmp:
        t=Path(tmp)
        for cc in ['gcc','clang']:
            executable=Path(shutil.which(cc)).resolve();external[str(executable)]=sha(executable);passed=execute([cc,'--version'],cc+'-version')[0]==0 and passed
            for mutation in [0,5,6]:
                objects=[]
                for name,path in [('driver',source[0]),('provider',source[2])]:
                    obj=t/(cc+'-'+str(mutation)+'-'+name+'.o');dep=obj.with_suffix('.d')
                    code,_=execute([cc,'-std=c17','-O3','-fno-lto','-Wall','-Wextra','-Wpedantic','-Werror','-I',ROOT/'labs/adt/include','-DMOD_MUTATION='+str(mutation),'-MD','-MF',dep,'-c',ROOT/path,'-o',obj],cc+'-'+str(mutation)+'-'+name)
                    assert code==0;objects.append(obj)
                    copied=folder/dep.name;shutil.copyfile(dep,copied);artifacts.append(dict(path=copied.relative_to(ROOT).as_posix(),sha256=sha(copied)))
                    for s in dep.read_text().replace('\\\n',' ').split(':',1)[1].split():
                        p=Path(s).resolve()
                        if p.is_relative_to(ROOT):
                            key=p.relative_to(ROOT).as_posix();assert key in before,'unbound repository dependency '+key
                        else:external.setdefault(str(p),sha(p))
                binary=t/(cc+'-'+str(mutation));assert execute([cc,'-fno-lto',*objects,'-o',binary],cc+'-'+str(mutation)+'-link')[0]==0
                code,out=execute([binary],cc+'-'+str(mutation)+'-execute')
                if mutation==0:
                    result=json.loads(out) if code==0 else {};ok=code==0 and result==dict(values=4294967296,accepted=8,rejected=4294967288,checksum=28,unchanged_rejections=True)
                else:result={};ok=code==1 and 'ADO_CHECK exhaustive raw='+('8' if mutation==5 else '0') in out
                observations.append(dict(compiler=cc,mutation=mutation,passed=ok,result=result));passed=passed and ok
            print(cc+' full enumeration/control outcomes recorded',flush=True)
    stable=all(sha(ROOT/p)==h for p,h in before.items());extstable=all(sha(Path(p))==h for p,h in external.items());passed=passed and stable and extstable
    result=dict(schema_version=1,passed=passed,status='passed' if passed else 'failed',source_stable=stable,input_sha256=before,external_inputs_stable=extstable,external_input_sha256=external,executions=records,observations=observations,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Actual all32-bit masks checked only for the original pure constructor on two native release builds.','Negotiation histories, arbitrary compilers and learner mastery are not proved by this enumeration.'])
    dump(ROOT/report,result);return passed
if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--report',default=REPORT);a=p.parse_args();raise SystemExit(0 if run(a.report) else 1)
