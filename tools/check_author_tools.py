#!/usr/bin/env python3
"""Verify optional author-tool installation; this is not curriculum acceptance."""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import importlib.util
import json
from pathlib import Path
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / "build/author-tools"


def digest(path):
    with Path(path).open("rb") as source:
        return hashlib.file_digest(source, "sha256").hexdigest()


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--report", type=Path, help="new immutable JSON receipt")
    args = parser.parse_args()
    if args.report and args.report.exists():
        parser.error("report already exists; use a new path")
    spec = importlib.util.spec_from_file_location("install_author_tools", ROOT / "tools/install_author_tools.py")
    installer = importlib.util.module_from_spec(spec)
    spec.loader.exec_module(installer)
    lock = installer.LOCK
    checks = []

    def record(name, passed, **details):
        checks.append({"name": name, "passed": bool(passed), **details})
        print(f"{'PASS' if passed else 'FAIL'} {name}")

    def command(name, args_, *, expected=0, contains=None):
        result = subprocess.run([str(x) for x in args_], cwd=ROOT, env=installer.wrapper.environment(),
                                capture_output=True, text=True, timeout=90)
        output = result.stdout + result.stderr
        passed = result.returncode == expected and (contains is None or contains in output)
        record(name, passed, command=[str(x) for x in args_], exit_code=result.returncode,
               output=output[:20000])
        return result

    wrapper = [sys.executable, ROOT / "tools/author-tool"]
    head_before = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
    index_before = subprocess.check_output(["git", "diff", "--cached", "--binary"], cwd=ROOT)
    command("graphify version", [*wrapper, "graphify", "--version"], contains=lock["tools"]["graphify"]["version"])
    command("OpenSpec version", [*wrapper, "openspec", "--version"], contains=lock["tools"]["openspec"]["version"])
    command("Spec Kit version", [*wrapper, "specify", "version"], contains=lock["tools"]["spec-kit"]["version"])
    command("Beads version", [*wrapper, "bd", "--version"], contains=lock["tools"]["beads"]["version"])
    command("OpenSpec project ready", [*wrapper, "openspec", "list", "--json"], contains='"changes"')
    result = command("Beads project ready", [*wrapper, "bd", "ready", "--json"])
    try:
        record("Beads ready response is JSON", isinstance(json.loads(result.stdout), list))
    except ValueError:
        record("Beads ready response is JSON", False)
    command("direct Beads init rejected", [*wrapper, "bd", "init"], expected=2, contains="upstream init attempts")
    command("pinned restoration is idempotent", [sys.executable, ROOT / "tools/install_author_tools.py", "--install", "--initialize-beads"])
    for name, path, field in (("OpenSpec", BASE / "bin/openspec", "sha256"),
                              ("Beads", BASE / "bin/bd", "binary_sha256")):
        key = "openspec" if name == "OpenSpec" else "beads"
        record(f"{name} executable checksum", digest(path) == lock["tools"][key][field], sha256=digest(path))
    for name, version in lock["tools"]["c-tools"]["versions"].items():
        command(f"{name} pinned version", [BASE / "python/bin" / name, "--version"], contains=f"version {version}")
    expected_skills = set(installer.SUPERPOWERS) | {"graphify"} | {
        f"openspec-{name}" for name in ("apply-change", "archive-change", "explore", "propose", "sync-specs", "update-change")
    } | {f"speckit-{name}" for name in ("analyze", "checklist", "clarify", "constitution", "converge",
                                       "implement", "plan", "specify", "tasks", "taskstoissues")}
    missing = sorted(name for name in expected_skills if not (ROOT / ".agents/skills" / name / "SKILL.md").is_file())
    record("32 project skill entrypoints", not missing, expected=len(expected_skills), missing=missing)
    record("Superpowers license retained", (ROOT / ".agents/skills/SUPERPOWERS-LICENSE").is_file())
    for relative in ("AGENTS.md", "openspec/config.yaml", ".specify/memory/constitution.md", ".graphifyignore"):
        record(f"restore template matches {relative}",
               (ROOT / relative).read_bytes() == (ROOT / "tools/author_tools_templates" / relative).read_bytes())
    # Exercise the dangerous upstream initialization behavior in an isolated location.
    with tempfile.TemporaryDirectory(prefix="cmastery-beads-verify-") as directory:
        destination = Path(directory) / ".beads"
        installer.initialize_beads(destination)
        result = subprocess.run([str(BASE / "bin/bd"), "--sandbox", "ready", "--json"],
                                cwd=directory, env={**installer.wrapper.environment(), "BEADS_DIR": str(destination)},
                                capture_output=True, text=True, timeout=60)
        no_git = not (Path(directory) / ".git").exists()
        record("isolated Beads initialization and relocation", result.returncode == 0 and json.loads(result.stdout) == [] and no_git,
               exit_code=result.returncode, output=result.stdout + result.stderr,
               no_git_directory=no_git)
    graph_path = BASE / "graphify-out/graph.json"
    graph = json.loads(graph_path.read_text()) if graph_path.exists() else {}
    nodes, edges = graph.get("nodes", []), graph.get("edges", [])
    record("local AST graph available", bool(nodes) and bool(edges), nodes=len(nodes), edges=len(edges),
           graph_sha256=digest(graph_path) if graph_path.exists() else None,
           ast_only=all(node.get("_origin") == "ast" for node in nodes))
    command("local graph query", [*wrapper, "graphify", "query", "run_native_pmu", "--budget", "600"], contains="run_native_pmu")
    head_after = subprocess.check_output(["git", "rev-parse", "HEAD"], cwd=ROOT, text=True).strip()
    index_after = subprocess.check_output(["git", "diff", "--cached", "--binary"], cwd=ROOT)
    record("Git HEAD and staged content unchanged", head_before == head_after and index_before == index_after,
           head_before=head_before, head_after=head_after)
    source_paths = ["tools/author_tools.lock.json", "tools/author_tools_requirements.lock", "tools/install_author_tools.py",
                    "tools/check_author_tools.py", "tools/author-tool", "tools/c_tools_requirements.lock", "AGENTS.md", ".graphifyignore",
                    "openspec/config.yaml", ".specify/memory/constitution.md", "planning/author_tools.md"]
    receipt = {"schema_version": 1, "generated_at": datetime.now(timezone.utc).isoformat(),
               "scope": "optional local author tools; not curriculum acceptance or learner assessment",
               "passed": all(check["passed"] for check in checks), "checks": checks,
               "source_sha256": {name: digest(ROOT / name) for name in source_paths},
               "tool_versions": {name: item.get("version", item.get("commit")) for name, item in lock["tools"].items()},
               "graph_limits": ["Derived navigation index, not semantic curriculum coverage or a final source-bound check",
                                "Code-only extraction; no LLM calls, source uploads or community clustering",
                                "Initial extraction reported 33 files with syntax errors and 941 files yielding no symbols",
                                "Regenerate after source changes; graph includes assessor code and must remain local/author-only"],
               "installation_limits": ["Python dependencies are version-pinned; their wheel hashes are not locked, except the hash-pinned C tools",
                                       "Linux x86-64 native binary pins; Python dependencies verified here on Python 3.14",
                                       "No Git hooks or automatic remote publication configured"]}
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(json.dumps(receipt, indent=2) + "\n")
    print(json.dumps({"passed": receipt["passed"], "checks": len(checks), "report": str(args.report) if args.report else None}))
    return 0 if receipt["passed"] else 1


if __name__ == "__main__":
    raise SystemExit(main())
