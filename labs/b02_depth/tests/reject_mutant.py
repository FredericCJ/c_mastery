"""Require a semantic assertion, not merely a crash or launch failure."""
import subprocess, sys
result = subprocess.run(sys.argv[1:], text=True, capture_output=True, timeout=10)
marker = "B02_ASSERT:" + sys.argv[2] + ":"
if result.returncode != 1 or marker not in result.stderr:
    raise SystemExit("Mutant was not rejected by a named semantic assertion: " + repr(result))
print("rejected", sys.argv[2], "by semantic assertion")
