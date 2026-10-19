"""Only the intended semantic assertion is an acceptable causal rejection."""
import subprocess,sys
p=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=30)
assert p.returncode==41 and 'SS_ASSERT:' in p.stderr,(p.returncode,p.stdout,p.stderr)
print('SS_MUTANT_REJECT',sys.argv[-1])
