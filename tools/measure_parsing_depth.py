"""Equal whole-match workloads plus actual object/helper and stack reports."""
import argparse,csv,datetime,hashlib,io,json,pathlib,subprocess
ROOT=pathlib.Path(__file__).resolve().parents[1]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);a=p.parse_args();out=ROOT/a.report
 run=out.parent/'runs'/('measure-'+datetime.datetime.now(datetime.timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));run.mkdir(parents=True);build=ROOT/'build'/run.name;build.mkdir(parents=True)
 private=sorted((ROOT/'assessor/parsing_depth').glob('*.c'))
 files=[ROOT/'labs/parsing_depth'/n for n in ['parsing.h','benchmark.c','CONTRACTS.md']]+private+[pathlib.Path(__file__)]
 pins={p.relative_to(ROOT).as_posix():sha(p) for p in files};commands=[];samples=[];undefined={}
 def execute(name,argv):
  r=subprocess.run(list(map(str,argv)),cwd=ROOT,capture_output=True,text=True,timeout=180);log=run/(name+'.txt');log.write_text(r.stdout+'\n'+r.stderr)
  commands.append(dict(id=name,command=list(map(str,argv)),exit_code=r.returncode,passed=r.returncode==0,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log)));assert r.returncode==0,(name,r.stderr);return r.stdout
 environment=execute('environment',['uname','-a']);cpu=execute('cpu',['lscpu'])
 for compiler in ['gcc','clang']:
  execute(compiler+'-version',[compiler,'--version']);binary=build/(compiler+'-benchmark')
  execute(compiler+'-build',[compiler,'-std=c17','-O3','-Wall','-Wextra','-Wpedantic','-Werror','-I',ROOT/'labs/parsing_depth',ROOT/'labs/parsing_depth/benchmark.c',*private,'-o',binary])
  raw=execute(compiler+'-measure',[binary]);rows=list(csv.DictReader(io.StringIO(raw)));assert len(rows)==81
  assert all(int(r['queries'])==1000 and int(r['nfa_ns'])>0 and int(r['dfa_ns'])>0 and int(r['nfa_states'])>0 and int(r['dfa_states'])>0 for r in rows)
  samples.append(dict(compiler=compiler,rows=rows,scope='Equal whole-input boolean matching, reused compiled representations; source construction/table-size ledger is separate from these match timings.'))
  asm=execute(compiler+'-disassembly',['objdump','-d',binary]);assert '<rx_match>' in asm and '<rx_dfa_match>' in asm
  for source in private:
   obj=build/(compiler+'-'+source.stem+'.o');execute(compiler+'-object-'+source.stem,[compiler,'-std=c17','-O3','-ffreestanding','-fno-builtin','-fstack-usage','-I',ROOT/'labs/parsing_depth','-c',source,'-o',obj]);undefined[compiler+'/'+source.stem]=execute(compiler+'-symbols-'+source.stem,['nm','-u',obj]).strip().splitlines()
 for source in private:
  obj=build/('m4-'+source.stem+'.o');execute('m4-object-'+source.stem,['arm-none-eabi-gcc','-std=c17','-O2','-mcpu=cortex-m4','-mthumb','-ffreestanding','-fno-builtin','-fstack-usage','-I',ROOT/'labs/parsing_depth','-c',source,'-o',obj]);undefined['m4/'+source.stem]=execute('m4-symbols-'+source.stem,['arm-none-eabi-nm','-u',obj]).strip().splitlines()
 allowed={'memcpy','memset','__aeabi_llsl','__aeabi_llsr','__aeabi_lasr'}
 assert all(line.split()[-1] in allowed for values in undefined.values() for line in values),undefined
 assert all(sha(ROOT/n)==h for n,h in pins.items())
 result=dict(schema_version=1,passed=True,source_sha256=pins,outputs_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in build.rglob('*') if p.is_file()},commands=commands,samples=samples,undefined_symbols=undefined,permitted_compiler_support=sorted(allowed),environment=environment,cpu=cpu,source_stable=True,limits=['Match timings exclude construction; state/table sizes and derivation obligations account for construction separately.','No universal DFA speedup, general parser throughput, physical-board behavior or complete freestanding link is claimed.','Object helper calls are explicitly reported; target integration must supply audited allowed support or remove it.','Timing has no correctness threshold; retained calls/checksums do not eliminate all experimental confounds.'])
 out.write_text(json.dumps(result,indent=2)+'\n');print('PASS162 raw match rows and object/stack audit',len(commands),'commands')
if __name__=='__main__':main()
