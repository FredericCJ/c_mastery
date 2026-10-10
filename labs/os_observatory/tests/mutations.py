"""Reject two mechanism defects through the ordinary public-interface tests."""
import argparse
from pathlib import Path
import subprocess
import tempfile

def main():
    p=argparse.ArgumentParser();p.add_argument('--compiler',required=True);p.add_argument('--impl',required=True);p.add_argument('--lab',required=True)
    args=p.parse_args(); lab=Path(args.lab); implementation=Path(args.impl)
    wrappers={
        'rr_becomes_fcfs':('schedulers','''
#include "oslab.h"
int os_real_simulate(const os_job*,size_t,os_policy,unsigned,os_schedule*);
size_t os_real_clock(const uint8_t*,size_t,size_t);
int os_simulate(const os_job*j,size_t n,os_policy p,unsigned q,os_schedule*s) {
 return os_real_simulate(j,n,p==OS_RR?OS_FCFS:p,q,s);
}
size_t os_clock_faults(const uint8_t*r,size_t n,size_t f) { return os_real_clock(r,n,f); }
'''),
        'clock_forgets_reference_bits':('clock','''
#include "oslab.h"
int os_real_simulate(const os_job*,size_t,os_policy,unsigned,os_schedule*);
int os_simulate(const os_job*j,size_t n,os_policy p,unsigned q,os_schedule*s) { return os_real_simulate(j,n,p,q,s); }
size_t os_clock_faults(const uint8_t*r,size_t n,size_t f) {
 if(!f||f>16||(!r&&n)) return SIZE_MAX;
 uint8_t pages[16]={0}; unsigned live[16]={0}; size_t hand=0,miss=0;
 for(size_t i=0;i<n;++i) { size_t k=0; while(k<f && !(live[k]&&pages[k]==r[i])) ++k;
  if(k==f) { ++miss;pages[hand]=r[i];live[hand]=1;hand=(hand+1)%f; }
 } return miss;
}
''')}
    with tempfile.TemporaryDirectory(prefix='os mutations ') as temp:
        folder=Path(temp); object_file=folder/'real.o'
        def run(command): return subprocess.run(command,capture_output=True,text=True,timeout=30)
        baseline=folder/'baseline'
        built=run([args.compiler,'-std=c17','-UNDEBUG','-I'+str(lab/'include'),str(implementation/'oslab.c'),
                   str(implementation/'platform.c'),str(lab/'tests/test_os.c'),'-pthread','-o',str(baseline)])
        if built.returncode: raise RuntimeError(built.stderr)
        for case in ['schedulers','clock']:
            tested=run([str(baseline),case])
            if tested.returncode: raise AssertionError('Unmodified baseline must pass before mutation: '+case)
        built=run([args.compiler,'-std=c17','-I'+str(lab/'include'),'-Dos_simulate=os_real_simulate',
                   '-Dos_clock_faults=os_real_clock','-c',str(implementation/'oslab.c'),'-o',str(object_file)])
        if built.returncode: raise RuntimeError(built.stderr)
        for name,(case,code) in wrappers.items():
            source=folder/(name+'.c');source.write_text(code); executable=folder/name
            built=run([args.compiler,'-std=c17','-UNDEBUG','-I'+str(lab/'include'),str(source),str(object_file),
                       str(implementation/'platform.c'),str(lab/'tests/test_os.c'),'-pthread','-o',str(executable)])
            if built.returncode: raise RuntimeError(built.stderr)
            tested=run([str(executable),case])
            if tested.returncode==0: raise AssertionError('Faulty mechanism accepted: '+name)
            print('rejected '+name)
if __name__=='__main__': main()
