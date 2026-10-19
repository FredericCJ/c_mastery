import subprocess
import sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True)
text=r.stdout+r.stderr
if r.returncode!=41 or 'ECC_CHECK:' not in text or any(s in text for s in ['AddressSanitizer','runtime error:']):
    print(text);raise SystemExit('Expected a compiled semantic ECC_CHECK rejection')
print('PASS intended semantic rejection: '+text.strip())
