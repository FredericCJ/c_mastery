import subprocess
import sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True)
text=r.stdout+r.stderr
if r.returncode!=41 or 'CS_CHECK:' not in text or any(x in text for x in ['AddressSanitizer','runtime error:']):
    print(text);raise SystemExit(1)
print('SP_EXPECTED_REJECTION: compiled semantic control')
