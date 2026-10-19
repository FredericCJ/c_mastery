"""Reconcile raw T55 receipts and rerun the five frozen provider guards."""
import argparse, hashlib, json, re, statistics, subprocess, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1];sys.path[:0]=[str(ROOT),str(ROOT/'tools')]
from research.v5.expansion.legacy_concurrency_followthrough.criterion_notes import NOTES
from measure_concurrency_followthrough import raw,distribution
FAMILIES=('spsc','identity','reclamation','grace','history')
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def load(p):return json.loads(p.read_text())
def pins(mapping):
 for p,h in mapping.items():assert sha(ROOT/p)==h,p
def inspect(m,t,e):
 checks=[]
 def check(name,value):
  assert value,name
  checks.append(dict(name=name,passed=True))
 for label,d in [('mechanisms',m),('measurements',t),('exports',e)]:
  check(label+' passed',d['passed'] and d['error'] is None);pins(d['source_sha256']);pins(d['artifacts'])
 for row in m['rows']+e['rows']:
  text=(ROOT/row['log']).read_text();check(row['name'],row['passed'] and row['returncode']==row['expected'] and sha(ROOT/row['log'])==row['sha256'] and (row['marker'] is None or row['marker'] in text) and 'runtime error:' not in text and 'ERROR: AddressSanitizer' not in text and 'WARNING: ThreadSanitizer' not in text)
 check('mechanism and export cardinality',len(m['rows'])==311 and len(e['rows'])==180 and len(e['packages'])==10)
 for row in t['commands']:
  text=(ROOT/row['log']).read_text();check(row['name'],row['passed'] and row['returncode']==0 and sha(ROOT/row['log'])==row['sha256'] and not any(x in text for x in ('runtime error:','ERROR: AddressSanitizer','WARNING: ThreadSanitizer')))
 check('measurement cardinality',len(t['commands'])==357 and len(t['observations'])==340 and len(t['summaries'])==64 and len(t['comparisons'])==8)
 rows={x['name']:x for x in t['observations']}
 check('unique observations',len(rows)==340 and sum(x['instrumented'] for x in rows.values())==20)
 for row in rows.values():
  path=ROOT/row['raw'];check(row['name']+' raw ledger',sha(path)==row['raw_sha256'] and raw(path,row['metadata'])==row['distribution'])
 for group in t['summaries']:
  selected=[rows[n] for n in group['runs']];check(str(group['key'])+' native summary',len(selected)==5 and {x['repeat'] for x in selected}==set(range(5)) and all(not x['instrumented'] and [x['compiler'],x['parameters'],x['metadata'].get('padded')]==group['key'] for x in selected) and distribution([x['metadata']['elapsed_ns'] for x in selected])==group['elapsed_ns'])
 for row in t['comparisons']:
  paired={kind:sorted([x for x in rows.values() if not x['instrumented'] and x['compiler']==row['compiler'] and x['parameters']==['queue',kind,4,row['payload_bytes'],row['allowed_cpus'],0,10000]],key=lambda x:x['repeat']) for kind in (1,3)}
  ratios=[a['metadata']['elapsed_ns']/b['metadata']['elapsed_ns'] for a,b in zip(paired[1],paired[3])]
  check(str((row['compiler'],row['payload_bytes'],row['allowed_cpus']))+' observed paired comparison',len(ratios)==5 and row['AP_over_mutex_paired_ratios']==ratios and row['median_ratio']==statistics.median(ratios) and row['locking_faster_in_median']==(statistics.median(ratios)>1) and row['threads']==8 and row['oversubscribed'])
 for cc in ('gcc','clang'):check(cc+' observed mutex faster case',any(x['compiler']==cc and x['locking_faster_in_median'] for x in t['comparisons']))
 base=(ROOT/m['rows'][0]['log']).parent;models=load(base/'models.json');check('ten explicit premise models',models['passed'] and len(models['checks'])==10)
 for cc in ('gcc','clang'):
  oracle=load(base/(cc+'-oracle/result.json'));check(cc+' finite independent history oracle',oracle['passed'] and oracle['cases']==67500 and oracle['accepted']==36768 and oracle['rejected']==30732 and oracle['specifications']==['FIFO','STACK'] and oracle['pending_masks']==[0,1,3] and oracle['event_schedules']==90 and sorted(x['calls'] for x in oracle['actual_protocol_histories'])==[3,4] and oracle['input_sha256']==sha(base/(cc+'-oracle/input.txt')))
 commands={x['name'] for x in m['rows']}
 for objective,note in NOTES.items():
  selectors=note[3];check(objective+' exact selectors',set(selectors['mechanisms'])<=commands and set(selectors['models'])<=models['checks'].keys() and set(selectors['providers'])<=set(FAMILIES))
  for selector in selectors['measurements']:check(objective+' measurement '+str(selector),all(any(not x['instrumented'] and x['compiler']==cc and all(x['metadata'].get(k)==v for k,v in selector.items()) for x in rows.values()) for cc in ('gcc','clang')))
 for name in ('advanced','concurrent'):
  old=ROOT/f'labs/execution/src/{name}.c';new=ROOT/f'labs/concurrency_followthrough/retained/{name}.c';text=new.read_text();original=old.read_text();normalized=re.sub(r'\(void\)a\d+;','',text[text.index('*/')+2:]);check(name+' incomplete companion changes only unused parameters',sha(old) in text and re.sub(r'\s+','',normalized)==re.sub(r'\s+','',original))
 check('four actual selected M4 links',len(m['checks'])==4 and all(x['passed'] for x in m['checks']))
 for cc in ('arm-none-eabi-gcc','clang'):
  for opt in ('O0','O3'):
   for suffix in ('imports','closed'):check(cc+opt+suffix+' empty',not (base/f'{cc}-{opt}-m4-{suffix}.txt').read_text().strip())
 return checks
