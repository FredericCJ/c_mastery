"""A compiled mutant counts only when a semantic CHECK, not a crash, rejects it."""
import subprocess
import sys

result = subprocess.run(sys.argv[1:], text=True, capture_output=True, timeout=15)
if result.returncode != 1 or "FD_CHECK:" not in result.stderr:
    print(result.stdout, result.stderr)
    raise SystemExit("mutant survived or failed without an identified semantic assertion")
print("compiled mutation rejected by", result.stderr.splitlines()[0])
