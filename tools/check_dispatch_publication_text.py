"""Check versioned text correction, actual current campaigns and learner exports."""
from pathlib import Path
import datetime,hashlib,json,subprocess,sys
from check_expansion_campaigns import pin_errors,execution_errors,generic_errors
ROOT=Path(__file__).resolve().parents[1];BASE=ROOT/'research/v5/modern/history/dispatch_encoding_20261018'
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads((ROOT/p).read_text(encoding='utf8'))
def main():
 correction=json.loads((BASE/'correction.json').read_text(encoding='utf8'));provider=correction['provider'];assert sha(ROOT/provider)==correction['corrected_sha256'];old=(ROOT/correction['snapshots'][provider]['path']).read_bytes().decode('utf8');current=(ROOT/provider).read_bytes().decode('utf8')
 for row in correction['changes']:assert old.count(row['before'])==1;old=old.replace(row['before'],row['after']);assert row['after'] in current
 assert old==current
 for item in correction['snapshots'].values():assert sha(ROOT/item['path'])==item['sha256']
 report='reviews/v5/expansion/modern_completion/dispatch_campaign.json';r=read(report);errors=pin_errors(r)+execution_errors(r)+generic_errors('dispatch',read('labs/dispatch/family.json'),r);assert r['passed'] and not errors,errors
 rows=[];work=ROOT/'build/dispatch-publication-exports'/datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');work.mkdir(parents=True)
 # The edition manifest selects the catalog location, never assume it.
 sys.path.insert(0,str(ROOT));from cmastery import exercises,learning
 selected=[r for r in exercises.catalog()['exercises'].values() if r['id'].startswith('X-T69-') or r['id'] in ['X-T70-002','X-T66-010']]
 def run(label,cmd,expected=0,marker=None):
  cmd=list(map(str,cmd));p=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=300);ok=p.returncode==expected and (not marker or marker in p.stdout+p.stderr);rows.append(dict(label=label,command=cmd,returncode=p.returncode,expected=expected,marker=marker,passed=bool(ok),stdout=p.stdout,stderr=p.stderr));(work/'commands.json').write_text(json.dumps(rows,indent=2)+'\n');assert ok,(label,p.stdout,p.stderr)
 for item in selected:
  dest=work/item['id'];run(item['id']+' init',[sys.executable,'-m','cmastery','init',item['project'],dest,'--exercise',item['id']]);exercises.workspace_contract(dest,item['id']);assert not any('assessor' in p.relative_to(dest).parts for p in dest.rglob('*'))
  if item['lab']['family']=='dispatch':assert sha(dest/'CONTRACTS.md')==correction['corrected_sha256']
  assert all(sha(dest/p)==sha(ROOT/p) for p in item['shared_contracts'])
 # Actual public capture compilation and execution with explicit external
 # predecessor implementations; the learner provider still rejects.
 dest=work/'X-T69-005'
 for reference in [False,True]:
  build=work/('reference' if reference else 'learner');cmd=['cmake','-S',dest,'-B',build,'-G','Ninja','-DCMASTERY_DOD_ROOT='+str(ROOT/'labs/dod'),'-DCMASTERY_OWNERSHIP_ROOT='+str(ROOT/'labs/ownership')]
  if reference:cmd+=['-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor')]
  run(str(reference)+' configure',cmd);run(str(reference)+' build',['cmake','--build',build,'--target','dispatch-capture','--parallel','4']);run(str(reference)+' execute',[build/'dispatch-capture','capture_codec'],0 if reference else 1,None if reference else 'capture')
 run('authority guard',[sys.executable,ROOT/'tools/check_ownership_authority_boundary.py','--self-test'],marker='"passed": true')
 artifacts=[(learning.catalog_dir()/'exercises.json').relative_to(ROOT).as_posix(),'research/v5/modern/history/dispatch_encoding_20261018/correction.json',provider,report,'research/v5/modern/ownership_authority_boundary_resources.json','research/v5/modern/ownership_authority_boundary_research_gate.json','research/v5/modern/ownership_authority_boundary_validation.json','research/v5/modern/ownership_authority_boundary_investigation_validation.json','research/v5/modern/ownership_authority_boundary_readiness.json','tools/check_dispatch_publication_text.py','tools/correct_dispatch_publication_text.py','cmastery/cli.py','cmastery/exercises.py','cmastery/assessments.py']
 artifacts+=list(r['inspected_paths']);artifacts+=list(read('research/v5/modern/ownership_authority_boundary_readiness.json')['input_sha256']);artifacts+=['curriculum/v5/exercises/'+item['id']+'.md' for item in selected]
 out=dict(passed=True,correction='research/v5/modern/history/dispatch_encoding_20261018/correction.json',commands=rows,export_count=len(selected),input_sha256={p:sha(ROOT/p) for p in sorted(set(artifacts))},scope='Eight teaching lines/13 ranges; original semantics unchanged, actual source-bound dispatch and authority campaigns, 13 affected exercise exports plus separate higher_order exercise control and actual capture provider/stub executions. No learner/external/physical acceptance.')
 (ROOT/'reviews/v5/expansion/modern_completion/dispatch_publication_correction.json').write_text(json.dumps(out,indent=2)+'\n');print(dict(passed=True,exports=len(selected),commands=len(rows)))
if __name__=='__main__':main()
