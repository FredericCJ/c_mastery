import subprocess
import sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True)
text=p.stdout+'\n'+p.stderr
if p.returncode==1 and 'ADO_CHECK' in text and not any(x in text for x in ['AddressSanitizer','ThreadSanitizer','runtime error:']):
    print('normal semantic rejection observed');raise SystemExit(0)
print(text);raise SystemExit(1)
