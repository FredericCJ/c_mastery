"""Build public P4 ordinary checks with explicit caller-selected providers."""
from pathlib import Path
import argparse,subprocess
p=argparse.ArgumentParser()
for name in ['source','parser','include','tests','build']:p.add_argument('--'+name,type=Path,required=True)
p.add_argument('--cc',default='gcc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True)
main=a.build/'main.c';main.write_text('#include <stdio.h>\nvoid cp_test_firmware_ring(void);void cp_test_firmware_tx(void);void cp_test_firmware_clock(void);int main(void){cp_test_firmware_ring();cp_test_firmware_tx();cp_test_firmware_clock();puts("P4_ORDINARY_PASS");}\n')
exe=a.build/'p4';subprocess.run([a.cc,'-std=c17','-O0','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(a.include),str(a.source),str(a.parser),str(a.tests),str(main),'-o',str(exe)],check=True)
raise SystemExit(subprocess.run([str(exe)]).returncode)
