"""Read-only metadata screening; this never opens a library document payload."""
import collections, hashlib, json, pathlib, sys
sys.stdout.reconfigure(encoding='utf8')
ROOT = pathlib.Path(__file__).resolve().parents[4]
HERE = pathlib.Path(__file__).resolve().parent
LIB = pathlib.Path('F:/Knowledge')
SWE = pathlib.Path('F:/Projects/Writing/corpora/SWE/explorer/data')
def read(p): return json.loads(p.read_text(encoding='utf-8-sig'))
def digest(p): return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
    pointer=read(LIB/'_census/library_database/CURRENT.json')
    manifest=LIB/'Library/_database/manifest.json'
    assert digest(manifest)==pointer['manifest_sha256']
    m=read(manifest); records=LIB/'Library/_database/documents.jsonl'
    assert digest(records)==m['artifact_sha256']['documents.jsonl']
    docs=[json.loads(line) for line in records.read_text(encoding='utf8').splitlines()]
    print('LIBRARY',len(docs),'metadata rows; no payload opened')
    print('CATEGORIES',json.dumps(dict(collections.Counter(tuple(d.get('subjects',[])) for d in docs).most_common()) if False else collections.Counter('/'.join(d.get('subjects',[])) for d in docs).most_common(),ensure_ascii=False))
    for filename in ['corpus.json','elements.json']:
        data=read(SWE/filename); print(filename,'sha256',digest(SWE/filename),'keys',list(data))
        nodes=data.get('nodes',[]);print('nodes',len(nodes),'first keys',list(nodes[0]) if nodes else [])
        print('kinds',collections.Counter(str(n.get('kind',n.get('type'))) for n in nodes))
        if filename=='elements.json':
            for n in nodes:
                if len(sys.argv)>1 and n.get('kind') in sys.argv[1:]:print('ELEMENT',json.dumps(n,ensure_ascii=False))
    queries=['algorithm','data structure','compiler','operating system','architecture','embedded','concurr','cryptograph','compression','Unicode','numerical','formal','real time','network','database','programming language']
    for q in queries if '--books' in sys.argv else []:
        selected=[d for d in docs if q in d.get('title','').lower()]
        print('QUERY',q,'COUNT',len(selected))
        for d in selected[:20]: print('BOOK',d['document_id'],d['title'],d.get('subjects'),d.get('document_class'))
if __name__=='__main__':main()
