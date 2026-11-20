#!/usr/bin/env python3
"""Install the lexical QMD runtime and search curated curriculum Markdown."""
import argparse
import fcntl
import json
import os
from pathlib import Path
import platform
import shutil
import subprocess
import sys
import tarfile
import tempfile

from install_author_tools import fetch, sha256

ROOT = Path(__file__).resolve().parents[1]
SOURCES = ROOT / "tools/qmd_author"
BASE = ROOT / "build/author-tools/qmd"
NODE_PIN = json.loads((SOURCES / "node.lock.json").read_text())
NODE_HOME = BASE / f"node-v{NODE_PIN['version']}-linux-x64"
NODE = NODE_HOME / "bin/node"
# A new lock creates a new installation. Failed restores never overwrite a working one.
RUNTIME = BASE / "runtime" / sha256(SOURCES / "package-lock.json")
CLI = RUNTIME / "node_modules/@tobilu/qmd/dist/cli/qmd.js"
COLLECTIONS = {
    "planning": "planning",
    "guides": "curriculum/guides",
    "projects": "curriculum/projects",
    "routes": "curriculum/v5/routes",
    "contracts": "curriculum/v5/contracts",
    "exercises": "curriculum/v5/exercises",
}


def install():
    if (platform.system(), platform.machine()) != ("Linux", "x86_64"):
        raise ValueError("The QMD runtime pin supports Linux x86-64.")
    if not NODE_HOME.exists():
        archive = BASE / "downloads" / NODE_PIN["url"].rsplit("/", 1)[-1]
        fetch(NODE_PIN["url"], archive, NODE_PIN["sha256"])
        with tempfile.TemporaryDirectory(dir=BASE) as stage:
            with tarfile.open(archive) as source:
                source.extractall(stage, filter="data")
            unpacked = Path(stage) / NODE_HOME.name
            if sha256(unpacked / "bin/node") != NODE_PIN["binary_sha256"]:
                raise ValueError("Unexpected Node binary in verified archive")
            unpacked.rename(NODE_HOME)
    if sha256(NODE) != NODE_PIN["binary_sha256"]:
        raise ValueError("Installed Node executable differs from the tool lock.")
    if not RUNTIME.exists():
        RUNTIME.parent.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(dir=RUNTIME.parent) as stage:
            for filename in ("package.json", "package-lock.json"):
                shutil.copyfile(SOURCES / filename, Path(stage) / filename)
            npm = NODE_HOME / "lib/node_modules/npm/bin/npm-cli.js"
            # The pinned SQLite package includes its prebuilt binding. Keyword search
            # uses no optional inference backends or package installation scripts.
            subprocess.run([NODE, npm, "ci", "--ignore-scripts", "--omit=optional",
                            "--no-audit", "--no-fund", "--cache", BASE / "npm-cache"],
                           cwd=stage, check=True)
            Path(stage).rename(RUNTIME)
    return qmd(["--version"])


def qmd(arguments):
    if not CLI.is_file() or not NODE.is_file():
        raise ValueError("Run 'python3 tools/author-tool qmd install' first.")
    config = BASE / "config"
    config.mkdir(exist_ok=True)
    collections = {}
    for name, relative in COLLECTIONS.items():
        directory = ROOT / relative
        # Public source review archives omit the author planning directory.
        if name == "planning" and not directory.exists() and not directory.is_symlink():
            continue
        if directory.resolve() != directory or not directory.is_dir():
            raise ValueError(f"Missing or redirected QMD collection directory: {relative}")
        collections[name] = {"path": str(directory), "pattern": "*.md"}
    config_path = config / "cmastery.yml"
    config_path.write_text(json.dumps({"collections": collections}, indent=2) + "\n")
    env = {
        "PATH": os.defpath,
        "CI": "1",
        "NO_COLOR": "1",
        "QMD_CONFIG_DIR": str(config),
        "XDG_CACHE_HOME": str(BASE / "cache"),
        "XDG_CONFIG_HOME": str(config),
        "INDEX_PATH": str(BASE / "cmastery.sqlite"),
    }
    # Naming the index explicitly overrides any auto-discovered .qmd/index.yml.
    return subprocess.run([NODE, CLI, "--index", "cmastery", *arguments],
                          cwd=BASE, env=env).returncode


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--version", action="store_true", help="show installed QMD version")
    commands = parser.add_subparsers(dest="command")
    commands.add_parser("install", help="restore pinned Node, QMD and npm dependencies")
    commands.add_parser("index", help="refresh the local keyword index")
    search = commands.add_parser("search", help="BM25 keyword search")
    search.add_argument("terms", nargs="+")
    search.add_argument("-c", "--collection", choices=COLLECTIONS)
    search.add_argument("-n", type=int, default=5, metavar="LIMIT")
    search.add_argument("--json", action="store_true")
    args = parser.parse_args()
    if not args.command and not args.version:
        parser.error("select install, index, search or --version")
    BASE.mkdir(parents=True, exist_ok=True)
    with (BASE / "qmd.lock").open("a") as lock:
        fcntl.flock(lock, fcntl.LOCK_EX)
        if args.version:
            return qmd(["--version"])
        if args.command == "install":
            return install()
        ready = BASE / "index-ready"
        if args.command == "index":
            ready.unlink(missing_ok=True)
            status = qmd(["update"])
            if status == 0:
                ready.touch()
            return status
        if not ready.is_file() or not (BASE / "cmastery.sqlite").is_file():
            parser.error("run 'python3 tools/author-tool qmd index' first")
        if args.collection and not (ROOT / COLLECTIONS[args.collection]).exists():
            parser.error(f"the '{args.collection}' collection is absent from this checkout")
        if args.n < 1:
            parser.error("-n must be a positive integer")
        options = ["search", "-n", str(args.n), "--full-path"]
        if args.collection:
            options.extend(["-c", args.collection])
        if args.json:
            options.append("--json")
        return qmd([*options, "--", " ".join(args.terms)])


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError, subprocess.CalledProcessError) as error:
        sys.exit(str(error))
