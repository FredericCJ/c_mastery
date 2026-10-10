"""Compile the selected driver core for Cortex-M4 and M7; no board claim."""
import argparse,pathlib,subprocess,tempfile
p=argparse.ArgumentParser();p.add_argument('--cc',required=True);p.add_argument('--source',required=True);p.add_argument('--include',required=True);a=p.parse_args()
with tempfile.TemporaryDirectory(prefix='cmastery-fw-') as d:
 for cpu in ('cortex-m4','cortex-m7'):
  output=pathlib.Path(d)/(cpu+'.o')
  subprocess.run([a.cc,'-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-ffreestanding','-fno-builtin','-mcpu='+cpu,'-mthumb','-I'+a.include,'-c',a.source,'-o',str(output)],check=True)
  nm=str(pathlib.Path(a.cc).with_name('arm-none-eabi-nm'))
  undefined=subprocess.check_output([nm,'-u',str(output)],text=True)
  if undefined.strip():raise SystemExit('Unexpected runtime imports: '+undefined)
print('Cortex-M4/M7 compilation and undefined-symbol audit passed; physical behavior remains unobserved.')
