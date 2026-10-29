"""Auditable copied-source adapter; never edits accepted baseline files."""
import argparse,hashlib,json
from pathlib import Path
from c_source import replace_unique
ROOT=Path(__file__).resolve().parents[1]
OLD="""    size_t len = 0;
    for (size_t i = 0; i < z; i++) {
      if (v[i] < '0' || v[i] > '9' ||
          len > (SIZE_MAX - (size_t)(v[i] - '0')) / 10)
        return fail(p, 400);
      len = len * 10 + (size_t)(v[i] - '0');
    }
    p->remaining = len;"""
NEW="""    uint32_t len = 0;
    int decision = hd_length((const unsigned char *)v, z, 1024, &len);
    if (decision == 400)
      return fail(p, 400);
    /* Valid over-cap values need only this sentinel at header finalization.
       This preserves its TE+CL-before-body-cap precedence. */
    p->remaining = decision == 413 ? 1025 : len;"""
def main():
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();source=ROOT/'assessor/communications/http.c';text=source.read_text(encoding='utf8');adapted=replace_unique(text,OLD,NEW.lstrip())
 a.output.parent.mkdir(parents=True,exist_ok=True);a.output.write_text('#include "http_depth.h"\n'+adapted,encoding='utf8');print(json.dumps(dict(source='assessor/communications/http.c',source_sha256=hashlib.sha256(source.read_bytes()).hexdigest(),output=str(a.output),changed_branch='Content-Length decimal parser only',scope='Explicit derived test copy; no accepted source mutation.')))
if __name__=='__main__':main()
