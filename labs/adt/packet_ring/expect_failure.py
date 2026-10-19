import subprocess
import sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True)
sys.stdout.write(p.stdout);sys.stderr.write(p.stderr)
assert p.returncode==1 and 'ADO_CHECK' in p.stderr and not any(x in p.stderr for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']), 'required normal semantic rejection missing'
