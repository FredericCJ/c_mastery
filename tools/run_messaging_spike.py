"""Execute pre-prose concurrency feasibility experiments, retaining exact logs."""
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
from common import ROOT, dump, sha
from v5_common import python_command

def main():
    if os.name == 'nt':
        return subprocess.run(python_command('tools/run_messaging_spike.py', platform='wsl'), cwd=ROOT).returncode
    dest=ROOT/'research/v5/modern/messaging_spike'
    dest.mkdir(parents=True,exist_ok=True)
    build=ROOT/'build/messaging-spike';build.mkdir(parents=True,exist_ok=True)
    inputs=['labs/messaging/include/messaging.h','labs/messaging/tests/tests.c','assessor/messaging/messaging.c','tools/run_messaging_spike.py']
    pins={p:sha(ROOT/p) for p in inputs};rows=[]
    def run(cmd,name):
        try:
            p=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=90)
            code=p.returncode;out=p.stdout+'\n'+p.stderr
        except subprocess.TimeoutExpired as e:
            code=None;out=str(e)
        log=dest/(name+'.log');log.write_text(out,encoding='utf-8',newline='\n')
        rows.append(dict(command=[str(x) for x in cmd],exit_code=code,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
        return code==0
    passed=True
    for mode,flags in [('plain',['-O2']),('asan',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie']),('tsan',['-O1','-g','-fsanitize=thread','-fno-omit-frame-pointer'])]:
        exe=build/mode
        ok=run(['gcc','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-pthread','-Ilabs/messaging/include',*flags,'labs/messaging/tests/tests.c','assessor/messaging/messaging.c','-o',str(exe)],mode+'-compile')
        if ok:ok=run([str(exe),'threads'],mode+'-threads')
        if ok:
            for case in ('nursery','channels','operations','actors','supervision','stealing','transactions','snapshots','sequences','streams'):
                ok=run([str(exe),case],mode+'-'+case) and ok
        passed=passed and ok
    dump(dest/'report.json',dict(status='passed' if passed else 'failed',passed=passed,input_sha256=pins,executions=rows,
        scope='Original fixed-capacity, sequentially-consistent C17 algorithms and pthread adapters. No weak-order proof, benchmark, learner assessment or physical validation.',
        earlier_observations=['Initial GCC -Werror compile rejected misleading indentation. clang-format made statement boundaries explicit; warnings were not disabled.']))
    print(json.dumps({'passed':passed,'commands':len(rows)}));return 0 if passed else 1
if __name__=='__main__':raise SystemExit(main())
