"""Supplemental original semantic controls and exact resource observations."""
from run_systems_x_followthrough import ROOT,LAB,REF,BASE,HEADERS,PUBLIC,sha
from pathlib import Path
import argparse,datetime,json,os,subprocess,sys
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();work=ROOT/'build/systems-x-controls'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True);rows=[]
 sources={Path(__file__),ROOT/'tools/run_systems_x_followthrough.py',REF/'costs.c',REF/'port_registration.py',*BASE.values(),ROOT/'labs/modern_bridge/tests/verify_fixtures.py',ROOT/'fixtures/modern_bridge/cases.json'}
 sources|={ROOT/'labs'/s for d in [HEADERS,PUBLIC] for s in d.values()}
 sources|={ROOT/f'labs/{f}/tests/tests.c' for f in ['b04_depth','b05_depth','b045_systems','modern_bridge']}
 sources|={p for p in LAB.rglob('*.h')};before={p.relative_to(ROOT).as_posix():sha(p) for p in sources}
 def run(name,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,capture_output=True,text=True,timeout=90);body=r.stdout+r.stderr;log=work/(str(len(rows))+'.log');log.write_text(body);passed=r.returncode==expected and (marker is None or marker in body) and not any(s in body for s in ['runtime error:','AddressSanitizer','ThreadSanitizer']);rows.append(dict(label=name,command=cmd,returncode=r.returncode,expected=expected,marker=marker,passed=bool(passed),raw=log.relative_to(ROOT).as_posix(),raw_sha256=sha(log)));print(name,bool(passed),flush=True);return passed
 port=work/'systems-port.c';run('registered source',[sys.executable,REF/'port_registration.py',port]);providers=dict(BASE,systems=port);incs=[LAB/x for x in ['reclamation','once','transfers']]+[ROOT/'labs'/Path(v).parent for v in HEADERS.values()];flags=['-std=c17','-O2','-Wall','-Wextra','-Wpedantic','-Werror','-pthread',*['-I'+str(x) for x in incs]]
 cases=[('b04_depth','b04',['memory'],'B4'),('b05_depth','b05',['verify','recover','copy','timer','response','grace'],'B5'),('b045_systems','systems',['paired','reduce','replace','wait','shoot','peer'],'S'),('modern_bridge','bridge',['once','hazard','epoch'],'MB')]
 for cc in ['gcc','clang']:
  run(cc+' costs compile',[cc,*flags,REF/'costs.c','-o',work/(cc+'-costs')]);run(cc+' costs',[work/(cc+'-costs')],marker='COST struct sx_once=')
  for family,provider,kinds,prefix in cases:
   for kind in kinds:
    for variant in ([1,2] if prefix=='MB' else [0]):
     define=prefix+'_MUTATE_'+kind.upper()+(str(variant) if variant else '');exe=work/(cc+'-'+define)
     if run(cc+' '+define+' compile',[cc,*flags,'-D'+define,providers[provider],ROOT/f'labs/{family}/tests/tests.c','-o',exe]):run(cc+' '+define+' reject',[exe,kind],1,'MB_CHECK:' if prefix=='MB' else 'DEPTH_CHECK:')
   stub=work/(cc+'-'+provider+'-stub')
   if run(cc+' '+provider+' stub compile',[cc,*flags,ROOT/'labs'/PUBLIC[provider],ROOT/f'labs/{family}/tests/tests.c','-o',stub]):
    for kind in kinds:run(cc+' '+provider+' stub '+kind,[stub,kind],1,'MB_CHECK:' if prefix=='MB' else 'DEPTH_CHECK:')
 run('independent modern finite expectations',[sys.executable,ROOT/'labs/modern_bridge/tests/verify_fixtures.py'],marker='"hazard_SC_orders": 10')
 rows.append(dict(label='source stable',passed=before=={p.relative_to(ROOT).as_posix():sha(p) for p in sources}));value=dict(passed=all(r['passed'] for r in rows),checks=rows,input_sha256=before,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},limits=['Finite original semantic controls and host ABI storage sizes; no target timing/mastery or universal proof.']);dest=ROOT/a.report;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_text(json.dumps(value,indent=2)+'\n');print(json.dumps({'passed':value['passed'],'checks':len(rows),'failed':[r['label'] for r in rows if not r['passed']]}));return int(not value['passed'])
if __name__=='__main__':raise SystemExit(main())
