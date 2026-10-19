"""Execute the public companion in an isolated export with explicit provider choice."""
import datetime,json,pathlib,shutil,subprocess
from common import ROOT,sha
def main():
 build=ROOT/'build'/('posix-edition-export-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'));export=build/'learner';export.mkdir(parents=True)
 names=['edition.h','learner.c','tests.c','run.py','CONTRACTS.md'];commands=[]
 for name in names:shutil.copyfile(ROOT/'labs/posix_edition'/name,export/name)
 for cc in ['gcc','clang']:
  for impl,expected,marker in [('learner',1,'PE_ASSERT:'),('reference',0,'PE_CLOSE')]:
   cmd=['python3',str(export/'run.py'),'--cc',cc]
   if impl=='reference':cmd+=['--source',str(ROOT/'assessor/posix_edition/edition.c')]
   r=subprocess.run(cmd,cwd=export,text=True,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,timeout=30);commands.append(dict(command=cmd,cwd=str(export),exit=r.returncode,expected=expected,marker=marker,stdout=r.stdout,passed=r.returncode==expected and marker in r.stdout))
 sources=[pathlib.Path(__file__),*[ROOT/'labs/posix_edition'/n for n in names],ROOT/'assessor/posix_edition/edition.c'];report=dict(passed=all(r['passed'] for r in commands),commands=commands,export_files={p.relative_to(ROOT).as_posix():sha(p) for p in export.iterdir()},private_sources_copied=False,source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in sources},limits=['Public export has no private implementation; reference selected by explicit outside path.','Explicit standalone shared-contract workflow, not an invented CLI exercise identifier.'])
 (ROOT/'research/v5/expansion/source_reconciliation/exports.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(passed=report['passed'],commands=len(commands))));return not report['passed']
if __name__=='__main__':raise SystemExit(main())
