"""Known-bad controls: require an actual named assertion/guest failure."""
from __future__ import annotations
import argparse,json,pathlib,re,shutil,subprocess,sys
P=pathlib.Path
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


def replace_tokens(text,anchor,replacement,name):
 found=anchor_spans(text,anchor)
 if len(found)!=1:raise RuntimeError('Mutation token drift: '+name)
 return text[:found[0][0]]+replacement+text[found[0][1]:]
def main():
 p=argparse.ArgumentParser();p.add_argument('--source',type=P,required=True);p.add_argument('--lab',type=P,required=True);p.add_argument('--build',type=P,required=True);p.add_argument('--cc',default='cc');a=p.parse_args();a.build.mkdir(parents=True,exist_ok=True);results=[]
 mutations=[('inheritance','cmx.c','pass < CMX_TASKS','pass < 0','inheritance'),('capacity','concurrent.c','(unsigned)(h - t) >= q->capacity','(unsigned)(h - t) > q->capacity','spsc'),('condition','cmx.c','relock(k, t, result);','ready(k, t, result);','condition')]
 for name,file,old,new,test in mutations:
  b=a.build/name;b.mkdir(exist_ok=True);source=(a.source/file).read_text()
  altered=b/file;altered.write_text(replace_tokens(source,old,new,name));files=[altered if f==file else a.source/f for f in ('cmx.c','concurrent.c','advanced.c')];exe=b/'control'
  c=subprocess.run([a.cc,'-std=c17','-O1','-I'+str(a.lab/'include'),*map(str,files),str(a.lab/'tests/contracts.c'),'-o',str(exe)],text=True,capture_output=True)
  if c.returncode:raise RuntimeError('Control build failed: '+c.stdout+c.stderr)
  r=subprocess.run([str(exe),test],text=True,capture_output=True,timeout=10);rejected=r.returncode==1 and 'FAIL ' in r.stderr
  results.append({'control':name,'rejected':rejected,'returncode':r.returncode,'output':r.stdout+r.stderr,'evidence_kind':'host'})
  if not rejected:raise RuntimeError('Known defect accepted: '+name)
 if shutil.which('arm-none-eabi-gcc') and shutil.which('qemu-system-arm'):
  for name,file,old,new,stage in [('fp-context','context.S','vldmiaeq r0!,{s16-s31}','vldmiaeq r0!,{s16-s31}\n movs r1,#0\n vmov s16,r1','fp'),('bss-clear','startup.S','3: cmp r1,r2','3: b 4f\n cmp r1,r2','boot')]:
   b=a.build/name;src=b/'source';src.mkdir(parents=True,exist_ok=True)
   for f in a.source.iterdir():
    if f.is_file():shutil.copyfile(f,src/f.name)
   path=src/file;code=path.read_text()
   if code.count(old)!=1:raise RuntimeError('Mutation token drift: '+name)
   path.write_text(code.replace(old,new,1))
   r=subprocess.run([sys.executable,str(a.lab/'targets/build.py'),'--target','cortex-m4-qemu','--stage',stage,'--run','--source',str(src),'--lab',str(a.lab),'--build',str(b/'image')],capture_output=True,text=True,timeout=40)
   receipt=json.loads((b/'image/report.json').read_text());x=receipt.get('execution',{});rejected=r.returncode!=0 and x.get('status')=='failed' and 'FAIL CMX Cortex-M' in x.get('output','')
   results.append({'control':name,'rejected':rejected,'returncode':x.get('returncode'),'output':x.get('output'),'evidence_kind':'emulator'})
   if not rejected:raise RuntimeError('Guest defect not discriminated: '+name+'\n'+r.stdout+r.stderr)
 else:results.append({'control':'ARM controls','status':'unavailable','rejected':False})
 (a.build/'controls.json').write_text(json.dumps(results,indent=2)+'\n');print('PASS execution controls: '+str(sum(r.get('rejected',False) for r in results))+' actual defects rejected')
if __name__=='__main__':main()
