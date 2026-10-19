"""Accept only the ordinary, bounded semantic assertion as a killed mutant."""
import subprocess,sys
r=subprocess.run(sys.argv[1:],capture_output=True,text=True,timeout=30)
sys.stdout.write(r.stdout);sys.stderr.write(r.stderr)
if r.returncode!=41 or 'CI_ASSERT:' not in r.stderr:
    raise SystemExit('Mutant was not rejected by its semantic property')
