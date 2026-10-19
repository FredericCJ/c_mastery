"""Record bounded actual source inspection before new real-time model authorship."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4];BASE=Path(__file__).resolve().parent
def main():
    old=json.loads((ROOT/'research/v5/expansion/legacy_concurrency/resources.json').read_text(encoding='utf-8'))
    book=next(r for r in old['resources'] if r['id']=='R-LCON-RT')
    pages=['PDF physical97: task parameters and assumptions A1–A7','PDF physical103 and117: RM and EDF definitions and implicit-deadline theorem assumptions',
      'PDF physical120–121: constrained deadlines and DM assignment/optimality scope','PDF physical122–123: response-time recurrence4.17, iteration4.18, sufficient/necessary distinction and initialization; page122 also visually inspected']
    resources=[{'id':'R-LRT-PERIODIC','title':book['title'],'authors':book.get('authors',[]),'url':book['url'],'kind':'book','edition':'Third edition2011',
      'domains':['T52'],'locators':pages,'access':'Actual local PDF passages read','role':'oracle','assignable':True,
      'reason':'Exact original periodic scheduling assumptions and no-jitter/no-blocking recurrence; fuller jitter/blocking claims require their own derivation.',
      'prerequisites':['Fixed-priority interference and discrete job model'],
      'inspection':{'date':'2026-10-17','method':'Extracted complete selected pages and visually inspected page122 formula/figure; verified local PDF SHA256.',
        'scope':'; '.join(pages),'limits':'These passages do not inspect the cited Audsley papers, prove arbitrary release jitter, certify measured WCET or establish all server/multicore variants.',
        'local_document_id':book['inspection']['local_document_id'],'local_sha256':book['inspection']['local_sha256']}},
      {'id':'R-LRT-COURSE','title':'Actual original T52 contracts and bounded scheduler/response source','authors':['C Mastery'],'url':'labs/execution/README.md',
       'kind':'course','edition':'Retained C17 execution baseline','domains':['T52'],'locators':[
        'curriculum/modules/T52.md complete study sequence and limits; PR-T52-01/02/03 complete canonical briefs and9 exact original criteria/delayed tasks',
        'assessor/execution/cmx.c cmx_response and cmx_edf_admit complete functions, lines664–723; response/edf tests in labs/execution/tests/contracts.c',
        'assessor/execution/advanced.c cmx_priority_order,cmx_partition,cmx_simulate complete functions; cmx_advanced.h model declaration'],
       'access':'Actual bounded source passages read','role':'oracle','assignable':True,'reason':'Retain actual producer contracts and distinguish their bounded support from missing job/backlog/full-width/error evidence.',
       'prerequisites':['Completed original CMX producer'],
       'inspection':{'date':'2026-10-17','method':'Read exact listed implementation, contracts and original criteria before successor design.',
         'scope':'Finite bounded integer analysis, partial priority-order output and one-job replacement simulator; exact original leaf tasks.',
         'limits':'No complete review of remaining T52 algorithms, actual target timing or independent learner evidence.'}}]
    def write(path,value):path.write_text(json.dumps(value,indent=2,ensure_ascii=False)+'\n',encoding='utf-8',newline='\n')
    path=BASE/'resources.json';write(path,{'schema_version':1,'resources':resources})
    write(BASE/'research_gate.json',{'status':'passage-inspected','domains':['T52'],'inspection_kind':'Author bounded actual source inspection; no independent human claim',
      'research_input_sha256':{path.relative_to(ROOT).as_posix():hashlib.sha256(path.read_bytes()).hexdigest()}})
if __name__=='__main__':main()
