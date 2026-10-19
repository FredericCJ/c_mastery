"""Apply individually owned ADT registrations without rewriting other modules."""
from pathlib import Path
import importlib.util
ROOT=Path(__file__).resolve().parents[1]
def register():
    for path in sorted((ROOT/'tools').glob('register_adt_*.py')):
        if path.name=='register_adt_followthrough.py':continue
        spec=importlib.util.spec_from_file_location(path.stem,path)
        module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
        module.register()
if __name__=='__main__':register()
