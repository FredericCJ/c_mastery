"""Validate only explicit T12 module claims; partial evidence cannot close T12."""
from copy import deepcopy
import argparse,json
from common import ROOT,read_json,dump,sha
MODULES={'format':{f'T12.09.{c}' for c in 'abc'},'unicode':{f'T12.{n:02}.{c}' for n in (5,6) for c in 'abc'},'bytes':{f'T12.{n:02}.{c}' for n in (3,4) for c in 'abc'},'allocation':{f'T12.07.{c}' for c in 'abc'},'conversion':{f'T12.08.{c}' for c in 'abc'},'headers':{f'T12.{n:02}.{c}' for n in (1,2) for c in 'abc'}}
MODULES.update(scanner={f'T12.10.{c}' for c in 'abc'},fileio={f'T12.11.{c}' for c in 'abc'},mathlab={f'T12.13.{c}' for c in 'abc'},sorting={f'T12.12.{c}' for c in 'abc'}|{'T12.16.a','T12.16.c'},picolib={'T12.16.b'})
MODULES.update(processlab={f'T12.14.{c}' for c in 'abc'},control={f'T12.15.{c}' for c in 'abc'})
def validate(d):
 errors=[];selected=d.get('selected_modules',[])
 if not selected or len(set(selected))!=len(selected) or not set(selected)<=MODULES.keys():return ['Unknown/duplicate/empty module scope']
 if d.get('passed') is not True or d.get('error') is not None:errors.append('Failed campaign')
 if d.get('topic_complete') is not False:errors.append('Partial campaign inflated to whole topic')
 expected={(o,v) for m in selected for o in MODULES[m] for v in (False,True)};obs=d.get('observations',[])
 if len(obs)!=len(expected) or {(x.get('objective'),x.get('variant')) for x in obs}!=expected:errors.append('Missing exact ordinary/changed observation')
 pins=d.get('input_sha256',{})
 if not pins:errors.append('Missing source pins')
 for path,h in pins.items():
  p=(ROOT/path).resolve()
  if not p.is_relative_to(ROOT.resolve()) or not p.is_file() or sha(p)!=h:errors.append('Stale input: '+path)
 artifacts=d.get('retained_artifacts',{})
 if not artifacts:errors.append('Missing retained build artifacts')
 for path,row in artifacts.items():
  p=(ROOT/path).resolve()
  if not p.is_relative_to((ROOT/'build/legacy_library').resolve()) or not p.is_file() or p.stat().st_size!=row.get('bytes') or sha(p)!=row.get('sha256'):errors.append('Changed retained artifact: '+path)
 commands=d.get('commands',[]);checks=d.get('checks',[])
 for row in commands:
  rc=row.get('exit');e=row.get('expected');ok=isinstance(rc,int) and rc>0 if e=='nonzero' else rc==e
  if row.get('diagnostic') and row['diagnostic'] not in row.get('stdout','')+row.get('stderr',''):ok=False
  if not ok or row.get('passed') is not True:errors.append('Inconsistent command evidence')
 if not commands or not checks or any(r.get('passed') is not True for r in checks):errors.append('Missing/failed property evidence')
 def executions(name):return [r for r in commands if r.get('argv') and (r['argv'][0].split('/')[-1]==name or r['argv'][0].endswith('/ld-linux.so.2') and r['argv'][-1].split('/')[-1]==name)]
 for module in selected:
  mutants={'format':['signed','terminator','premature','learner'],'unicode':['range','swallow','rollback','unicode-learner'],'bytes':['direction','cursor','padding','bytes-learner'],'allocation':['alloc-overflow','alloc-owner','alloc-rollback','allocation-learner'],'conversion':['convert-errno','convert-range','convert-grammar','conversion-learner'],'headers':['headers-zero','headers-narrow','headers-capability','headers-learner'],'scanner':['scanner-commit','scanner-suffix','scanner-drain','scanner-learner'],'fileio':['fileio-replay','fileio-error','fileio-close','fileio-learner'],'mathlab':['mathlab-errno','mathlab-restore','mathlab-policy','mathlab-learner'],'sorting':['sorting-nan','sorting-bias','sorting-partial','sorting-learner'],'picolib':['picolib-drop','picolib-learner'],'processlab':['processlab-scale','processlab-copy','processlab-learner'],'control':['control-signal','control-cleanup','control-payload','control-learner']}[module];marker=module.upper()+'-CHECK:' if module!='unicode' else 'UNICODE-CHECK:'
  for name in mutants:
   rows=executions(name)
   expected_code={'picolib-drop':13,'picolib-learner':11}.get(name,1)
   if len(rows)!=1 or rows[0]['exit']!=expected_code or module!='picolib' and marker not in rows[0]['stdout']+rows[0]['stderr']:errors.append('Missing semantic rejection: '+name)
  for cc in ('gcc','clang'):
   for profile in (('debug','release') if module=='picolib' else ('debug','release','sanitize')):
    suffixes={'format':[''],'unicode':['-unicode','-locale'],'bytes':['-bytes','-representations'],'allocation':['-allocation','-allocation-host'],'conversion':['-conversion','-conversion-host'],'headers':['-headers'],'scanner':['-scanner'],'fileio':['-fileio','-fileio-host'],'mathlab':['-mathlab'],'sorting':['-sorting','-sorting-host'],'picolib':['-picolib'],'processlab':['-processlab','-processlab-host'],'control':['-control','-control-c-threads','-control-pthreads']}[module]
    for suffix in suffixes:
     rows=[r for r in executions(cc+'-'+profile+suffix) if 'borrow' not in r['argv']]
     if len(rows)!=1 or rows[0]['exit']!=0 or module!='picolib' and '-PASS:' not in rows[0]['stdout']:errors.append('Missing positive profile: '+cc+'-'+profile+suffix)
 if 'format' in selected:
  wrong=[r for r in commands if r.get('expected')=='nonzero' and any(x.endswith('/format/'+n+'.c') for x in r['argv'] for n in ('size','ptrdiff','long-double','width'))]
  if len(wrong)!=8 or any('format' not in r['stderr'].lower() for r in wrong):errors.append('Missing intended format translation rejection')
 if 'unicode' in selected:
  rows=[r for r in checks if r['name']=='independent full scalar encoding and bounded streaming oracle']
  if len(rows)!=2 or any(r['observed']['counts']!={'scalar_encodings':1112064,'utf16_encodings':1112064,'stream_calls':153457,'invalid_rejections':47836,'capacity_rejections':7939,'incomplete_prefixes':23453,'codec_deferred_impossible_prefix':71} or r['observed']['seed']!=120617 for r in rows):errors.append('Missing exact independent codec bounds')
  names={r['name'] for r in checks}
  if not {'actual wchar object manifest linux','actual wchar object manifest windows','original decoder M4 object has no runtime imports'}<=names:errors.append('Missing portability/import observation')
 if 'bytes' in selected and not any(r['name']=='actual accepted FCursor imported without cloning' for r in checks):errors.append('Missing actual prerequisite integration')
 if 'allocation' in selected and not any(r['name']=='actual accepted checked-size provider imported' for r in checks):errors.append('Missing allocation predecessor integration')
 if 'conversion' in selected:
  rows=[r for r in checks if r['name']=='independent numeric grammar and unbounded range oracle']
  if len(rows)!=2 or any(r['observed']['tokens']!=9350 or r['observed']['counts']['signed']!=18700 or r['observed']['counts']['unsigned']!=37400 for r in rows):errors.append('Missing independent numeric bounds')
  if not any(r['name']=='actual accepted strict decimal parser imported' for r in checks):errors.append('Missing numeric predecessor integration')
 if 'headers' in selected:
  names={r['name'] for r in checks};required={'actual wider intermediate executed','strict parser final image has empty runtime closure','LTO removes unreachable library path','AVR actual dependency identities',*['actual header acceptance matrix '+cc for cc in ('gcc','clang')]}
  if not required<=names:errors.append('Missing actual header ABI/closure evidence')
  for row in checks:
   if row['name'].startswith('actual header acceptance matrix '):
    cells=row['observed']['cells']
    if len(cells)!=30 or len({(r['edition'],r['header']) for r in cells})!=30 or any(r['accepted']!=(r['exit']==0) or r['exit']<0 for r in cells):errors.append('Incomplete header observation matrix')
  wrong=[r for r in commands if any(x.endswith('/headers/avr-wrong.c') for x in r['argv']) and r['expected']=='nonzero']
  if len(wrong)!=1 or 'format' not in wrong[0]['stderr']:errors.append('Missing AVR format rejection')
  if len(executions('i386-headers'))!=1:errors.append('Missing actual narrow ABI execution')
 if 'scanner' in selected:
  rows=[r for r in checks if r['name']=='independent escaped-record oracle']
  if len(rows)!=2 or any(r['observed']['counts']['cases']!=19180 for r in rows):errors.append('Missing independent scanner bounds')
  if not any(r['name']=='actual cursor and strict integer predecessors imported' for r in checks):errors.append('Missing parser prerequisite imports')
  wrong=[r for r in commands if r.get('expected')=='nonzero' and any(x.endswith('/scanner/wrong-destination.c') for x in r['argv'])]
  if len(wrong)!=2 or any('format' not in r['stderr'] for r in wrong):errors.append('Missing scanner translation controls')
 if 'fileio' in selected:
  if not any(r['name']=='actual accepted prefix writer imported; zero remains terminal under that contract' for r in checks):errors.append('Missing copy predecessor import')
  borrowed=[r for r in commands if r['argv'][-1]=='borrow']
  if len(borrowed)!=6 or any(r['exit']!=0 or 'borrowed-alive' not in r['stdout'] for r in borrowed):errors.append('Missing borrowed stdout executions')
 if 'mathlab' in selected:
  names={r['name'] for r in checks}
  needed={'MPFR actual host dependencies','exact glibc2.41 libm loaded','musl1.2.5 values runner is static','exact comparison runtime hashes',*['actual generic selection ABI '+cc for cc in ('gcc','clang')]}
  if not needed<=names:errors.append('Missing math dependency/type/runtime evidence')
  rows=[r for r in checks if r['name'].startswith('independent math oracle ')]
  if {r['name'].removeprefix('independent math oracle ') for r in rows}!={'native-gcc','native-clang','glibc241','musl125'} or len(rows)!=4 or any(r['observed']['cases']!=18 or r['observed']['precision']!=512 for r in rows):errors.append('Missing bounded independent math oracle')
  for cc in ('gcc','clang'):
   rows=executions(cc+'-fast-mathlab')
   if len(rows)!=1 or rows[0]['exit']!=0 or 'unsupported fast profile rejected' not in rows[0]['stdout']:errors.append('Missing actual fast profile refusal')
  for name,exitcode in [('counterfeit',1),('invalid-trailing',2),('invalid-duplicate',2),('invalid-missing',2)]:
   rows=[r for r in commands if r['argv'][-1].endswith('/math-'+name+'.txt')]
   if len(rows)!=1 or rows[0]['exit']!=exitcode:errors.append('Missing math oracle rejection: '+name)
 if 'sorting' in selected:
  rows=[r for r in checks if r['name'].startswith('independent sorting histogram and finite laws ')]
  if len(rows)!=6 or any(r['observed']!={'arrays':3906,'integer_triples':125,'record_triples':512,'failure_cuts':33,'toy_predictions':8} for r in rows):errors.append('Missing independent sorting bounds')
  required={'actual accepted stable sort provider imported','actual glibc threshold and total allocation denial','exact glibc2.41 sorting runtime loaded','musl sorting runner is static','exact sorting source and runtime identities'}
  if not required<={r['name'] for r in checks}:errors.append('Missing sorting source/provider/failure evidence')
  if not any(r['name']=='actual glibc threshold and total allocation denial' and r['observed']['cases']==12 for r in checks):errors.append('Missing exact sorting allocation cases')
 if 'picolib' in selected:
  required={'exact tagged options contradict old posix-io premise','actual seekable file roundtrip','POSIX file backend runtime closure','actual POSIX-console archive linked','exact Picolibc sources builds and libraries','retained supported configuration false','retained supported configuration true'}
  if not required<={r['name'] for r in checks}:errors.append('Missing exact Picolibc configuration/closure evidence')
  rejected=[r for r in commands if '-Dposix-io=false' in r['argv']]
  if len(rejected)!=1 or rejected[0]['exit']!=1 or 'Unknown option: "posix-io"' not in rejected[0]['stdout']+rejected[0]['stderr']:errors.append('Missing actual obsolete-option rejection')
  for name in ('read','lseek'):
   rows=[r for r in commands if '-Wl,--wrap='+name in r['argv']]
   if len(rows)!=1 or not isinstance(rows[0]['exit'],int) or rows[0]['exit']<=0 or '__wrap_'+name not in rows[0]['stderr']:errors.append('Missing exact backend rejection: '+name)
 if 'processlab' in selected:
  rows=[r for r in checks if r['name'].startswith('actual clocks and changed scheduling ')]
  if len(rows)!=6 or any(r['observed']['samples']!=12 or r['observed']['environment_updates']!=1000 or r['observed']['owned_snapshots']!=1000 or len(r['observed']['raw'])!=12 for r in rows):errors.append('Missing clock/process-state bounds')
  names={r['name'] for r in checks}
  if not {'actual time ABI 32','actual time ABI 64','actual narrow time runtime hashes'}<=names or len(executions('process-time32'))!=1 or len(executions('process-time64'))!=1:errors.append('Missing actual time ABI execution')
  for cc in ('gcc','clang'):
   for mode,wanted in [('normal','BA'),('quick','YX'),('immediate','')]:
    rows=[r for r in checks if r['name']=='actual isolated termination '+cc+' '+mode]
    if len(rows)!=1 or rows[0]['observed']['events']!=wanted or (rows[0]['observed']['stdout']!='buffered:' if mode=='normal' else rows[0]['observed']['stdout'] not in ('','buffered:')):errors.append('Missing isolated termination '+cc+' '+mode)
 if 'control' in selected:
  for cc in ('gcc','clang'):
   rows=executions(cc+'-control-signal')
   if len(rows)!=1 or rows[0]['exit']!=0 or 'CONTROL-PASS:owned child signal' not in rows[0]['stdout']:errors.append('Missing POSIX signal execution')
  rows=[r for r in commands if r['argv'][0].endswith('/genmc')]
  if len(rows)!=3 or sorted(r['exit'] for r in rows)!=[0,42,42] or any('Rrlx (payload, 0) [INIT]' not in r['stdout']+r['stderr'] or 'Error: Safety violation!' not in r['stdout']+r['stderr'] for r in rows if r['exit']==42):errors.append('Missing actual RC11 counterexamples')
  named=[r for r in checks if r['name'].startswith('actual RC11 publication model ')]
  if len(named)!=3 or any(r['observed']['executions']<1 for r in named):errors.append('Missing finite model exploration')
  names={r['name'] for r in checks}
  if not {'actual M0 atomic helper dependency','actual M4 publication final closure is empty','actual GenMC identity'}<=names:errors.append('Missing atomic target closure')
  links=[r for r in commands if r['argv'][0]=='arm-none-eabi-ld' and any(x.endswith('/cortex-m0-publication.elf') for x in r['argv'])]
  if len(links)!=1 or links[0]['exit']<=0 or '__atomic_load_4' not in links[0]['stderr'] or '__atomic_store_4' not in links[0]['stderr']:errors.append('Missing intended atomic helper rejection')
 return errors
