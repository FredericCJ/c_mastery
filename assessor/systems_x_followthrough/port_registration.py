"""Exact zero-initialized scratch experiment; retained B045 provider untouched."""
from pathlib import Path
import hashlib,json,sys
ROOT=Path(__file__).resolve().parents[2]
sys.path.insert(0,str(ROOT/'tools'))
from c_source import replace_unique
source=ROOT/'assessor/b045_systems/systems.c'
text=source.read_text();old='unsigned char current[32],candidate[32];';new='unsigned char current[32]={0},candidate[32]={0};'
destination=Path(sys.argv[1]);destination.write_text(replace_unique(text,old,new))
print(json.dumps({'source':source.relative_to(ROOT).as_posix(),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'output_sha256':hashlib.sha256(destination.read_bytes()).hexdigest(),'exact_change':[old,new],'reason':'GCC15.2 -O2 -Werror maybe-uninitialized for the valid zero-length classify span; initialize local scratch, preserving admitted callback extents and outcomes; no warning suppression.'}))
