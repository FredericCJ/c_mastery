import subprocess,sys
p=subprocess.run([sys.argv[1]],capture_output=True,text=True,timeout=30)
assert p.returncode==41 and 'ES_ASSERT:' in p.stderr,(p.returncode,p.stdout,p.stderr)
print('ES_MUTANT_REJECTED')