def main():
 p=argparse.ArgumentParser(description=__doc__);p.add_argument('--report',required=True);p.add_argument('--output',default='reviews/v5/expansion/legacy_library/support_validation.json');a=p.parse_args();data=read_json(ROOT/a.report);errors=validate(data);controls=[]
 if not errors:
  cases=[('missing changed criterion',lambda d:d['observations'].pop(),'Missing exact ordinary/changed observation'),('inflated whole topic',lambda d:d.update(topic_complete=True),'Partial campaign inflated to whole topic'),('missing retained builds',lambda d:d.update(retained_artifacts={}),'Missing retained build artifacts')]
  if 'format' in data['selected_modules']:cases.append(('translation control omitted',lambda d:d.update(commands=[r for r in d['commands'] if not (r.get('expected')=='nonzero' and any(x.endswith('/format/width.c') for x in r['argv']))]),'Missing intended format translation rejection'))
  if 'unicode' in data['selected_modules']:cases.append(('weaker codec bound',lambda d:[r['observed']['counts'].update(stream_calls=1) for r in d['checks'] if r['name']=='independent full scalar encoding and bounded streaming oracle'],'Missing exact independent codec bounds'))
  if 'headers' in data['selected_modules']:cases.append(('narrow ABI omitted',lambda d:d.update(commands=[r for r in d['commands'] if not(r['argv'][0].endswith('/ld-linux.so.2') and r['argv'][-1].endswith('/i386-headers'))]),'Missing actual narrow ABI execution'))
  if 'scanner' in data['selected_modules']:cases.append(('weaker parser bound',lambda d:[r['observed']['counts'].update(cases=1) for r in d['checks'] if r['name']=='independent escaped-record oracle'],'Missing independent scanner bounds'))
  if 'fileio' in data['selected_modules']:cases.append(('borrowed stdout omitted',lambda d:d.update(commands=[r for r in d['commands'] if r['argv'][-1]!='borrow']),'Missing borrowed stdout executions'))
  if 'mathlab' in data['selected_modules']:cases.append(('weaker math oracle',lambda d:[r['observed'].update(precision=53) for r in d['checks'] if r['name'].startswith('independent math oracle ')],'Missing bounded independent math oracle'))
  if 'sorting' in data['selected_modules']:cases.append(('weaker sorting histogram',lambda d:[r['observed'].update(arrays=1) for r in d['checks'] if r['name'].startswith('independent sorting histogram and finite laws ')],'Missing independent sorting bounds'))
  if 'picolib' in data['selected_modules']:cases.append(('obsolete option not tested',lambda d:d.update(commands=[r for r in d['commands'] if '-Dposix-io=false' not in r['argv']]),'Missing actual obsolete-option rejection'))
  if 'processlab' in data['selected_modules']:
   cases.append(('weaker environment campaign',lambda d:[r['observed'].update(owned_snapshots=1) for r in d['checks'] if r['name'].startswith('actual clocks and changed scheduling ')],'Missing clock/process-state bounds'))
   cases.append(('time32 not executed',lambda d:d.update(commands=[r for r in d['commands'] if not(r['argv'][0].endswith('/ld-linux.so.2') and r['argv'][-1].endswith('/process-time32'))]),'Missing actual time ABI execution'))
  if 'control' in data['selected_modules']:
   cases.append(('model compilation failure passed as counterexample',lambda d:[r.update(exit=1,expected=1) for r in d['commands'] if r['argv'][0].endswith('/genmc') and r['exit']==42],'Missing actual RC11 counterexamples'))
   cases.append(('unrelated linker failure passed as closure rejection',lambda d:[r.update(stderr='linker unavailable') for r in d['commands'] if r['argv'][0]=='arm-none-eabi-ld' and r['exit']!=0],'Missing intended atomic helper rejection'))
  for module in data['selected_modules']:
   name={'format':'signed','unicode':'range','bytes':'direction','allocation':'alloc-owner','conversion':'convert-errno','headers':'headers-capability','scanner':'scanner-commit','fileio':'fileio-close','mathlab':'mathlab-restore','sorting':'sorting-bias','picolib':'picolib-drop','processlab':'processlab-copy','control':'control-cleanup'}[module];cases.append((module+' mutant crash',lambda d,n=name:[r.update(exit=-11,expected=-11) for r in d['commands'] if r['argv'][0].endswith('/'+n)],'Missing semantic rejection: '+name))
  for name,mutate,wanted in cases:
   bad=deepcopy(data);mutate(bad);assert wanted in validate(bad),name;controls.append(name+' rejected')
  bad=deepcopy(data);bad['input_sha256'][next(iter(bad['input_sha256']))]='0'*64;assert any(e.startswith('Stale input:') for e in validate(bad));controls.append('stale source rejected')
 result=dict(passed=not errors,errors=errors,negative_guards=controls,selected_modules=data.get('selected_modules'),input_sha256={a.report:sha(ROOT/a.report),'tools/check_library_followthrough.py':sha(ROOT/'tools/check_library_followthrough.py')});dump(ROOT/a.output,result);print(json.dumps(result));return bool(errors)
if __name__=='__main__':raise SystemExit(main())
