"""Deterministic corrections to newly authored B06 specs; no baseline edits."""
from pathlib import Path
HERE=Path(__file__).parent
def change(name,pairs):
    path=HERE/name;s=path.read_text(encoding='utf-8')
    for old,new in pairs:
        if new in s:continue
        if old in s:s=s.replace(old,new)
        else:raise ValueError((name,old))
    path.write_text(s,encoding='utf-8',newline='\n')
change('spec_t57.py',[("refs=['KREF','LOCKS','WORK','KUNIT']","refs=['KREF','LOCKS','WORK','KUNIT','UCOPY']")])
change('spec_t58.py',[("CHECK(po_stack(4096,9,&x)&&x==4080);","CHECK(po_stack(4096,9,&x)&&x==4080);CHECK(po_stack(4096,1,&x)&&x==4080);")])
change('spec_t61.py',[("for an in-place byte vector whose failure predicate", "for an in-place byte vector of length at most65535 whose failure predicate"),("uint64_t sum=0;for(j=0;j<n;++j)","uint64_t sum=0;if(n>65535)return n;for(j=0;j<n;++j)")])
change('spec_t62.py',[("refs=['EPOLL','URING']","refs=['EPOLL','URING','CANCEL','FSYNC','SENDFILE']"),("CHECK(li_expired(UINT32_MAX-4,0,7,&o)&&o==0);","CHECK(li_expired(UINT32_MAX-4,0,7,&o)&&o==0);CHECK(li_expired(UINT32_MAX-4,UINT32_MAX-3,7,&o)&&o==0);"),("An asynchronous cancel operation can complete independently from the operation it targets.","Cancel and target have distinct completion records. In the inspected io_uring contract, successful cancellation has posted the target CQE by submission return; EALREADY may leave a later target completion. Application consumption and ownership remain separate from kernel posting.")])
change('spec_t63.py',[("refs=['C11','ASAN']","refs=['C11','ASAN','FORMAT']")])
change('spec_t64.py',[("fabs, sqrt and isfinite are permitted primitives","fabs, fmax, sqrt and isfinite are permitted primitives"),("Return true when |actual-expected|<=atol+rtol*|expected|, evaluating a scaled comparison", "Use the mathematical target |actual-expected|<=atol+rtol*|expected| and define actual admission by the supplied rounded scaled comparison (boundary rounding is part of this contract), scaling")])
