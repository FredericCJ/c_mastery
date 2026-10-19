"""Record actual equal-contract suffix workloads and generated core dependencies."""
import argparse,csv,datetime,hashlib,io,json,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 parser=argparse.ArgumentParser();parser.add_argument('--report',required=True);args=parser.parse_args()
 out=ROOT/args.report;run=out.parent/'runs'/('measure-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));run.mkdir(parents=True)
 build=ROOT/'build'/run.name;build.mkdir(parents=True)
 sources=[ROOT/'labs/corpus_index'/n for n in ['index.h','benchmark.c','CONTRACTS.md','dependency-contract.md']]+sorted((ROOT/'assessor/corpus_index').glob('*.c'))+[pathlib.Path(__file__)]
 pins={str(p.relative_to(ROOT)).replace('\\','/'):sha(p) for p in sources};commands=[];samples=[];undefined={}
 def execute(name,args):
  r=subprocess.run(list(map(str,args)),cwd=ROOT,capture_output=True,text=True,timeout=180);p=run/(name+'.txt');p.write_text(r.stdout+'\n'+r.stderr);commands.append(dict(id=name,command=list(map(str,args)),exit_code=r.returncode,passed=r.returncode==0,log=p.relative_to(ROOT).as_posix(),log_sha256=sha(p)));assert r.returncode==0,(name,r.stderr);return r.stdout
 environment=execute('environment',['uname','-a']);cpu=execute('cpu',['lscpu'])
 private=sorted((ROOT/'assessor/corpus_index').glob('*.c'))
 for compiler in ['gcc','clang']:
  version=execute(compiler+'-version',[compiler,'--version'])
  binary=build/(compiler+'-benchmark');execute(compiler+'-build',[compiler,'-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror','-I',ROOT/'labs/corpus_index',ROOT/'labs/corpus_index/benchmark.c',*private,'-o',binary])
  raw=execute(compiler+'-measure',[binary]);rows=list(csv.DictReader(io.StringIO(raw)));assert len(rows)==81
  assert all(int(r['queries'])==20000 and int(r['object_bytes'])>0 and int(r['index_ns'])>0 and int(r['scan_ns'])>0 for r in rows)
  samples.append(dict(compiler=compiler,rows=rows,scope='Count-only exact four-byte queries; build cost recorded separately; no universal speedup claim.'))
  asm=execute(compiler+'-disassembly',['objdump','-d',binary]);assert '<ci_suffix_range>' in asm and 'ci_suffix_build' in asm
  for source in private:
   obj=build/(compiler+'-'+source.stem+'.o');execute(compiler+'-object-'+source.stem,[compiler,'-std=c17','-O3','-ffreestanding','-fno-builtin','-fstack-usage','-I',ROOT/'labs/corpus_index','-c',source,'-o',obj]);undefined[compiler+'/'+source.stem]=execute(compiler+'-symbols-'+source.stem,['nm','-u',obj]).strip().splitlines()
 for source in private:
  obj=build/('m4-'+source.stem+'.o');execute('m4-object-'+source.stem,['arm-none-eabi-gcc','-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fstack-usage','-I',ROOT/'labs/corpus_index','-c',source,'-o',obj]);undefined['m4/'+source.stem]=execute('m4-symbols-'+source.stem,['arm-none-eabi-nm','-u',obj]).strip().splitlines()
 allowed={'memcpy','__aeabi_uldivmod'}
 assert all(line.split()[-1] in allowed for values in undefined.values() for line in values)
 stable=all(sha(ROOT/n)==h for n,h in pins.items());assert stable
 outputs={p.relative_to(ROOT).as_posix():sha(p) for p in build.rglob('*') if p.is_file()}
 result=dict(schema_version=1,passed=True,status='passed',source_sha256=pins,outputs_sha256=outputs,commands=commands,samples=samples,undefined_symbols=undefined,environment=environment,cpu=cpu,source_stable=stable,limits=['Actual hosted timing samples and object audits only; no physical board performance.','Cortex-M4 objects are not a linked freestanding application. Listed implicit helpers remain target integration obligations; none is silently supplied.','No timing threshold is a correctness gate; retained calls and checksums limit dead-code elimination, not all experimental confounds.','Independent learner/reviewer evidence is not inferred.'])
 out.write_text(json.dumps(result,indent=2)+'\n');print('PASS suffix workload162 rows and core symbol/stack audit',len(commands),'commands')
if __name__=='__main__':main()
