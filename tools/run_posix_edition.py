"""Execute the bounded source-edition classifier; never induce kernel close faults."""
import datetime,json,pathlib,sys
import run_thread_lifecycle as harness
ROOT=harness.ROOT;PUBLIC=ROOT/'labs/posix_edition';PRIVATE=ROOT/'assessor/posix_edition'
HERE=ROOT/'research/v5/expansion/source_reconciliation'
BUILD=ROOT/'build'/('posix-edition-'+datetime.datetime.now().strftime('%Y%m%d%H%M%S'))
PROFILES=[('debug',['-O0','-g']),('release',['-O3','-DNDEBUG']),('sanitized',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
MUTANTS=[('RETRY','close'),('RETAIN','close'),('ZERO','close'),('PRIORITY','read'),('FAIR','read')]
def pins():
 return {p.relative_to(ROOT).as_posix():harness.sha(p) for p in sorted([pathlib.Path(__file__),ROOT/'tools/run_thread_lifecycle.py',*[p for tree in [PUBLIC,PRIVATE] for p in tree.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])}
def main():
 BUILD.mkdir(parents=True);HERE.mkdir(parents=True,exist_ok=True);before=pins();harness.BUILD=BUILD;harness.COMMANDS.clear();harness.OUTPUTS.clear();error=None
 def build(tag,cc,flags,source):
  out=BUILD/tag;harness.run(tag+'.build',[cc,'-std=c17','-Wall','-Wextra','-Werror','-Wpedantic',*flags,'-I',PUBLIC,source,PUBLIC/'tests.c','-o',out]);harness.OUTPUTS.add(out);return out
 try:
  harness.run('kernel',['uname','-a'])
  for cc in ['gcc','clang']:
   harness.run(cc+'.version',[cc,'--version'])
   for mode,flags in PROFILES:
    tag=cc+'.'+mode;out=build(tag,cc,flags,PRIVATE/'edition.c')
    for case in ['close','read']:harness.run(tag+'.'+case,[out,case],marker='PE_'+case.upper())
    for mutant,case in MUTANTS:
     bad=build(tag+'.mutant.'+mutant,cc,[*flags,'-DPE_MUTANT_'+mutant],PRIVATE/'edition.c');harness.run(tag+'.reject.'+mutant,[bad,case],41,'PE_ASSERT:')
    stub=build(tag+'.stub',cc,flags,PUBLIC/'learner.c')
    for case in ['close','read']:harness.run(tag+'.stub.'+case,[stub,case],41,'PE_ASSERT:')
 except Exception as e:error=str(e)
 if pins()!=before:error='Inputs changed during campaign'
 report=dict(schema_version=1,status='pass' if error is None else 'fail',failure=error,source_sha256=pins(),commands=harness.COMMANDS,outputs_sha256={p.relative_to(ROOT).as_posix():harness.sha(p) for p in sorted(harness.OUTPUTS)},limits=['Pure selected-contract model, no actual POSIX2024 kernel or signal-interrupted close execution.','No scheduler fairness or arbitrary mixed-policy admission result.','Compiled semantic mutants differ from diagnostic-only negative fixtures.','Author model execution is not independent review, delayed learner performance or physical evidence.'])
 (HERE/'posix_validation.json').write_text(json.dumps(report,indent=2)+'\n');print(json.dumps(dict(status=report['status'],commands=len(harness.COMMANDS),failure=error)));return bool(error)
if __name__=='__main__':raise SystemExit(main())
