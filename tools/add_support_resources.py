"""Record bounded official support-page inspections used in the common guide."""
from common import ROOT, dump

def main():
    specs=[
      ('R-WG14-PROJECTS-20261004','C — Project status and milestones','ISO/IEC JTC1/SC22/WG14','https://www.open-std.org/jtc1/sc22/wg14/www/projects',
       ['ISO/IEC 9899 - Revision of the C standard','Change and Clarification Requests'],
       'Published-edition identity table and explicit similar-draft qualifications; no linked standard clauses inspected here.',
       'Distinguishes published ISO editions, early later-edition drafts and committee corrections.'),
      ('R-GCC-C-STATUS-20261004','C Standards Support in GCC','GCC project','https://gcc.gnu.org/projects/c-status.html',
       ['C23 Support in GCC','C17 Support in GCC','C11 Support in GCC','C99 Support in GCC','Further notes'],
       'Dialect/default paragraphs, selected C23 feature rows, C11/C99 summaries and floating-point qualifications.',
       'Supplies qualified documented compiler-feature introductions, separate from local capability probes.'),
      ('R-CLANG-C-STATUS-20261004','C Support in Clang','LLVM/Clang project','https://clang.llvm.org/c_status.html',
       ['C23 implementation status','C17 implementation status'],
       'Overall partial/unknown status, mode availability and selected nullptr/typeof/auto/constexpr/embed feature rows.',
       'Prevents treating a C23 dialect flag or selected features as a whole-edition conformance certificate.')
    ]
    records=[]
    for key,title,author,url,locators,scope,reason in specs:
        records.append({'id':key,'title':title,'authors':[author],'edition':'Live official page, inspected 2026-10-04','year':None,
          'url':url,'access':'Free public web document','domains':['T05','T07'],'role':'Primary status/identity reference','reason':reason,'assignable':True,
          'locators':locators,'inspection':{'date':'2026-10-04','method':'Direct official-page body inspection with web retrieval','scope':scope,
          'limits':'Mutable page. No complete implementation audit or inspection of every linked proposal. Pin the selected installed release and execute its own probes.',
          'evidence_urls':[url]}})
    dump(ROOT/'references/supplemental_resources.json',{'resources':records})

if __name__=='__main__': main()
