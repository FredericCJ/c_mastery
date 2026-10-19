"""Only the intended semantic assertion is a mutant rejection."""
import subprocess
import sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True)
text=r.stdout+r.stderr
ok=r.returncode==41 and 'BL_CHECK:' in text and not any(x in text for x in
    ['AddressSanitizer','runtime error:','LeakSanitizer','ThreadSanitizer'])
print(text,end='')
raise SystemExit(0 if ok else 1)
