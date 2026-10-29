"""Explicit composition of completed startup and interrupt learner adapters."""
import argparse,hashlib,importlib.util,json,re,shutil,subprocess
from pathlib import Path
PUBLIC=Path(__file__).resolve().parent
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


def spans(text,anchor):
 return anchor_spans(text,anchor)
def swap(text,anchor,replacement):
 found=spans(text,anchor)
 if len(found)!=1:raise ValueError('Startup prerequisite changed; inspect hook before composing')
 return text[:found[0][0]]+replacement+text[found[0][1]:]
def prepare(lab,kernel,startup_public,startup_completed,implementation,work,mode=0,mutant=0,compiler='gcc',optimization='-O2',observer=None):
 if mode not in (0,1) or mutant not in (0,1,2):raise ValueError('Unknown experiment')
 work.mkdir(parents=True,exist_ok=False);adapter=work/'composed';adapter.mkdir()
 old=(startup_completed/'startup_guest.c').read_text(encoding='utf-8')
 parked='void ss_parked(unsigned index){\n __atomic_store_n(&ss_observed[index].quiescent,1,__ATOMIC_RELEASE);\n for(;;)__asm__ volatile("cli;hlt");\n}'
 if len(spans(old,parked))!=1 or len(spans(old,'text("SS_GUEST_PASS:verified-generation-startup-and-quiescence\\n");'))!=1:raise ValueError('Startup prerequisite changed; inspect hook before composing')
 old=swap(old,parked,'void ss_parked(unsigned index){\n __atomic_store_n(&ss_observed[index].quiescent,1,__ATOMIC_RELEASE);\n si_worker(index);for(;;)__asm__ volatile("cli;hlt");\n}')
 old=swap(old,'text("SS_GUEST_PASS:verified-generation-startup-and-quiescence\\n");','si_bsp_enter((uintptr_t)(cpus[0].boot+sizeof cpus[0].boot));text("SI_GUEST_CHECK:unexpected-return\\n");')
 (adapter/'startup_guest.c').write_text('static void si_worker(unsigned);void si_bsp(void);void si_bsp_enter(uintptr_t);\n'+old+'\n'+(implementation/'interrupts_guest.c').read_text(encoding='utf-8'),encoding='utf-8',newline='\n')
 (adapter/'startup_trampoline.S').write_text((startup_completed/'startup_trampoline.S').read_text(encoding='utf-8')+'\n'+(implementation/'interrupts_frames.S').read_text(encoding='utf-8'),encoding='utf-8',newline='\n');shutil.copyfile(startup_completed/'startup.c',adapter/'startup.c');shutil.copyfile(implementation/'interrupts.c',adapter/'interrupts.c')
 spec=importlib.util.spec_from_file_location('interrupts_startup',startup_public/'guest.py');startup=importlib.util.module_from_spec(spec);spec.loader.exec_module(startup)
 extra=['-DSI_MODE='+str(mode),'-DSI_MUTANT='+str(1 if mutant==1 else 0),'-DSI_GUEST_MUTANT='+str(mutant),'-I'+str(adapter),'-I'+str(PUBLIC)]
 def run(command,**kwargs):
  command=list(command)
  if command[0] in ('gcc','clang') and '-c' in command:command[1:1]=extra
  if observer:return observer(command,**kwargs)
  result=subprocess.run(list(map(str,command)),capture_output=True,text=True,**kwargs)
  if result.returncode:raise ValueError(str(command)+'\n'+result.stdout+result.stderr)
  return result.stdout
 guest,instrumentation=startup.prepare(lab,kernel,adapter,work/'image',0x8000,1,0,compiler,optimization,run)
 return startup,guest,{'startup':instrumentation,'mode':mode,'mutant':mutant,'extra_flags':extra,'scope':'Explicit completed startup prerequisite, original controlled C/assembly worker and exact known hook; no source change to prerequisite.','composed_sha256':{p.name:hashlib.sha256(p.read_bytes()).hexdigest() for p in adapter.iterdir()}}
if __name__=='__main__':
 p=argparse.ArgumentParser(description=__doc__)
 for name in ('lab','kernel','startup-public','startup-completed','implementation','work'):p.add_argument('--'+name,type=Path,required=True)
 p.add_argument('--mode',type=int,choices=(0,1),default=0);p.add_argument('--mutant',type=int,choices=(0,1,2),default=0);p.add_argument('--cpus',type=int,choices=(1,2,4),default=4);a=p.parse_args()
 startup,g,info=prepare(a.lab,a.kernel,a.startup_public,a.startup_completed,a.implementation,a.work,a.mode,a.mutant)
 r=subprocess.run(startup.command(g,a.work/'image',a.cpus,'max' if a.mode else 'qemu64'),capture_output=True,text=True,timeout=60);(a.work/'execution.log').write_text(r.stdout+r.stderr,encoding='utf-8',newline='\n');print(r.returncode);print(r.stdout+r.stderr);raise SystemExit(0 if r.returncode==33 and 'SI_GUEST_PASS:' in r.stdout else 1)
