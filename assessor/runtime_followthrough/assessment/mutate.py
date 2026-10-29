"""Exact-source semantic controls; original provider unchanged."""
from pathlib import Path
import hashlib,json,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[3]/'tools'))
from c_source import replace_unique
source=Path(sys.argv[1]);out=Path(sys.argv[2]);kind=sys.argv[3];s=source.read_text()
if kind=='verifier':old='if (h != 1)';new='if (h != 1 && h != 0)'
elif kind=='roots':
 old='    s = cvm_root_push(v, a);\n    if (s != CVM_OK)\n      return s;\n    s = cvm_root_push(v, b);\n    if (s != CVM_OK) {\n      cvm_root_pop(v);\n      return s;\n    }\n    s = cvm_collect(v);\n    cvm_root_pop(v);\n    cvm_root_pop(v);'
 new='    s = cvm_collect(v);'
else:raise SystemExit('unsupported control')
lead=old[:len(old)-len(old.lstrip())];out.write_text(replace_unique(s,old,new[len(lead):] if new.startswith(lead) else new));out.with_suffix('.mutation.json').write_text(json.dumps(dict(source=str(source),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),kind=kind,old=old,new=new,output_sha256=hashlib.sha256(out.read_bytes()).hexdigest()),indent=2)+'\n')
