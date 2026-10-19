"""Accept only a semantic assertion refusal."""
import subprocess,sys
p=subprocess.run(sys.argv[1:],text=True,capture_output=True);out=p.stdout+p.stderr;print(out,end='')
raise SystemExit(0 if p.returncode==1 and 'SR_CHECK' in out and not any(x in out for x in ['AddressSanitizer','runtime error:','ThreadSanitizer']) else 1)
