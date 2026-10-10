"""Inspect search-index reproduction differences without dumping the index."""
import json
from common import ROOT, dump


def read_index(path):
    value=path.read_text(encoding='utf-8')
    return json.loads(value[value.index('(')+1:value.rindex(')')])


def main():
    latest=max((ROOT/'build/reproducibility').iterdir(),key=lambda p:p.name)
    original=read_index(ROOT/'build/html/searchindex.js')
    rebuilt=read_index(latest/'build/html/searchindex.js')
    differences={}
    for key in original.keys()|rebuilt.keys():
        if original.get(key)!=rebuilt.get(key):
            a,b=original.get(key),rebuilt.get(key)
            if isinstance(a,dict) and isinstance(b,dict):
                changed=[k for k in a.keys()|b.keys() if a.get(k)!=b.get(k)]
                differences[key]={'count':len(changed),'samples':[
                    {'key':k,'original':a.get(k),'rebuilt':b.get(k)} for k in sorted(changed)[:3]]}
            else:
                differences[key]={'type':type(a).__name__,'original_count':len(a) if hasattr(a,'__len__') else None,
                                  'rebuilt_count':len(b) if hasattr(b,'__len__') else None}
    report={'semantic_equal':original==rebuilt,'differing_fields':differences,
            'rebuild_directory':latest.relative_to(ROOT).as_posix()}
    dump(ROOT/'reviews/reproducibility_diagnosis.json',report)
    print(json.dumps(report,indent=2))


if __name__=='__main__':main()
