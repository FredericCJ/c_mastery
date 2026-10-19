"""Finite prediction checks; technical explanations require the separate rubric."""
import argparse,json
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('candidate');p.add_argument('--key',required=True);a=p.parse_args();candidate=json.loads(Path(a.candidate).read_text());key=json.loads(Path(a.key).read_text());bad=sorted(k for k in set(key)|set(candidate) if candidate.get(k)!=key.get(k));print(json.dumps(dict(passed=not bad,failed_fields=bad,reasoning_assessment='not-performed')));raise SystemExit(bool(bad))
