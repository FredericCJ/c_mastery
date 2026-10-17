import subprocess,sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=10)
if r.returncode!=1 or 'B03_ASSERT:'+sys.argv[2]+':' not in r.stderr:
    raise SystemExit('Not a semantic mutant rejection: '+repr(r))
print('rejected',sys.argv[2],'by semantic assertion')
