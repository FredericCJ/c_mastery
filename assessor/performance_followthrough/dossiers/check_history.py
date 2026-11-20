"""Verify preserved files, rederive the three AMD captures, and check the replay fixture.

Use --git-sources in the author clone to verify archived C sources and headers
against the original branch, before the main API adaptation.
"""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
import sys

from extract_replay import ARCHIVE, PUBLIC, ROOT, extract

sys.path.insert(0, str(ROOT / "labs/performance_followthrough/dossiers"))
from run_dossier import analyze

HANDOFF = "1cf5f5cf49d25ad6fc6a3f4389709c73b3e9c027"


def digest(data):
    return hashlib.sha256(data).hexdigest()


def require(ok, message):
    if not ok:
        raise ValueError(message)


def check_sources(rows):
    for row in rows:
        path = row["path"].partition("/c_mastery/")[2]
        for target in (path, "planning/handoff/b8_wip/" + path):
            obj = subprocess.run(["git", "show", f"{HANDOFF}:{target}"], cwd=ROOT, capture_output=True)
            if obj.returncode == 0 and digest(obj.stdout) == row["sha256"]:
                break
        else:
            raise ValueError(f"no archived source matches {path}")


def check(directory, git_sources):
    campaign = json.loads((directory / "campaign.json").read_text())
    build = json.loads((directory / "build.json").read_text())
    missing = []
    for name, expected in campaign["files_sha256"].items():
        path = directory / name
        if not path.exists():
            missing.append(name)
        else:
            require(digest(path.read_bytes()) == expected, f"historical file changed: {path}")
    require(missing == [build["binary"]], f"unexpected missing files: {missing}")
    require(build["binary_sha256"] == campaign["binary_sha256"], "conflicting executable identity")
    if git_sources:
        check_sources(build["sources"])
    result = analyze(directory)
    historic = json.loads((directory / "analysis_rederived.json").read_text())
    for field in ("hypotheses", "rows", "regimes"):
        require(result[field] == historic[field], f"{directory.name}: reanalysis differs at {field}")
    return {"dossier": directory.name, "rows": result["rows"], "regime_groups": len(result["regimes"]),
            "missing_original_executable": missing[0], "compiled_source_files": len(build["sources"]),
            "binary_sha256": build["binary_sha256"], "historical_data_files_checked": len(campaign["files_sha256"]) - 1}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--git-sources", action="store_true")
    args = parser.parse_args()
    folders = ("representation_locality", "dispatch_predictability", "concurrent_state")
    results = [check(ARCHIVE.parent / name, args.git_sources) for name in folders]
    require(json.loads(PUBLIC.read_text()) == extract(), "public replay differs from its archive")
    print(json.dumps(results, indent=2))
