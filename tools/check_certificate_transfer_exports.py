"""Actual original exercise exports with explicit, separately supplied author providers."""
import datetime,json,pathlib,subprocess,sys,tempfile
from common import ROOT,dump,sha
from register_certificate_transfer import BASE,IDS
sys.path.insert(0,str(ROOT))
from cmastery import exercises,learning

def main():
 selected={ident:learning.digest(exercises.exercise(ident)) for ident in IDS}
 specs={};inputs={};commands=[];exports=[];folder=ROOT/'reviews/v5/expansion/certificate_transfer'/('exports-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));folder.mkdir(parents=True)
 for id in IDS:
  brief='curriculum/v5/exercises/'+id+'.md';specs[id]=json.loads((ROOT/brief).read_text(encoding='utf8').split('```json\n')[1].split('\n```')[0]);d=specs[id]
  for n in [brief,*d['fixtures'],*d['shared_contracts']]:inputs[n]=sha(ROOT/n)
 for n in ['tools/check_certificate_transfer_exports.py','tools/register_certificate_transfer.py','cmastery/cli.py','cmastery/assessments.py','cmastery/exercises.py','assessor/certificate_transfer/stream.c','assessor/certificate_transfer/tree.c','assessor/match_transfer/match.c','assessor/algorithm_frontier/frontier.c','assessor/algorithm_tree/certificate.c','assessor/containers_depth/indexed.c','assessor/algorithm_lookup/lookup.c','assessor/algorithm_depth/matching.c']:inputs[n]=sha(ROOT/n)
 for p in (ROOT/'cmastery').glob('*.py'):inputs[p.relative_to(ROOT).as_posix()]=sha(p)
 def run(n,args,code=0,marker=None):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True,timeout=120);out=r.stdout+'\n'+r.stderr;log=folder/(n+'.log');log.write_text(out,encoding='utf8');ok=r.returncode==code and(marker is None or marker in out);commands.append(dict(id=n,command=list(map(str,args)),exit_code=r.returncode,expected_exit=code,required_marker=marker,passed=ok,output=log.relative_to(ROOT).as_posix(),output_sha256=sha(log)));assert ok,(n,out)
 with tempfile.TemporaryDirectory(prefix='c-mastery-T36-') as temporary:
  for id in IDS:
   workspace=pathlib.Path(temporary)/id;d=specs[id];run(id+'.init',[sys.executable,'-m','cmastery','init',d['project'],workspace,'--exercise',id]);assets={}
   for n in d['fixtures']+d['shared_contracts']:assert (workspace/n).is_file() and sha(workspace/n)==inputs[n],n;assets[n]=inputs[n]
   assert not (workspace/'assessor').exists()
   match=id.endswith('003');lab='match_transfer' if match else 'certificate_transfer';providers=['assessor/algorithm_lookup/lookup.c','assessor/algorithm_depth/matching.c'] if match else ['assessor/algorithm_frontier/frontier.c','assessor/algorithm_tree/certificate.c','assessor/containers_depth/indexed.c']
   includes=['labs/algorithm_lookup','labs/algorithm_depth','labs/containers_depth'] if match else ['labs/algorithm_frontier','labs/algorithm_tree','labs/algorithm_depth','labs/containers_depth','labs/algorithms/include']
   command=[sys.executable,workspace/'labs'/lab/'run.py']+sum((['--provider',ROOT/p] for p in providers),[])+sum((['--include',workspace/p] for p in includes),[])
   run(id+'.stub',command,41,'MT_ASSERT:' if match else 'CT_ASSERT:');run(id+'.reference',command+['--implementation',ROOT/'assessor'/lab],0,'MT_PASS' if match else 'CT_PASS');run(id+'.missing',command+['--provider',workspace/'absent.c'],2,'missing explicit provider/implementation')
   exports.append(dict(id=id,assets=assets,private_sources_copied=False,explicit_author_providers=providers,learner_performance='not-assessed'))
 assert all(sha(ROOT/n)==h for n,h in inputs.items())
 assert all(learning.digest(exercises.exercise(i))==h for i,h in selected.items())
 report=dict(selected_catalog_sha256=selected,passed=True,status='pass',commands=commands,source_sha256=inputs,exports=exports,limits=['Actual isolated public packets and explicit external author providers; compiling stubs reject and missing prerequisites reject.','Original independent learner/delayed/external review/physical dimensions remain pending.']);dump(ROOT/BASE/'exports.json',report);print(dict(passed=True,commands=len(commands),exercises=len(exports)))
if __name__=='__main__':main()
