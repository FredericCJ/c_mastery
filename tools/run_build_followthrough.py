"""Source-bound T33/T34 author campaign; tool failures never become proof success."""
import argparse,hashlib,importlib.util,json,os,re,shutil,subprocess,sys,tempfile
from datetime import datetime,timezone
from pathlib import Path
from statistics import median
from c_source import replace_unique,tokenize
def replace_in_comments(text,pattern,replacement,expected):
 # ACSL annotations live in comments, which are not tokens for c_source matching;
 # match layout-insensitively inside each comment and require the exact count.
 out=[];pos=0;count=0;rx=re.compile(r'\s*'.join(re.escape(t) for t in pattern.split()))
 for t in tokenize(text):
  if t.kind=='comment':
   hits=rx.findall(t.text)
   if hits:
    if len(hits)!=1:raise ValueError('ACSL anchor matched %d times in one comment: %s'%(len(hits),pattern))
    out+=[text[pos:t.start],rx.sub(lambda _:replacement,t.text)];pos=t.end;count+=1
 if count!=expected:raise ValueError('ACSL anchor matched %d comments, expected %d: %s'%(count,expected,pattern))
 return ''.join(out)+text[pos:]
ROOT=Path(__file__).resolve().parents[1];LAB=ROOT/'labs/build_followthrough';REF=ROOT/'assessor/build_followthrough';BASE=ROOT/'reviews/v5/expansion/legacy_build_followthrough'
PROFILES=[('gcc-debug','gcc',['-O0']),('gcc-release','gcc',['-O3']),('gcc-sanitize','gcc',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer']),('clang-debug','clang',['-O0']),('clang-release','clang',['-O3']),('clang-sanitize','clang',['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer'])]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def sources():return sorted([Path(__file__).resolve(),*[p for base in (LAB,REF) for p in base.rglob('*') if p.is_file() and '__pycache__' not in p.parts]])
def semantic(out,code,kind,positive):
 if code!=(0 if positive else 1) or (('BF_' if kind=='formal' else 'BP_')+('PASS:' if positive else 'CHECK:')) not in out or any(x in out for x in ('runtime error:','ERROR: AddressSanitizer','Segmentation fault')):raise ValueError('Wrong semantic control: '+out[:1000])
def main(report):
 target=(ROOT/report).resolve()
 if not target.is_relative_to(BASE.resolve()):raise ValueError('Report outside package')
 logs=BASE/'runs'/datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ');logs.mkdir(parents=True)
 v={'schema_version':1,'created_utc':datetime.now(timezone.utc).isoformat(),'inspected_paths':{p.relative_to(ROOT).as_posix():sha(p) for p in sources()},'commands':[],'observations':[],'open_gates':[]}
 env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:abort_on_error=1',UBSAN_OPTIONS='halt_on_error=1')
 env['PATH']=str(Path.home()/'cmastery-b06-tools/usr/bin')+os.pathsep+env['PATH']
 def run(cmd,label,expect=0,marker=None,timeout=60):
  cmd=list(map(str,cmd))
  try:r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,env=env,timeout=timeout);out=r.stdout+r.stderr;code=r.returncode
  except (OSError,subprocess.TimeoutExpired) as e:out=str(e);code=-1
  p=logs/(str(len(v['commands'])).zfill(4)+'-'+label+'.log');p.write_text(out,encoding='utf-8')
  row={'label':label,'command':cmd,'exit_code':code,'log':p.relative_to(ROOT).as_posix()};v['commands'].append(row)
  if expect is not None and code!=expect or marker is not None and marker not in out:raise ValueError(label+': '+out[-3000:])
  return out,row
 def save(p,label):
  dest=logs/label;shutil.copyfile(p,dest);return dest.relative_to(ROOT).as_posix()
 try:
  v['toolchains']={cc:run([cc,'--version'],cc+'-version')[0].splitlines()[0] for cc in ('gcc','clang')}
  spec=importlib.util.spec_from_file_location('bf_models',LAB/'models.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m);v['models']=m.analyze()
  with tempfile.TemporaryDirectory(prefix='build-followthrough-') as td:
   work=Path(td)
   common=['-std=c17','-Wall','-Wextra','-Werror','-pedantic','-I',str(LAB)]
   for profile,cc,flags in PROFILES:
    for kind,mutants in [('formal',[0,1,2,3,'stub']),('performance',[0,1,2,'stub'])]:
     for mutant in mutants:
      exe=work/(profile+'-'+kind+'-'+str(mutant));src=LAB if mutant=='stub' else REF;extra=[src/'simd.c'] if kind=='performance' else []
      cmd=[cc,*common,*flags,*([] if mutant=='stub' else ['-D'+('BF' if kind=='formal' else 'BP')+'_MUTANT='+str(mutant)]),src/(kind+'.c'),LAB/(kind+'_tests.c'),*extra,'-o',exe]
      _,b=run(cmd,exe.name+'-build');out,e=run([exe],exe.name,0 if mutant==0 else 1);semantic(out,e['exit_code'],kind,mutant==0)
      v['observations'].append({'kind':kind,'profile':profile,'mutant':mutant,'build':b,'execution':e,'passed':True})
   # Source/IR/object/link inspection; no report message substitutes for output tests.
   for cc in ('gcc','clang'):
    for kind,flags in [('strict',['-frounding-math','-ffp-contract=off']),('relaxed',['-fno-signed-zeros','-fno-trapping-math','-fassociative-math','-ffp-contract=fast'])]:
     exe=work/(cc+'-float-'+kind);run([cc,*common,'-O3',*flags,REF/'performance.c',REF/'simd.c',LAB/'floating.c','-lm','-o',exe],cc+'-float-'+kind+'-build');out,_=run([exe],cc+'-float-'+kind)
     if kind=='strict' and 'sum=3 ' not in out:raise ValueError('Strict floating contract changed')
   for cc in ('gcc','clang'):
    obj=work/(cc+'-core.o');flags=['-fopt-info-vec-all'] if cc=='gcc' else ['-Rpass=loop-vectorize','-Rpass-missed=loop-vectorize','-Rpass-analysis=loop-vectorize']
    run([cc,*common,'-O3',*flags,'-fstack-usage','-c',REF/'performance.c','-o',obj],cc+'-vector-report');save(obj,cc+'-core.o')
    run(['nm','-u',obj],cc+'-undefined');run(['objdump','-dr',obj],cc+'-object-disassembly');run(['nm','-S','--size-sort',obj],cc+'-symbol-sizes')
    strict=work/(cc+'-strict.o');run([cc,*common,'-O2','-ffreestanding','-fno-builtin','-c',REF/'performance.c','-o',strict],cc+'-strict-core')
    out,_=run(['nm','-u',strict],cc+'-strict-undefined')
    if {line.split()[-1] for line in out.splitlines()}-{'bf_sse2'}:raise ValueError('Unexpected strict core runtime dependency')
    save(strict,cc+'-strict.o')
    for su in work.glob('*.su'):save(su,cc+'-'+su.name)
    ir=work/(cc+'.ir');run([cc,*common,'-O3',*(['-S','-emit-llvm'] if cc=='clang' else ['-S','-fdump-tree-optimized']),REF/'performance.c','-o',ir],cc+'-ir');save(ir,cc+'.ir')
    for dump in work.glob('*.optimized'):save(dump,cc+'-'+dump.name)
    # Separate baseline/specialized translation units; minimum architecture is x86-64/SSE2.
    base=work/(cc+'-base.o');simd=work/(cc+'-simd.o');test=work/(cc+'-test.o');exe=work/(cc+'-separate')
    run([cc,*common,'-O2','-march=x86-64','-mno-avx','-fno-tree-vectorize','-c',REF/'performance.c','-o',base],cc+'-baseline-unit')
    run([cc,*common,'-O2','-msse2','-mno-avx','-c',REF/'simd.c','-o',simd],cc+'-specialized-unit')
    run([cc,*common,'-O2','-c',LAB/'performance_tests.c','-o',test],cc+'-test-unit');run([cc,base,simd,test,'-o',exe],cc+'-separate-link');run([exe],cc+'-separate-run',marker='BP_PASS:');save(exe,cc+'-separate.elf');run(['objdump','-d',exe],cc+'-linked-disassembly');run(['nm',exe],cc+'-linked-symbols')
   # GCC hosted plain/LTO/archive/PGO; collection and firmware runtime boundaries are separate.
   avx=work/'avx2.o';baseline=work/'avx-test.o';avxe=work/'avx-test'
   run(['gcc',*common,'-O2','-mavx2','-c',REF/'avx2.c','-o',avx],'avx2-specialization');run(['gcc',*common,'-O2','-march=x86-64','-mno-avx','-c',LAB/'avx2_test.c','-o',baseline],'avx2-baseline-detection');run(['gcc',avx,baseline,'-o',avxe],'avx2-link');out,row=run([avxe],'avx2-transfer',expect=None)
   if row['exit_code']==77:v['open_gates'].append({'gate':'AVX2 physical host execution','state':'unsupported','evidence':row})
   elif row['exit_code'] or 'AVX2_PASS:' not in out:raise ValueError('AVX2 transfer failed')
   save(avxe,'avx2.elf');run(['objdump','-d',avxe],'avx2-final-code')
   for mode,extra in [('plain',[]),('lto',['-flto'])]:
    elf=work/('firmware-'+mode+'.elf');mapfile=work/('firmware-'+mode+'.map')
    run(['clang','--target=arm-none-eabi','-mcpu=cortex-m4','-mthumb','-std=c17','-O2','-ffreestanding','-fno-builtin','-ffunction-sections','-fdata-sections','-fstack-usage',*extra,'-I',LAB,REF/'performance.c',LAB/'firmware.c',LAB/'startup.S','-nostdlib','-fuse-ld='+str(Path.home()/'cmastery-b06-tools/usr/bin/ld.lld-21'),'-Wl,--gc-sections,-Map='+str(mapfile),'-Wl,-T,'+str(LAB/'firmware.ld'),'-o',elf],mode+'-firmware-build')
    out,_=run(['nm','-u',elf],mode+'-firmware-undefined')
    if out.strip():raise ValueError('Unexpected firmware helper dependency')
    out,_=run(['nm',elf],mode+'-firmware-symbols')
    if '__gcov' in out:raise ValueError('Profile runtime in firmware')
    run(['size',elf],mode+'-firmware-size');run(['readelf','-h','-S','-s',elf],mode+'-firmware-elf');save(elf,'firmware-'+mode+'.elf');save(mapfile,'firmware-'+mode+'.map')
   for cc in ('gcc','clang'):
    extra=work/(cc+'-extra');run([cc,*common,'-O3',REF/'performance.c',REF/'simd.c',LAB/'profile_extra.c','-o',extra],cc+'-extra-build')
    out,row=run([extra],cc+'-clamp-locality-measure',marker='EXTRA_PASS:');v['observations'].append({'kind':'clamp-measure','compiler':cc,'execution':row,'passed':True});save(extra,cc+'-extra.elf')
   measure=[REF/'performance.c',REF/'simd.c',LAB/'measure.c'];bench=work/'bench';profile_dir=work/'profiles';profile_dir.mkdir()
   for mode,flags in [('plain',[]),('lto',['-flto']),('pgo-generate',['-fprofile-generate='+str(profile_dir)]),('pgo-use',['-fprofile-use='+str(profile_dir),'-Werror=coverage-mismatch','-Werror=missing-profile'])]:
    cmd=['/usr/bin/time','-f','BUILD elapsed=%e peak_kib=%M','gcc',*common,'-O3',*flags,*measure,'-o',bench]
    run(cmd,mode+'-build');save(bench,mode+'.elf');run(['size',bench],mode+'-size');run(['nm',bench],mode+'-symbols');run(['nm','-u',bench],mode+'-undefined');run(['objdump','-d',bench],mode+'-disassembly')
    for n in (8,4096,65536):
     out,row=run([bench,n,40,0 if mode=='pgo-generate' else 1],mode+'-measure-'+str(n));samples=[line.split(',') for line in out.splitlines() if line.startswith('sample,')]
     if len(samples)!=18:raise ValueError('Missing paired samples')
     for pair in range(9):
      ps=[r for r in samples if int(r[2])==pair]
      if len(ps)!=2 or ps[0][5]!=ps[1][5] or int(ps[0][5])!=m.measure_checksum(n,pair,40):raise ValueError('Unmatched independent semantic checksum')
     summary={str(path):{'median_ns':median([int(r[4]) for r in samples if int(r[3])==path]),'range_ns':[min(int(r[4]) for r in samples if int(r[3])==path),max(int(r[4]) for r in samples if int(r[3])==path)]} for path in (0,1)}
     v['observations'].append({'kind':'measurement','mode':mode,'n':n,'execution':row,'samples':samples,'summary':summary,'passed':True})
    if mode=='pgo-generate':
     for p in profile_dir.rglob('*.gcda'):save(p,'profile-'+p.name)
   # Changed-source mismatch at the same object/profile identity: restore exact source after run.
   changed=work/'changed';changed.mkdir();copies=[]
   for p in measure:q=changed/p.name;shutil.copyfile(p,q);copies.append(q)
   mismatch=work/'mismatch';mp=work/'mismatch-profile';mp.mkdir()
   run(['gcc',*common,'-O3','-fprofile-generate='+str(mp),*copies,'-o',mismatch],'mismatch-train-build');run([mismatch,8,10,0],'mismatch-train')
   text=copies[0].read_text();copies[0].write_text(replace_unique(text,'if(x<0)return 0;','if(x < -10)return 1;if(x<0)return 0;'))
   out,row=run(['gcc',*common,'-O3','-fprofile-use='+str(mp),'-Werror=coverage-mismatch',*copies,'-o',mismatch],'mismatch-diagnostic',expect=None)
   if row['exit_code']==0 or 'coverage-mismatch' not in out:raise ValueError('Missing intended PGO mismatch')
   # Archive plugin participation is retained through compile/archive/final-link commands.
   objs=[]
   for source in (REF/'performance.c',REF/'simd.c'):
    obj=work/(source.stem+'-lto.o');run(['gcc',*common,'-O3','-flto','-c',source,'-o',obj],'archive-'+source.stem);objs.append(obj)
   archive=work/'libkernel.a';run(['gcc-ar','rcs',archive,*objs],'archive-create');exe=work/'archive-test';run(['gcc',*common,'-O3','-flto',LAB/'performance_tests.c',archive,'-Wl,-Map='+str(work/'archive.map'),'-o',exe],'archive-link');run([exe],'archive-run',marker='BP_PASS:');save(work/'archive.map','archive.map');save(exe,'archive.elf');run(['objdump','-d',exe],'archive-linked-code')
   cbmc=Path.home()/'cmastery-b06-tools/usr/bin/cbmc';run([cbmc,'--version'],'cbmc-version')
   base=[cbmc,LAB/'sum_cbmc.c',REF/'formal.c','-I',LAB,'--bounds-check','--pointer-check','--signed-overflow-check','--unwinding-assertions']
   run([*base,'--show-loops'],'cbmc-loop-ids')
   for unwind,code in [(4,10),(5,0)]:run([*base,'--unwind',unwind,'--trace'],'cbmc-unwind-'+str(unwind),code,'VERIFICATION FAILED' if code else 'VERIFICATION SUCCESSFUL')
   run([*base,'--unwind',5,'-DBF_MUTANT=1','--trace'],'cbmc-double-sum',10,'VERIFICATION FAILED')
   run([*base,'--unwind',5,'-DBF_MUTANT=2','--trace'],'cbmc-frame',10,'VERIFICATION FAILED')
   # Negated witness assertions avoid CBMC6.11's crashing --cover/cover path.
   # A counterexample to n!=k supplies a reachable n=k; vacuous assumptions remove it.
   for witness in (0,4):
    run([*base,'--unwind','5','--trace','-DBF_WITNESS='+str(witness)],'reachable-witness-'+str(witness),10,'VERIFICATION FAILED')
   run([*base,'--unwind','5','--trace','-DBF_WITNESS=0','-DVACUOUS'],'vacuous-witness',0,'VERIFICATION SUCCESSFUL')
   run([cbmc,LAB/'environment_cbmc.c',REF/'formal.c','-I',LAB,'--unwind','4','--unwinding-assertions','--bounds-check','--pointer-check','--trace'],'cbmc-environment',0,'VERIFICATION SUCCESSFUL')
   run([cbmc,LAB/'nested_cbmc.c','--show-loops'],'nested-loop-ids');run([cbmc,LAB/'nested_cbmc.c','--unwind','5','--unwinding-assertions'],'nested-complete',0,'VERIFICATION SUCCESSFUL')
   frama=os.environ.get('FRAMAC') or shutil.which('frama-c') or str(Path.home()/'cmastery-formal-tools/bundle/bin/frama-c')
   if frama:
    run([frama,'-version'],'framac-version');run([frama,'-wp-list-provers'],'wp-provers')
    wp=work/'wp.json';out,_=run([frama,'-wp','-wp-rte','-wp-prover','qed,z3','-wp-cache','none','-wp-print','-wp-deprecated-report-json='+str(wp),'-machdep','x86_64',REF/'proof.c'],'wp-goals',timeout=60);save(wp,'wp-goals.json')
    if '43 / 44' not in out or 'typed_false_bound_ensures' not in out:raise ValueError('WP goal inventory changed; inspect before acceptance')
    widened=work/'widened.c';widened.write_text(replace_in_comments((REF/'proof.c').read_text(),'a[k] <= 10','a[k] <= 4294967295',2))
    run([frama,'-wp','-wp-rte','-wp-prover','qed,z3','-wp-cache','none','-wp-fct','proved_sum','-machdep','x86_64',widened],'wp-widened-range',timeout=60);save(widened,'widened.c')
    for lower in ('-1','0'):run([frama,'-eva','-machdep','x86_64','-cpp-extra-args=-DEVA_MIN='+lower,LAB/'eva.c'],'eva-'+lower)
   else:v['open_gates'].append({'gate':'Frama-C WP/Eva actual execution','state':'unavailable','commands':[['frama-c','-wp','-wp-rte','-machdep','x86_64',str(LAB/'sum_wp.c')],['frama-c','-eva',str(LAB/'eva.c')]]})
   perf=shutil.which('perf') or str(Path.home()/'cmastery-formal-tools/perf/root/usr/bin/perf')
   sampler=work/'sampled';run(['gcc',*common,'-O2','-g','-pg','-fno-omit-frame-pointer',*measure,'-o',sampler],'gprof-build');save(sampler,'gprof.elf');env['GMON_OUT_PREFIX']=str(work/'gmon')
   run([sampler,65536,4000,1],'gprof-workload',timeout=60);gmon=list(work.glob('gmon.*'))
   if len(gmon)!=1:raise ValueError('Missing gprof raw samples')
   save(gmon[0],'gmon.raw');run(['gprof','-b',sampler,gmon[0]],'gprof-report');run(['nm','-n',sampler],'gprof-addresses');run(['objdump','-d',sampler],'gprof-code');env.pop('GMON_OUT_PREFIX',None)
   if perf:
    env['LD_LIBRARY_PATH']=str(Path.home()/'cmastery-formal-tools/perf/root/usr/lib/x86_64-linux-gnu')
    run([perf,'--version'],'perf-version')
    out,row=run([perf,'stat','-e','cycles,cache-misses','--',bench,65536,20,1],'perf-counter',expect=None)
    if row['exit_code']:v['open_gates'].append({'gate':'hardware PMU counter','state':'unsupported-or-denied','evidence':row})
    out,row=run([perf,'record','-o',work/'perf.data','-e','cpu-clock','--',bench,65536,500,1],'perf-sampling',expect=None)
    if row['exit_code']:v['open_gates'].append({'gate':'actual supported sampler','state':'unsupported-or-denied','evidence':row})
    else:
     save(work/'perf.data','perf.data');run([perf,'report','--stdio','-i',work/'perf.data','--sort','symbol,dso'],'perf-report');run([perf,'script','-i',work/'perf.data','-F','ip,sym,dso'],'perf-address-samples')
   else:v['open_gates'].append({'gate':'hardware counter and supported sampler','state':'perf executable unavailable'})
 except (ValueError,OSError,subprocess.SubprocessError) as e:v['error']=str(e)
 v['source_stable']=all(sha(ROOT/p)==h for p,h in v['inspected_paths'].items());v['passed']='error' not in v and v['source_stable'];v['log_sha256']={p.relative_to(ROOT).as_posix():sha(p) for p in logs.rglob('*') if p.is_file()}
 for p in (target,logs/'campaign.json'):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,indent=2)+'\n',encoding='utf-8')
 print(json.dumps({'passed':v['passed'],'commands':len(v['commands']),'open_gates':v['open_gates'],'error':v.get('error')}));return 0 if v['passed'] else 1
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--report',required=True);raise SystemExit(main(p.parse_args().report))
