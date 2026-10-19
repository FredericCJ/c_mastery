"""One exact original-target mutation; frozen producer remains unchanged."""
import hashlib,json,pathlib,sys
source=pathlib.Path(sys.argv[1]);output=pathlib.Path(sys.argv[2]);text=source.read_text();start=text.index('int pi_step(');end=text.index('\nint pi_recover_begin(',start);body=text[start:end];old='if (!s->phase || token != s->token)';assert body.count(old)==1
output.write_text(text[:start]+body.replace(old,'if ((void)token, !s->phase)')+text[end:]);print(json.dumps(dict(status='pass',mutation='ignore event token in pi_step',source=str(source),source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),output_sha256=hashlib.sha256(output.read_bytes()).hexdigest())))
