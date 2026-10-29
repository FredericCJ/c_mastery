"""Exact versioned SC experiment; frozen provider remains untouched."""
from pathlib import Path
import hashlib,json,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[2]/'tools'))
from c_source import replace_all_unique
root=Path(__file__).resolve().parents[2]
source=root/'assessor/b03_depth/depth.c'
text=source.read_text()
old_load='return memory_order_acquire;';old_store='memory_order b3_store_order(void){return memory_order_release;}'
target=Path(sys.argv[1]);target.write_text(replace_all_unique(text,[(old_load,'return memory_order_seq_cst;'),(old_store,'memory_order b3_store_order(void){return memory_order_seq_cst;}')]))
print(json.dumps({'provider':source.relative_to(root).as_posix(),'sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'output_sha256':hashlib.sha256(target.read_bytes()).hexdigest(),'changes':['b3_load_order acquire -> seq_cst','b3_store_order release -> seq_cst']}))
