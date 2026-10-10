"""Run the separately authored finite OS/RT fixture oracle."""
from pathlib import Path
import runpy
runpy.run_path(str(Path(__file__).resolve().parents[1]/'research/v3/verify_fixtures.py'),run_name='__main__')
