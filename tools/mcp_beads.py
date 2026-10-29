#!/usr/bin/env python3
"""Launch the local beads-mcp stdio server with the repository's sandboxed bd.

Claude Code starts this from `.mcp.json`. The server only shells out to the
pinned `bd` binary in sandbox mode with telemetry disabled; it opens no
network listener and pushes nothing. Restore with
`python3 tools/install_author_tools.py --install`.
"""
import os
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[1]
BASE = ROOT / "build/author-tools"
SERVER = BASE / "beads-mcp/bin/beads-mcp"
SHIM = BASE / "bin/bd-sandboxed"


def main():
    if not SERVER.is_file() or not (BASE / "bin/bd").is_file():
        print("beads-mcp is not installed; run python3 tools/install_author_tools.py --install",
              file=sys.stderr)
        return 2
    # beads-mcp invokes BEADS_PATH directly, so give it a wrapper that always
    # adds --sandbox (no auto-sync or remote operations).
    SHIM.write_text('#!/bin/sh\nexec "{}" --sandbox "$@"\n'.format(BASE / "bin/bd"))
    SHIM.chmod(0o755)
    env = dict(os.environ)
    env.update(BEADS_PATH=str(SHIM), BEADS_WORKING_DIR=str(ROOT), BEADS_ACTOR="claude",
               DO_NOT_TRACK="1", BD_DISABLE_METRICS="1",
               DOLT_ROOT_PATH=str(BASE / "dolt-config"))
    os.execve(str(SERVER), [str(SERVER)], env)


if __name__ == "__main__":
    raise SystemExit(main())
