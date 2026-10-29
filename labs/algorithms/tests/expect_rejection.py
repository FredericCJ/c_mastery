"""Require a semantic assertion, not an arbitrary nonzero process result."""
import argparse
import json
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("command", nargs=argparse.REMAINDER)
    args = parser.parse_args()
    try:
        run = subprocess.run(args.command, capture_output=True, text=True, timeout=25)
    except subprocess.TimeoutExpired:
        print(json.dumps({"outcome": "timeout"}))
        return 1
    except OSError as error:
        print(json.dumps({"outcome": "infrastructure-failure", "error": str(error)}))
        return 1
    diagnostic = re.search(r"AddressSanitizer|LeakSanitizer|ThreadSanitizer|runtime error:", run.stderr)
    semantic = run.returncode == 1 and re.search(r"test_algo\.c:\d+: .+", run.stderr) and not diagnostic
    outcome = "semantic-rejection" if semantic else "unexpected-result"
    if run.returncode < 0:
        outcome = "unrelated-crash"
    elif diagnostic:
        outcome = "unexpected-diagnostic"
    print(json.dumps({"outcome": outcome, "exit_code": run.returncode,
                      "stdout": run.stdout, "stderr": run.stderr}))
    return 0 if semantic else 1


if __name__ == "__main__":
    raise SystemExit(main())
