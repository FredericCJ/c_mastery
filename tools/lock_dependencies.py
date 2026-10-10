import subprocess
import sys
from pathlib import Path
root=Path(__file__).resolve().parents[1]
result=subprocess.run([sys.executable,'-m','pip','freeze'],check=True,capture_output=True,text=True)
(root/'requirements-lock.txt').write_text(result.stdout,encoding='utf-8')
