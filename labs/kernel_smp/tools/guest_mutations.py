"""Assessor-only mutation controls: compile copies; require actual guest failure."""
import argparse,json,os,re,shutil,subprocess,sys
from pathlib import Path
import guest

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


def replace_unique(text,before,after):
 spans=anchor_spans(text,before)
 if len(spans)!=1:raise RuntimeError('Mutation anchor matched %d times: %s'%(len(spans),before))
 return text[:spans[0][0]]+after+text[spans[0][1]:]
def main():
 p=argparse.ArgumentParser();p.add_argument('--lab',type=Path,required=True);p.add_argument('--source',type=Path,required=True);p.add_argument('--build',type=Path,required=True);a=p.parse_args()
 a.build.mkdir(parents=True,exist_ok=True);os.environ['CMASTERY_CPUS']='4';records=[]
 specs=[('tlb_ack','smp.c','__atomic_store_n(&shoot_ack[c],__atomic_load_n(&shoot_generation,__ATOMIC_ACQUIRE),__ATOMIC_RELEASE);','/* mutation: omit remote acknowledgement */','tlb'),('msix_mask','kernel.c','entry[2]=43;barrier();entry[3]=0;','entry[2]=43;barrier();entry[3]=1;','async')]
 for name,file,before,after,stage in specs:
  root=a.build/name;source=root/'source';shutil.copytree(a.source,source,dirs_exist_ok=True)
  path=source/file;path.write_text(replace_unique(path.read_text(),before,after))
  output=root/'guest';guest.build(a.lab.resolve(),source.resolve(),output.resolve())
  cmd=[sys.executable,str(a.lab/'tools/guest.py'),'stage','--stage',stage,'--lab',str(a.lab),'--source',str(source),'--build',str(output)]
  result=subprocess.run(cmd,capture_output=True,text=True,timeout=30)
  evidence=json.loads((output/('stage-'+stage+'.json')).read_text());log=(output/('stage-'+stage+'.log')).read_text()
  # Missing marker alone is not enough: require guest failure or the configured
  # host deadline after the preceding real guest milestone, not external interruption.
  killed=result.returncode!=0 and not evidence['passed'] and ('FAIL' in log or (name=='tlb_ack' and evidence.get('deadline_expired') and 'PASS smp-affinity-migration' in log))
  records.append({'mutation':name,'detected':killed,'command':cmd,'exit':result.returncode,'detector':'host deadline' if evidence.get('deadline_expired') else 'guest failure','log':str(output/('stage-'+stage+'.log')),'kernel_sha256':json.loads((output/'build.json').read_text())['kernel_sha256']})
  print(name,'detected',killed,flush=True)
  if not killed:print(result.stdout,result.stderr,log)
 report={'passed':all(x['detected'] for x in records),'records':records};(a.build/'mutations.json').write_text(json.dumps(report,indent=2)+'\n')
 if not report['passed']:raise SystemExit(1)
if __name__=='__main__':main()
