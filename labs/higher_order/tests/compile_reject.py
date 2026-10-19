import pathlib,subprocess,sys,tempfile
with tempfile.TemporaryDirectory() as t:
 p=subprocess.run([sys.argv[1],'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+sys.argv[3],'-c',sys.argv[2],'-o',str(pathlib.Path(t)/'probe.o')],capture_output=True,text=True)
 assert p.returncode!=0 and ('incompatible' in p.stderr or 'passing' in p.stderr),p.stdout+p.stderr
print('Intended incompatible nominal carrier rejected')
