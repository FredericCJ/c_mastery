"""Resolve installed tools without inheriting a previous author's home directory."""
from __future__ import annotations

import hashlib
import os
from pathlib import Path
import re
import shutil
import subprocess


def resolve(name, override=None):
    """Explicit argument, CMASTERY_TOOL_<NAME>, then PATH; never a silent fallback."""
    key = 'CMASTERY_TOOL_' + re.sub('[^A-Za-z0-9]', '_', name).upper()
    selected = override or os.environ.get(key) or name
    path = shutil.which(str(selected))
    if path is None:
        raise FileNotFoundError(f'{name} unavailable: install it or set {key}')
    return str(Path(path).absolute())


def identify(name, override=None):
    path = resolve(name, override)
    with open(path, 'rb') as stream:
        digest = hashlib.file_digest(stream, 'sha256').hexdigest()
    result = subprocess.run([path, '--version'], capture_output=True, text=True, timeout=15)
    return dict(name=name, path=path, resolved_path=str(Path(path).resolve()), sha256=digest,
                version=(result.stdout + result.stderr).strip(), exit_code=result.returncode)
