#!/usr/bin/env python3
"""Install and run the read-only Serena pilot using repository-local state."""
import argparse
import fcntl
import os
from pathlib import Path
import subprocess
import venv

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "tools/serena_author"
STATE = ROOT / "build/author-tools/serena"
BIN = STATE / "venv/bin"
COMPILE_DB = STATE / "cmake-foundations"
CONFIG_HOME = STATE / "home"


def install():
    if not (BIN / "python").is_file():
        venv.create(STATE / "venv", with_pip=True)
    subprocess.run([BIN / "python", "-m", "pip", "install", "--disable-pip-version-check",
                    "-r", ASSETS / "requirements.lock"], check=True)
    subprocess.run(["cmake", "-S", ROOT / "labs/foundations", "-B", COMPILE_DB,
                    "-G", "Ninja", "-DCMAKE_EXPORT_COMPILE_COMMANDS=ON",
                    "-DCMASTERY_ASSESSOR_ROOT="], check=True)
    project = STATE / "project"
    project.mkdir(exist_ok=True)
    project_stage = project / "project.yml.new"
    project_stage.write_bytes((ASSETS / "project.yml").read_bytes())
    project_stage.replace(project / "project.yml")
    CONFIG_HOME.mkdir(exist_ok=True)
    config_stage = CONFIG_HOME / "serena_config.yml.new"
    config_stage.write_bytes((ASSETS / "global.yml").read_bytes())
    config_stage.chmod(0o600)
    config_stage.replace(CONFIG_HOME / "serena_config.yml")


def serve():
    if not (COMPILE_DB / "compile_commands.json").is_file() or not (BIN / "serena").is_file():
        raise SystemExit("Install the local pilot first with tools/author-tool serena install.")
    env = dict(os.environ, SERENA_HOME=str(CONFIG_HOME), SERENA_USAGE_REPORTING="false",
               DO_NOT_TRACK="1", XDG_CACHE_HOME=str(STATE / "cache"),
               PATH=str(BIN) + os.pathsep + os.environ["PATH"])
    os.chdir(ROOT)
    args = [BIN / "serena", "start-mcp-server", "--transport", "stdio", "--project", ROOT,
            "--context", ASSETS / "context.yml", "--mode", "no-memories", "--mode", "no-onboarding",
            "--agent-interface", "tools", "--enable-web-dashboard", "false",
            "--open-web-dashboard", "false", "--enable-gui-log-window", "false", "--log-level", "WARNING"]
    os.execve(BIN / "serena", [str(arg) for arg in args], env)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("action", choices=("install", "serve", "check"))
    action = parser.parse_args().action
    if action == "install":
        STATE.mkdir(parents=True, exist_ok=True)
        with (STATE / "install.lock").open("a") as lock:
            fcntl.flock(lock, fcntl.LOCK_EX)
            install()
    elif action == "serve":
        serve()
    else:
        subprocess.run([BIN / "python", ASSETS / "check.py"], check=True, cwd=ROOT, timeout=150)
