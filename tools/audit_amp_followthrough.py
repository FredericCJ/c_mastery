"""Inspect actual machine records, resources, accepted ledger and capture gates."""
from pathlib import Path
import argparse,copy,hashlib,importlib.util,json,re,subprocess,sys,tempfile
ROOT=Path(__file__).resolve().parents[1]
from stack_amp_followthrough import bounds
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 p=argparse.ArgumentParser();p.add_argument('--campaign',type=Path,required=True);p.add_argument('--work',type=Path,required=True);a=p.parse_args();work=a.work.resolve();work.mkdir(parents=True,exist_ok=False);checks=[];commands=[];images=[]
 def check(name,value):
  checks.append(dict(name=name,passed=bool(value)))
  if not value:raise ValueError(name)
 def run(name,cmd):
  cmd=list(map(str,cmd));r=subprocess.run(cmd,cwd=ROOT,capture_output=True,text=True,timeout=60);log=work/(name+'.txt');log.write_text(r.stdout+r.stderr);commands.append(dict(name=name,command=cmd,returncode=r.returncode,log=log.relative_to(ROOT).as_posix(),sha256=sha(log)));check(name,r.returncode==0);return r.stdout+r.stderr
 try:
  campaign=json.loads(a.campaign.read_text());check('campaign passed',campaign['passed'] and len(campaign['rows'])==147)
  for path,h in campaign['source_sha256'].items():check('source '+path,sha(ROOT/path)==h)
  for row in campaign['rows']:check('command '+row['name'],row['passed'] and sha(ROOT/row['log'])==row['log_sha256'])
  for directory in sorted(a.campaign.parent.iterdir()):
   if not directory.is_dir() or not (directory/'report.json').exists():continue
   r=json.loads((directory/'report.json').read_text());negative='mutant' in directory.name
   check(directory.name+' status',r['passed']!=negative)
   for path,h in r['input_sha256'].items():check(directory.name+' input '+path,sha(ROOT/path)==h)
   for path,h in r['artifacts'].items():check(directory.name+' artifact '+path,sha(ROOT/path)==h)
   q=[x for x in r['rows'] if x['label'].endswith('emulator')]
   if q:
    check(directory.name+' one machine run',len(q)==1)
    if negative:check(directory.name+' exact failure',q[0]['returncode']==(229 if 'mutant1' in directory.name else 237));continue
    text=(ROOT/q[0]['log']).read_text();v=[int(x,16) for x in text.splitlines() if re.fullmatch('[0-9a-f]{8}',x)]
    check(directory.name+' 41 trace words',len(v)==41)
    offered,done,dropped,epoch,traffic,h0,h1,h2,switches=v[:9];pairs=[v[9+i*2:11+i*2] for i in range(16)]
    check(directory.name+' per-ID conservation',offered==512 and done+dropped==offered and epoch==2 and all((a&b)==0 and (a|b)==0xffffffff for a,b in pairs) and sum(x.bit_count() for x,y in pairs)==dropped and sum(y.bit_count() for x,y in pairs)==done)
    check(directory.name+' bounded resources',h0<=2 and h1<=4 and h2<=8 and switches>=20)
    check(directory.name+' traffic placement',traffic==(0 if 'local' in directory.name else done))
    if 'burst' in directory.name or 'local' in directory.name:check(directory.name+' deliberate overload',dropped>0)
   for image in r['images']:
    check(directory.name+' '+image['core']+' no physical claim',image['physical_status']=='not-run' and not image['external_runtime'] and not image['unresolved'])
    elf=ROOT/image['elf'];check(directory.name+' '+image['core']+' ELF',sha(elf)==image['sha256']);base=elf.parent
    sizes={m[1]:int(m[2]) for line in (base/'cost.txt').read_text().splitlines() if (m:=re.match(r'^(\.\w+)\s+(\d+)\s+',line))}
    check(directory.name+' '+image['core']+' RAM',sizes.get('.bss',0)+sizes.get('.data',0)+8192<131072 and sizes.get('.shared')==8192)
    dis=(base/'disassembly.txt').read_text();bodies={m[1]:m[2] for m in re.finditer(r'<([^>]+)>:\n(.*?)(?=\n[0-9a-f]+ <|\Z)',dis,re.S)}
    for name in ('af_batch','af_receive'):
     if name in bodies:check(directory.name+' '+image['core']+' ordering '+name,'dmb' in bodies[name])
    frames=[]
    for file in base.glob('*.su'):
     for line in file.read_text().splitlines():
      fields=line.split('\t')
      if len(fields)>=3:frames.append(dict(function=fields[0].rsplit(':',1)[-1],bytes=int(fields[1]),qualifier=fields[2]))
    check(directory.name+' '+image['core']+' compiler frame records',bool(frames) and all('unbounded' not in x['qualifier'] for x in frames))
    stack=bounds(base,frames);check(directory.name+' '+image['core']+' selected stack capacity',stack['MSP_selected_bytes']<8192 and (stack['PSP_selected_bytes'] is None or stack['PSP_selected_bytes']<2048))
    images.append(dict(profile=directory.name,image=image,sizes=sizes,compiler_frames=frames,linked_stack_bounds=stack,largest_reported_c_frame=max(x['bytes'] for x in frames),limits='Selected linked paths, compiler frames and actual task canary checks; not an exhaustive all-interrupt stack/WCET proof.'))
  for cc in ('gcc','clang'):
   for opt in ('O0','O3'):
    for name,files in [('ledger',['labs/amp_followthrough/core/ledger_tests.c','core/ledger.c']),('layout',['labs/amp_followthrough/core/layout_tests.c','core/amp.c']),('ack',['assessor/amp_followthrough/core/ack_probe.c','core/amp.c'])]:
     exe=work/(name+'-'+cc+'-'+opt);run(name+'-'+cc+'-'+opt+'-compile',[cc,'-std=c17','-'+opt,'-Wall','-Wextra','-Werror','-Ilabs/amp_followthrough/core',ROOT/files[0],ROOT/'assessor/amp_followthrough'/files[1],'-o',exe]);run(name+'-'+cc+'-'+opt,[exe])
  spec=importlib.util.spec_from_file_location('af_capture',ROOT/'labs/amp_followthrough/capture.py');mod=importlib.util.module_from_spec(spec);spec.loader.exec_module(mod)
  raw=work/'synthetic-capture.txt';raw.write_text('Synthetic validator fixture. No physical observation.\n');r={'images':[{'core':'m7','sha256':'7'*64},{'core':'m4','sha256':'4'*64}]};c={'physical_status':'observed','board_id':'synthetic','silicon_revision':'synthetic','probe_id':'synthetic','boot_configuration':'synthetic','image_readback_sha256':{'m7':'7'*64,'m4':'4'*64},'clock_segments':[{'core':'m7','frequency_hz':480000000,'elapsed_counts':48000,'probe_min_counts':1,'probe_max_counts':24}],'measurement_limits':['Synthetic validator fixture only.'],'cases':[{'name':name,'accepted_ids':[1,2,3],'completed_ids':[1],'dropped_ids':[2],'uncertain_ids':[3],'raw_file':raw.name,'epoch':1,'sample_drops':0,'result':'fail-closed' if name=='unexpected-reset' else 'synthetic'} for name in sorted(mod.CASES)],'raw_files':[{'path':raw.name,'sha256':sha(raw)}]}
  check('synthetic capture structure accepted',mod.validate(c,r,work)['passed'])
  for label,edit in [('not-observed',lambda x:x.update(physical_status='not-run')),('wrong-image',lambda x:x['image_readback_sha256'].update(m4='0'*64)),('lost-case',lambda x:x['cases'].pop()),('duplicated-ID',lambda x:x['cases'][0]['completed_ids'].append(2)),('changed-raw',lambda x:x['raw_files'][0].update(sha256='0'*64))]:
   bad=copy.deepcopy(c);edit(bad)
   try:mod.validate(bad,r,work);rejected=False
   except ValueError:rejected=True
   check('capture rejects '+label,rejected)
  template=json.loads((ROOT/'labs/amp_followthrough/capture_template.json').read_text())
  try:mod.validate(template,r,work);rejected=False
  except ValueError:rejected=True
  check('empty physical template rejected',rejected)
 except Exception as e:passed=False;error=str(e)
 else:passed=True;error=None
 paths=[Path(__file__).resolve(),ROOT/'tools/stack_amp_followthrough.py',ROOT/'labs/amp_followthrough/capture.py',ROOT/'labs/amp_followthrough/capture_template.json',*[ROOT/p for p in ('labs/amp_followthrough/core/ledger.h','labs/amp_followthrough/core/ledger_tests.c','labs/amp_followthrough/core/layout_tests.c','assessor/amp_followthrough/core/ledger.c','assessor/amp_followthrough/core/amp.c','assessor/amp_followthrough/core/ack_probe.c')]]
 result=dict(passed=passed,error=error,checks=checks,commands=commands,images=images,campaign_sha256=sha(a.campaign),source_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in paths},artifacts={p.relative_to(ROOT).as_posix():sha(p) for p in work.rglob('*') if p.is_file()},scope='Actual source/image/raw-ledger audit plus exact supplemental native checks; capture tests are explicitly synthetic, never physical evidence.')
 (work/'result.json').write_text(json.dumps(result,indent=2)+'\n');print(json.dumps(dict(passed=passed,error=error,checks=len(checks),commands=len(commands),images=len(images))));return int(not passed)
if __name__=='__main__':raise SystemExit(main())
