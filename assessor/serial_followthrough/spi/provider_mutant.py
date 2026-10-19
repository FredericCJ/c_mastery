"""Copy exactly one original edge defect; never mutate the frozen provider."""
import hashlib,json,pathlib,sys
source,out=map(pathlib.Path,sys.argv[1:]);raw=source.read_bytes();text=raw.decode('utf8')
needle='samples[i] = (uint8_t)((i & 1u) == (mode & 1));'
assert text.count(needle)==1
out.write_text(text.replace(needle,'samples[i] = (uint8_t)((i & 1u) != (mode & 1));'),encoding='utf8')
print(json.dumps(dict(source_sha256=hashlib.sha256(raw).hexdigest(),output_sha256=hashlib.sha256(out.read_bytes()).hexdigest(),mechanism='Wrong sampling edge without changing data or clock polarity')))
