#!/usr/bin/env python3
"""Restore pinned, optional author tools without modifying Git history or hooks."""
from __future__ import annotations

import argparse
from datetime import datetime, timezone
import hashlib
import importlib.machinery
import importlib.util
import json
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tarfile
import tempfile
import urllib.request

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / "build/author-tools"
LOCK = json.loads((ROOT / "tools/author_tools.lock.json").read_text())
_loader = importlib.machinery.SourceFileLoader("author_tool", str(ROOT / "tools/author-tool"))
_spec = importlib.util.spec_from_loader(_loader.name, _loader)
wrapper = importlib.util.module_from_spec(_spec)
_loader.exec_module(wrapper)
SUPERPOWERS = (
    "brainstorming", "diagnosing-superpowers", "dispatching-parallel-agents",
    "executing-plans", "finishing-a-development-branch", "receiving-code-review",
    "requesting-code-review", "subagent-driven-development", "systematic-debugging",
    "test-driven-development", "using-git-worktrees", "using-superpowers",
    "verification-before-completion", "writing-plans", "writing-skills",
)


def run(args, *, cwd=ROOT, extra_env=None):
    env = wrapper.environment()
    if extra_env:
        env.update(extra_env)
    subprocess.run([str(a) for a in args], cwd=cwd, env=env, check=True)


def sha256(path):
    with Path(path).open("rb") as source:
        return hashlib.file_digest(source, "sha256").hexdigest()


def fetch(url, destination, expected):
    destination.parent.mkdir(parents=True, exist_ok=True)
    if destination.exists() and sha256(destination) == expected:
        return
    temporary = destination.with_suffix(destination.suffix + ".download")
    with urllib.request.urlopen(url, timeout=120) as response, temporary.open("wb") as output:
        shutil.copyfileobj(response, output)
    if sha256(temporary) != expected:
        raise RuntimeError(f"Checksum mismatch: {url}; download retained at {temporary}")
    temporary.replace(destination)


def python_packages():
    python = BASE / "python/bin/python"
    if not python.exists():
        run([sys.executable, "-m", "venv", BASE / "python"])
    requirements = ROOT / LOCK["python_lock"]
    expected = dict(line.strip().split("==", 1) for line in requirements.read_text().splitlines() if line.strip())
    result = subprocess.run([
        str(python), "-c",
        "import importlib.metadata as m,json; print(json.dumps({d.metadata['Name'].lower().replace('_','-'):d.version for d in m.distributions()}))",
    ], capture_output=True, text=True, check=True)
    installed = json.loads(result.stdout)
    if any(installed.get(name.lower().replace("_", "-")) != version for name, version in expected.items()):
        run([python, "-m", "pip", "install", "--disable-pip-version-check", "-r", requirements])


def binaries():
    bindir = BASE / "bin"
    bindir.mkdir(parents=True, exist_ok=True)
    openspec = LOCK["tools"]["openspec"]
    target = bindir / "openspec"
    fetch(openspec["binary_url"], target, openspec["sha256"])
    target.chmod(0o755)
    beads = LOCK["tools"]["beads"]
    target = bindir / "bd"
    if target.exists() and sha256(target) == beads["binary_sha256"]:
        return
    archive = BASE / f"beads_{beads['version']}_linux_amd64.tar.gz"
    fetch(beads["binary_url"], archive, beads["archive_sha256"])
    with tarfile.open(archive) as source:
        member = source.getmember("bd")
        if not member.isfile():
            raise RuntimeError("Beads archive bd entry is not a regular file")
        with source.extractfile(member) as data, target.open("wb") as output:
            shutil.copyfileobj(data, output)
    if sha256(target) != beads["binary_sha256"]:
        raise RuntimeError("Beads executable checksum mismatch")
    target.chmod(0o755)


def copy_missing(source, destination):
    """Preserve local customizations; only initialize absent integration files."""
    for item in sorted(source.rglob("*")):
        relative = item.relative_to(source)
        target = destination / relative
        if item.is_symlink():
            raise RuntimeError(f"Unexpected integration symlink: {item}")
        if item.is_file() and not target.exists():
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(item, target)


