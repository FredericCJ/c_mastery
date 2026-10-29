"""Compile one explicit assessor mutation and demand semantic rejection.

Mutation definitions stay in the private assessor export. Compilation errors
are infrastructure failures, never accepted as a successful negative control.
"""
import argparse,json,pathlib,re,subprocess,tempfile
_ANCHOR_TOKEN = re.compile(r"[A-Za-z_]\w*|\d+|[^\s]")


def anchor_spans(text, anchor):
    """Character spans where `anchor`'s tokens occur in order in `text`.

    Same rule as tools/c_source.py: braces in the source that the anchor
    lacks may be skipped only when they balance inside the match, so an
    anchor written for `if (x) stmt;` still finds `if (x) { stmt; }`.
    """
    hay = [(m.group(), m.start(), m.end()) for m in _ANCHOR_TOKEN.finditer(text)]
    needle = _ANCHOR_TOKEN.findall(anchor)
    spans = []
    for start, (token, _, _) in enumerate(hay):
        if not needle or token != needle[0]:
            continue
        j, depth, ok = start, 0, True
        for want in needle:
            while j < len(hay) and hay[j][0] != want and hay[j][0] in "{}":
                depth += 1 if hay[j][0] == "{" else -1
                if depth < 0:
                    ok = False
                    break
                j += 1
            if not ok or j >= len(hay) or hay[j][0] != want:
                ok = False
                break
            j += 1
        if not ok:
            continue
        while depth > 0 and j < len(hay) and hay[j][0] == "}":
            depth -= 1
            j += 1
        if depth == 0:
            spans.append((hay[start][1], hay[j - 1][2]))
    return spans

p=argparse.ArgumentParser();p.add_argument('--cc',required=True);p.add_argument('--source',required=True);p.add_argument('--test',required=True);p.add_argument('--mutations',required=True);p.add_argument('--case',required=True);p.add_argument('--include',action='append',default=[]);p.add_argument('--extra',action='append',default=[]);p.add_argument('--define',action='append',default=[]);a=p.parse_args()
changes=json.loads(pathlib.Path(a.mutations).read_text())[a.case]
source=pathlib.Path(a.source).read_text()
for before,after in changes:
 found=anchor_spans(source,before)
 if len(found)!=1:raise SystemExit('Mutation anchor must occur exactly once: '+before)
 source=source[:found[0][0]]+after+source[found[0][1]:]
with tempfile.TemporaryDirectory(prefix='cmastery-mutant-') as d:
 root=pathlib.Path(d);c=root/'mutant.c';c.write_text(source);exe=root/'mutant'
 command=[a.cc,'-std=c17','-g','-Wall','-Wextra','-Wno-misleading-indentation',str(c),a.test,*a.extra,*['-I'+i for i in a.include],*['-D'+i for i in a.define],'-lm','-o',str(exe)]
 build=subprocess.run(command,capture_output=True,text=True)
 if build.returncode:raise SystemExit('Mutation did not compile:\n'+build.stdout+build.stderr)
 result=subprocess.run([str(exe),a.case],capture_output=True,text=True,timeout=10)
 if result.returncode!=1:raise SystemExit(f'Mutation must fail by a contract assertion, got {result.returncode}:\n'+result.stdout+result.stderr)
 print('Semantic rejection confirmed:',a.case)
