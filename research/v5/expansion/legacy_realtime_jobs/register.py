"""Register actually inspected multi-job scheduling passages before implementation."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
def main():
    old=json.loads((ROOT/'research/v5/expansion/legacy_concurrency/resources.json').read_text(encoding='utf-8'))
    book=next(r for r in old['resources'] if r['id']=='R-LCON-RT')
    locators=['PDF physical97: A1–A7 and separate release/relative/absolute deadline definitions',
      'PDF physical103,117,120–123: RM/EDF/DM ordering, constrained-deadline and response analysis assumptions',
      'PDF physical124–125: worked response iterations and guarantee pseudocode',
      'PDF physical126–128: level-i workload, processor-demand interval definition and synchronous constrained-deadline dbf; page128 visually inspected']
    resources=[{'id':'R-LRJ-BOOK','title':book['title'],'authors':book.get('authors',[]),'url':book['url'],'kind':'book','edition':'Third edition2011','domains':['T52'],
      'locators':locators,'access':'Actual bounded local PDF passages inspected','role':'oracle','assignable':True,'reason':'Exact scheduling/deadline/demand assumptions and finite-model boundaries.',
      'prerequisites':['Completed fixed-priority response and CMX scheduler artifacts'],
      'inspection':{'date':'2026-10-17','method':'Read complete selected text pages and visually inspect page128 equation/interval diagram before authoring.',
        'scope':'; '.join(locators),'limits':'Selected passages do not inspect all cited proofs, certify finite-horizon sufficiency, establish arbitrary deadline/backlog optimality, or provide physical timing.',
        'local_document_id':book['inspection']['local_document_id'],'local_sha256':book['inspection']['local_sha256']}},
      {'id':'R-LRJ-COURSE','title':'Original fixed-priority and EDF criteria with actual retained finite models','authors':['C Mastery'],'url':'labs/execution/README.md',
       'kind':'course','edition':'Retained C17 execution baseline','domains':['T52'],'locators':[
        'PR-T52-01/02 complete canonical briefs and six original criterion tasks/variants; T52 complete module teaching',
        'assessor/execution/advanced.c complete cmx_priority_order and cmx_simulate; cmx_advanced.h finite model scope',
        'assessor/execution/cmx.c complete cmx_response/cmx_edf_admit and corresponding original response/edf test bodies'],
       'access':'Actual bounded local source passages inspected','role':'oracle','assignable':True,'reason':'Compose actual producers and retain exact old prefix/one-job limitations rather than silently changing accepted source.',
       'prerequisites':['Completed original CMX and checked response successor'],
       'inspection':{'date':'2026-10-17','method':'Read actual source and full specified original contracts before new model design.',
         'scope':'Stable ordering, bounded finite demand and one-job replacement semantics; exact original learner obligations.',
         'limits':'No whole-domain code review, actual target scheduler latency or learner assessment.'}}]
    def write(p,v):p.write_text(json.dumps(v,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')
    path=BASE/'resources.json';write(path,{'schema_version':1,'resources':resources})
    write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T52'],'inspection_kind':'Author bounded primary/source inspection; not independent human review',
      'research_input_sha256':{path.relative_to(ROOT).as_posix():hashlib.sha256(path.read_bytes()).hexdigest()}})
if __name__=='__main__':main()