def integrations():
    skills = ROOT / ".agents/skills"
    # Restore repository-specific defaults before upstream initializers create templates.
    defaults = ROOT / "tools/author_tools_templates"
    for relative in ("AGENTS.md", "openspec/config.yaml", ".specify/memory/constitution.md", ".graphifyignore"):
        target = ROOT / relative
        source = defaults / relative
        if not target.exists() and source.is_file():
            target.parent.mkdir(parents=True, exist_ok=True)
            shutil.copy2(source, target)
    # Upstream initializers run in disposable staging directories, never this Git checkout.
    # Copy only skill/template content; exclude their generated AGENTS and hook files.
    with tempfile.TemporaryDirectory(prefix="cmastery-tool-integrations-") as directory:
        stage = Path(directory)
        if not (skills / "graphify/SKILL.md").exists():
            target = stage / "graphify"
            target.mkdir()
            run([BASE / "python/bin/graphify", "install", "--project", "--platform", "codex"], cwd=target)
            copy_missing(target / ".codex/skills/graphify", skills / "graphify")
        if not all((skills / f"openspec-{name}/SKILL.md").exists() for name in
                   ("apply-change", "archive-change", "explore", "propose", "sync-specs", "update-change")):
            target = stage / "openspec"
            target.mkdir()
            run([BASE / "bin/openspec", "init", "--tools", "codex", "--profile", "core", "--no-animation"], cwd=target)
            copy_missing(target / ".agents/skills", skills)
            copy_missing(target / "openspec", ROOT / "openspec")
        if not (skills / "speckit-implement/SKILL.md").exists() or not (ROOT / ".specify/templates").exists():
            target = stage / "spec-kit"
            target.mkdir()
            run([BASE / "python/bin/specify", "init", "--here", "--force", "--non-interactive",
                 "--integration", "codex", "--integration-options=--skills", "--ignore-agent-tools", "--script", "sh"], cwd=target)
            copy_missing(target / ".agents/skills", skills)
            copy_missing(target / ".specify", ROOT / ".specify")
        if not all((skills / name / "SKILL.md").exists() for name in SUPERPOWERS):
            source = BASE / "sources/superpowers"
            pin = LOCK["tools"]["superpowers"]
            if not source.exists():
                source.parent.mkdir(parents=True, exist_ok=True)
                run(["git", "clone", "--no-checkout", pin["upstream"], source])
            run(["git", "-C", source, "checkout", "--detach", pin["commit"]])
            actual = subprocess.check_output(["git", "-C", str(source), "rev-parse", "HEAD"], text=True).strip()
            if actual != pin["commit"]:
                raise RuntimeError("Superpowers source identity mismatch")
            for name in SUPERPOWERS:
                copy_missing(source / "skills" / name, skills / name)
            license_path = skills / "SUPERPOWERS-LICENSE"
            if not license_path.exists():
                shutil.copy2(source / "LICENSE", license_path)


def initialize_beads(destination=None):
    target = Path(destination) if destination else ROOT / ".beads"
    if any((target / name).exists() for name in ("dolt", "embeddeddolt", "proxieddb")):
        print(f"Existing Beads storage retained: {target}")
        return
    metadata = target / "metadata.json"
    if metadata.exists() and json.loads(metadata.read_text()).get("dolt_mode") != "embedded":
        raise RuntimeError("Refusing to replace configured non-embedded Beads storage")
    # Explicit BEADS_DIR prevents bd from doing git init; /tmp is outside the checkout.
    # A fresh project has no Git repository in which the upstream commit code can act.
    with tempfile.TemporaryDirectory(prefix="cmastery-beads-") as directory:
        stage = Path(directory)
        database = stage / ".beads"
        probe = subprocess.run(["git", "rev-parse", "--is-inside-work-tree"], cwd=stage,
                               capture_output=True, text=True)
        if probe.returncode == 0:
            raise RuntimeError("Beads initialization requires a stage outside any Git repository")
        run([BASE / "bin/bd", "--sandbox", "init", "--skip-agents", "--skip-hooks",
             "--non-interactive", "--role", "maintainer", "--prefix", "cmastery", "--remote", ""],
            cwd=stage, extra_env={"BEADS_DIR": str(database)})
        if (stage / ".git").exists():
            raise RuntimeError("Unexpected Git initialization in isolated Beads stage")
        if target.exists():
            stamp = datetime.now(timezone.utc).strftime("%Y%m%dT%H%M%S.%fZ")
            backup = BASE / "beads-config-backups" / stamp
            shutil.copytree(target, backup)
            copy_missing(database, target)
            shutil.copy2(database / "metadata.json", metadata)
            print(f"Retained previous configuration at {backup}")
        else:
            shutil.copytree(database, target)
        # Generated database configuration contains no staging-root paths.
        print(f"Initialized local Beads storage: {target}")


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--install", action="store_true", help="restore binaries, Python packages and project skills")
    parser.add_argument("--initialize-beads", action="store_true", help="initialize missing local DB outside Git; preserve existing storage")
    args = parser.parse_args()
    if not args.install and not args.initialize_beads:
        parser.error("select --install and/or --initialize-beads")
    if platform.system() != "Linux" or platform.machine() not in ("x86_64", "AMD64"):
        parser.error("the pinned native binary distribution supports Linux x86-64")
    BASE.mkdir(parents=True, exist_ok=True)
    if args.install:
        python_packages()
        binaries()
        integrations()
    if args.initialize_beads:
        initialize_beads()
    print("Optional author-tool restoration completed; curriculum evidence unchanged.")


if __name__ == "__main__":
    main()
