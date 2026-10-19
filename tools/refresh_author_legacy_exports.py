"""Actual exports and existing exact guards; preserve original C campaigns."""
import argparse,datetime,json,os,subprocess,sys
from pathlib import Path
from common import ROOT,sha,dump
BASE='reviews/v5/expansion/author_legacy_export_refresh'
PACKAGES=[('legacy_'+n,n,n) for n in ['capstone_firmware','capstone_linux','object_alias','object_atlas','object_buffer','object_cursor','object_clinic','object_clinic_contract','vm_compiler']]+[
 ('legacy_checkpoint_protocol','checkpoint_protocol','checkpoint_protocol_followthrough'),
 ('legacy_checkpoint_writer','checkpoint_writer','checkpoint_writer_followthrough'),
 ('legacy_language_headers','header_definitions','header_definitions')]
def prepare():
 path=ROOT/BASE/'manifest.json';assert not path.exists(),'Preserve prepared history'
 entries=[];history={}
 for folder,export,stem in PACKAGES:
  here='research/v5/expansion/'+folder;support=here+'/completion_dispositions.json';value=json.loads((ROOT/support).read_text());exporter='tools/check_'+export+'_exports.py';closer='tools/close_'+stem+'.py';guard='tools/check_'+stem+'.py'
  campaign=next(p for p in value['current_reports'] if p.endswith('/validation.json'))
  for n in {support,exporter,*value['current_reports']}:
   source=ROOT/n;dest=ROOT/BASE/'history'/n;dest.parent.mkdir(parents=True,exist_ok=True);dest.write_bytes(source.read_bytes());history[n]=sha(source)
  text=(ROOT/exporter).read_text();assert 'envelope_export_inputs' not in text
  lines=text.splitlines();index=next(i for i,s in enumerate(lines) if s.lstrip().startswith("(HERE/'exports.json').write_text"));indent=lines[index][:-len(lines[index].lstrip())]
  lines[index:index]=[indent+'from envelope_export_inputs import bind_project_exports',indent+'files={ROOT/n for n in pins};errors=[]',indent+"bind_project_exports(ROOT,files,[dict(q,returncode=q['exit_code']) for q in commands],errors)",indent+'assert not errors,errors',indent+'pins={rel(p):sha(p) for p in sorted(files)}']
  text='\n'.join(lines)+'\n';compile(text,exporter,'exec');(ROOT/exporter).write_text(text,encoding='utf8',newline='\n')
  entries.append(dict(folder=folder,support=support,exporter=exporter,closer=closer,guard=guard,campaign=campaign,campaign_sha256=sha(ROOT/campaign),rows=len(value['updates'])))
 dump(path,dict(packages=entries,history_sha256=history,scope='Actual selected project/public assets exported and existing mechanism guards rerun; no C campaign rewritten.'))
 print('PREPARED',len(entries))
def run():
 assert os.name!='nt','Use WSL for actual hosted/target exports'
 manifest=json.loads((ROOT/BASE/'manifest.json').read_text());folder=ROOT/BASE/'runs'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');folder.mkdir(parents=True)
 progress=ROOT/BASE/'progress.json';old=json.loads(progress.read_text()) if progress.exists() else dict(completed=[])
 if progress.exists():(folder/'previous-progress.json').write_bytes(progress.read_bytes())
 completed=old['completed']
 for p in manifest['packages']:
  assert sha(ROOT/p['campaign'])==p['campaign_sha256']
  if p['folder']=='legacy_object_clinic':
   # Its v2 successor deliberately preserves this old envelope/export pair.
   # Refresh the current overlay below; never convert the historical pair into
   # current evidence against a card containing the later clarification.
   retained=ROOT/'research/v5/expansion/legacy_object_clinic_contract/historical_completion_dispositions.json'
   assert sha(ROOT/p['support'])==sha(retained),'Historical clinic envelope changed'
   continue
  prior=next((x for x in completed if x['support']==p['support']),None)
  if prior:
   assert sha(ROOT/prior['support'])==prior['sha256'];continue
  steps=[[sys.executable,p['exporter']]]
  if p['folder']=='legacy_vm_compiler':steps.append([sys.executable,'tools/check_vm_compiler_generation.py'])
  steps += [[sys.executable,p['closer']],[sys.executable,p['guard'],'--self-test']]
  executions=[]
  for index,command in enumerate(steps):
   result=subprocess.run(command,cwd=ROOT,capture_output=True,text=True,timeout=1800);log=folder/(p['folder']+'-'+str(index)+'.log');log.write_text(result.stdout+'\n'+result.stderr,encoding='utf8');executions.append(dict(command=command,exit_code=result.returncode,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)))
   if result.returncode:
    dump(progress,dict(passed=False,completed=completed,failed=p,executions=executions));print(result.stdout+result.stderr,flush=True);return 1
  assert sha(ROOT/p['campaign'])==p['campaign_sha256']
  guard=json.loads(result.stdout);assert guard.get('passed') is True and guard['negative_controls'] and all(r['rejected'] for r in guard['negative_controls'])
  completed.append(dict(support=p['support'],sha256=sha(ROOT/p['support']),rows=p['rows'],campaign=p['campaign'],campaign_sha256=p['campaign_sha256'],campaign_unchanged=True,guard=executions[-1]['log'],executions=executions));dump(progress,dict(passed=True,completed=completed));print('REFRESH_PASS',p['folder'],flush=True)
 dump(ROOT/BASE/'refreshed_packages.json',dict(passed=True,packages=completed,source_sha256={p:sha(ROOT/p) for p in ['tools/refresh_author_legacy_exports.py','tools/envelope_export_inputs.py',BASE+'/manifest.json']},limits='Actual export and current metadata evidence only; original C execution and external dimensions preserved.'))
 return 0
if __name__=='__main__':
 parser=argparse.ArgumentParser();parser.add_argument('--prepare',action='store_true');a=parser.parse_args()
 if a.prepare:prepare()
 else:raise SystemExit(run())
