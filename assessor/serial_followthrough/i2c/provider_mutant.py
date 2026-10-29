"""One exact original-target mutation; frozen producer remains unchanged."""
import hashlib,json,pathlib,sys
sys.path.insert(0,str(pathlib.Path(__file__).resolve().parents[3]/'tools'))
from c_source import function_span,replace_unique
source=pathlib.Path(sys.argv[1]);output=pathlib.Path(sys.argv[2]);text=source.read_text();start,end=function_span(text,'pi_step');body=text[start:end];old='if (!s->phase || token != s->token)'
output.write_text(text[:start]+replace_unique(body,old,'if ((void)token, !s->phase)')+text[end:]);print(json.dumps(dict(status='pass',mutation='ignore event token in pi_step',source=str(source),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),output_sha256=hashlib.sha256(output.read_bytes()).hexdigest())))