def main():
 p=argparse.ArgumentParser();p.add_argument('--work',type=Path,required=True);p.add_argument('--mechanisms',type=Path,required=True);p.add_argument('--measurements',type=Path,required=True);p.add_argument('--exports',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);commands=[];checks=[];sources={Path(__file__).relative_to(ROOT).as_posix():sha(Path(__file__))};components={}
 try:
  for label,path in [('mechanisms',a.mechanisms),('measurements',a.measurements),('exports',a.exports)]:components[label]=dict(path=path.as_posix(),sha256=sha(path))
  checks=inspect(*(load(p) for p in (a.mechanisms,a.measurements,a.exports)))
  for family in FAMILIES:
   script=f'tools/check_advanced_{family}.py';command=[sys.executable,script,'--self-test'];r=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=180);path=work/(family+'.txt');path.write_text(r.stdout+r.stderr);d=json.loads(r.stdout);okay=r.returncode==0 and d['passed'] and not d['errors'] and all(x['rejected'] for x in d['negative_controls']);commands.append(dict(name=family,command=command,returncode=r.returncode,passed=okay,log=path.relative_to(ROOT).as_posix(),sha256=sha(path),negative_controls=len(d['negative_controls'])));assert okay,family
   directory=ROOT/f'research/v5/expansion/legacy_advanced_{family}'
   for f in [ROOT/script,*[directory/x for x in ('validation.json','resources.json','research_gate.json','exports.json','completion_dispositions.json')]]:sources[f.relative_to(ROOT).as_posix()]=sha(f)
   print(family,'PASS',flush=True)
  error=None;passed=True
 except Exception as exc:passed=False;error=repr(exc)
 report=dict(passed=passed,error=error,checks=checks,commands=commands,components=components,source_sha256=sources,artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Exact raw receipt reconciliation and current five frozen provider guards, without new human or physical evidence.')
 (work/'result.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,checks=len(checks),provider_guards=len(commands))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
