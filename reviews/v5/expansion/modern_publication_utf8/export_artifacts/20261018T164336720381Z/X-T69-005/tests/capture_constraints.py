import pathlib,subprocess,sys,tempfile
compiler,source,dod=sys.argv[1:];source=pathlib.Path(source)
with tempfile.TemporaryDirectory(prefix='cmastery-capture-type-') as directory:
 for bad in [False,True]:
  command=[compiler,'-std=c17','-Wall','-Wextra','-Wpedantic','-Werror','-I'+str(source.parent.parent/'include'),'-I'+str(pathlib.Path(dod)/'include')]
  if bad:command+=['-DDS_BORROW_RETURN']
  r=subprocess.run([*command,'-c',str(source),'-o',str(pathlib.Path(directory)/'probe.o')],capture_output=True,text=True)
  assert (r.returncode!=0)==bad,(command,r.stderr)
  if bad:assert 'incompatible' in r.stderr.lower(),r.stderr
print('typed scalar thunk accepted; borrowed-pointer return rejected by incompatible function-pointer diagnostic')
