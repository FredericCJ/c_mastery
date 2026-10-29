"""Explicit successor composition; retained files are never rewritten."""
import argparse,hashlib,json,sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT/'tools'))
from adapt_http_depth_baseline import OLD,NEW
from c_source import replace_unique
def compose(destination):
 source=ROOT/'assessor/communications/http.c';text=source.read_text()
 # Keep the historical serializer callable for a discrepancy probe. The new
 # serializer is a separately reviewable implementation, not a hidden edit.
 text='#include "http_depth.h"\n'+replace_unique(replace_unique(text,OLD,NEW.lstrip()),'int http_response(','int retained_http_response(')
 text+='\n#include "flow.h"\nint http_response(const struct http_request *r,const char *d,int c,void *v,size_t n,size_t *z){return hf_response(r,d,c,0,0,v,n,z);}\n'
 destination.parent.mkdir(parents=True,exist_ok=True);destination.write_text(text,encoding='utf-8',newline='\n')
 return {'source':source.relative_to(ROOT).as_posix(),'source_sha256':hashlib.sha256(source.read_bytes()).hexdigest(),'changes':['Exact previously audited u32 Content-Length branch','Historical serializer retained under distinct symbol','Explicit separate successor serializer dispatch'],'output':str(destination)}
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--output',type=Path,required=True);a=p.parse_args();print(json.dumps(compose(a.output)))
