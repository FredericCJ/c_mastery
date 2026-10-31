"""Classify sweep producer failures by the tail of their logs."""
import re, sys, json
from pathlib import Path
L = Path(sys.argv[1])
rules = [
    ("usage", r"error: the following arguments are required|error: --\S+ is required|: error: argument"),
    ("infra-private-toolchain", r"/home/frede/|cmastery-[a-z0-9-]+-tools|build/b06-tools|build/posix-io-toolchain|build/checkpoint-tools|\.conda/|build/portability-compiler|build/header-sources|cmastery-algorithm-abi-sources|build/[a-z0-9-]+-tools/"),
    ("stale-catalog", r"Exercise catalog is stale"),
    ("rubric-export", r"exported rubric mismatch"),
    ("anchor", r"AnchorError|anchor matched|assert (text|source|body)\.count\(|count\(old\)==1|AssertionError: \('?[^)]*(unsigned|int|static)"),
    ("source-pin-stale", r"stale|changed during|source_sha256|historical receipts|input_sha256"),
    ("compile-error", r"error: |cc1: all warnings being treated as errors"),
    ("timeout", r"TimeoutExpired|timed out"),
    ("assertion", r"AssertionError"),
    ("missing-file", r"FileNotFoundError|No such file or directory"),
]
rows = [l.split("\t") for l in (L / "runs.tsv").read_text().splitlines() if l.strip()]
out = {}
for r in rows:
    pid, rc = r[0], r[1]
    if rc == "0":
        continue
    log = (L / f"{pid}.log").read_text(errors="replace") if (L / f"{pid}.log").exists() else ""
    tail = log[-3000:]
    cls = "other"
    for name, pat in rules:
        if re.search(pat, tail):
            cls = name
            break
    out.setdefault(cls, []).append(pid)
print(json.dumps({k: len(v) for k, v in out.items()}))
if len(sys.argv) > 2:
    Path(sys.argv[2]).write_text(json.dumps(out, indent=1))
