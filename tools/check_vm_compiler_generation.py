"""Run the actual T46 author twice in isolation; preserve current receipt bytes."""
import copy,datetime,hashlib,json,pathlib,runpy,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];HERE=ROOT/'research/v5/expansion/legacy_vm_compiler';AUTHOR=ROOT/'research/v5/expansion/legacy_diagnostics/author.py'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def generate(with_hook=True):
 if with_hook:scope=runpy.run_path(str(AUTHOR))
 else:
  text=AUTHOR.read_text(encoding='utf8');start=text.index("        if domain == 'T46':");end=text.index('        dump(base/',start);scope={'__file__':str(AUTHOR),'__name__':'isolated_negative_control'};exec(compile(text[:start]+text[end:],str(AUTHOR),'exec'),scope)
 records={};analyses={};fn=scope['author'];g=fn.__globals__;g['dump']=lambda p,d:records.setdefault(pathlib.Path(p).relative_to(ROOT).as_posix(),copy.deepcopy(d));g['write']=lambda p,s:analyses.setdefault(pathlib.Path(p).relative_to(ROOT).as_posix(),s)
 fn('T46',(AUTHOR.parent/'T46.cards').read_text(encoding='utf8'));return records,analyses
def errors(records):
 out=[]
 for n,new in records.items():
  old=json.loads((ROOT/n).read_text(encoding='utf8'))
  for k in ['criteria','baseline_binding','shared_contracts','source_records','execution_support','additional_successor_support']:
   if old.get(k)!=new.get(k):out.append(n+' '+k)
  if set(old['supplied_artifacts'])!=set(new['supplied_artifacts']):out.append(n+' supplied assets')
 return out
def main():
 folder=ROOT/'build'/('vm-generation-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True);first,a=generate();second,b=generate();assert len(first)==12 and not errors(first);assert first==second and a==b
 removed,_=generate(False);assert errors(removed);broken=copy.deepcopy(first);broken[next(iter(broken))]['criteria'].pop();assert errors(broken)
 outputs={}
 for name,value in [('first.json',first),('second.json',second),('analyses.json',a)]:
  p=folder/name;p.write_text(json.dumps(value,indent=2)+'\n',encoding='utf8');outputs[p.relative_to(ROOT).as_posix()]=sha(p)
 paths={AUTHOR,pathlib.Path(__file__),AUTHOR.parent/'T46.cards',ROOT/'research/v5/expansion/legacy_vm_representation/author_support.py',*[ROOT/n for n in first]}
 paths|={p for p in (ROOT/'research/v5/expansion').glob('legacy_vm_*/attachment*.json')};paths|={ROOT/r['source_register'] for p in list(paths) if p.name.startswith('attachment') for r in [json.loads(p.read_text(encoding='utf8'))]}
 commands=[dict(id=n,command=[sys.executable,str(pathlib.Path(__file__).relative_to(ROOT))],operation=n,execution='in-process check within this actual generator-validation invocation',passed=True) for n in ['actual-first-pass','original-bindings-and-attachments','actual-second-pass-identical','removed-hook-rejected','lost-criterion-rejected']]
 out=dict(status='pass',commands=commands,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sorted(paths)},outputs_sha256=outputs,limits=['Actual generator function executed with isolated output sinks; no frozen canonical/assessor bytes rewritten.','PR-T46-02 supplied-artifact ordering differs from historical publication order, but exact asset set, criteria, baseline and all attachment metadata are preserved. Two newly generated outputs are identical.','No new C execution, independent review or learner evidence is created by regeneration.'])
 (HERE/'generator_validation.json').write_text(json.dumps(out,indent=2)+'\n',encoding='utf8');print('PASS five actual generation/preservation controls,12 T46 projects')
if __name__=='__main__':main()
