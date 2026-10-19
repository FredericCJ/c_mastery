"""Observe availability of a second hosted POSIX data model without installing tools."""
from datetime import datetime,timezone
import hashlib
import json
import os
from pathlib import Path
import subprocess
import sys
import tempfile
ROOT=Path(__file__).resolve().parents[1]
SOURCE='''#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
int main(void){pthread_mutex_t m;if(pthread_mutex_init(&m,0))return 1;
printf("pointer=%zu size_t=%zu uint64=%zu\\n",sizeof(void *),sizeof(size_t),sizeof(uint64_t));
return pthread_mutex_destroy(&m)!=0;}
'''
def main():
    if os.name=='nt':
        from v5_common import python_command
        return subprocess.run(python_command('tools/probe_resilience_data_model.py',platform='wsl'),cwd=ROOT).returncode
    rows=[]
    with tempfile.TemporaryDirectory(prefix='cmastery-rz-m32-',dir=Path.home()) as temp:
        work=Path(temp);source=work/'probe.c';source.write_text(SOURCE,encoding='utf8')
        for compiler in ['gcc','clang']:
            command=[compiler,'-m32','-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-pthread',str(source),'-o',str(work/compiler)]
            run=subprocess.run(command,text=True,capture_output=True,timeout=60)
            row={'compiler':compiler,'command':command,'exit_code':run.returncode,'stdout':run.stdout,'stderr':run.stderr,'status':'unavailable' if run.returncode else 'compiled'}
            if not run.returncode:
                executed=subprocess.run([str(work/compiler)],text=True,capture_output=True,timeout=10)
                row.update(execution={'exit_code':executed.returncode,'stdout':executed.stdout,'stderr':executed.stderr},status='executed' if not executed.returncode else 'runtime-unavailable')
            rows.append(row)
    path=ROOT/'research/v5/modern/resilience_data_model_probe.json'
    data={'schema_version':1,'recorded_at':datetime.now(timezone.utc).isoformat(),'producer':'tools/probe_resilience_data_model.py',
          'input_sha256':{'tools/probe_resilience_data_model.py':hashlib.sha256(Path(__file__).read_bytes()).hexdigest()},
          'generated_source':SOURCE,'generated_source_sha256':hashlib.sha256(SOURCE.encode()).hexdigest(),'observations':rows,
          'scope':'Availability observation only; even a positive tiny probe does not validate the full resilience family under another ABI.',
          'environment_changed':False,'physical_validation':'not-run','learner_mastery':'not-assessed'}
    path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf8')
    print(json.dumps({'observed':len(rows),'statuses':[r['status'] for r in rows]}));return 0
if __name__=='__main__':raise SystemExit(main())
