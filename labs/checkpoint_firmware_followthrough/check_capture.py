"""Validate P4 capture consistency; synthetic checks never become physical evidence."""
from pathlib import Path
import argparse,copy,hashlib,json,statistics,tempfile,zlib
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def validate(d,base,synthetic=False):
 errors=[];metrics={}
 def need(x,s):
  if not x:errors.append(s)
 need(d.get('status')=='observed','not observed')
 need(d.get('kind')==('synthetic' if synthetic else 'physical'),'wrong evidence tier')
 for k in ['board','revision','clock_id','build_id','compiler_flags','setup_id','pin_route','correlation_argument','limits']:need(bool(d.get(k)),'missing '+k)
 instrument=d.get('instrument',{});need(instrument.get('sample_rate_hz',0)>0 and instrument.get('uncertainty_ns',0)>0 and bool(instrument.get('threshold_loading')),'unqualified instrument')
 need(d.get('nominal_core_hz')==16000000 and d.get('measured_core_hz',0)>0 and d.get('clock_uncertainty_ppm',-1)>=0,'clock qualification')
 raw=d.get('raw_files',[]);need(bool(raw),'missing raw captures')
 for r in raw:
  p=(base/r.get('path','')).resolve();need(p.is_relative_to(base.resolve()) and p.is_file(),'raw file boundary')
  if p.is_file() and p.is_relative_to(base.resolve()):need(sha(p)==r.get('sha256'),'raw identity')
 image=d.get('image',{});p=(base/image.get('path','')).resolve();need(p.is_relative_to(base.resolve()) and p.is_file(),'missing image')
 if p.is_file() and p.is_relative_to(base.resolve()):need(sha(p)==image.get('sha256'),'image identity')
 c=d.get('capture',{});need(c.get('magic')==0x50344331 and c.get('version')==1,'capture ABI');need(c.get('state') in [2,3],'snapshot not stopped')
 need(c.get('mode') in [0,1,2] and c.get('trace_enabled') in [0,1] and c.get('ring_capacity') in [2,4],'selected profile')
 counts=[c.get(k,-1) for k in ['received','accepted','dropped','completed']];need(all(type(v) is int and 0<=v<=64 for v in counts),'capture counts');received,accepted,dropped,completed=counts
 need(received==accepted+dropped and completed<=accepted,'event conservation');need(type(d.get('source_count')) is int and d.get('source_count',-1)>=received,'source/ISR accounting');need(d.get('missing_interrupts')==d.get('source_count',0)-received,'missing IRQ accounting')
 need(c.get('retained') in [0,1] and (c.get('state')!=3 or c.get('first_fault',0)!=0),'failure snapshot')
 rows=c.get('rows',[]);need(len(rows)==64,'full bounded RAM row array');duration=(c.get('clock_end',0)-c.get('clock_start',0))&0xffffffff;need(0<duration<0x80000000,'capture wrap ambiguity')
 selected=rows[:max(0,received)];need([r.get('sequence') for r in selected]==list(range(max(0,received))),'missing/repeated firmware sequence')
 failed=sum(bool(r.get('flags',0)&32) for r in selected);need(d.get('failed_operations')==failed and d.get('outstanding')==accepted-completed-failed and accepted>=completed+failed,'terminal/outstanding accounting')
 need(sum(bool(r.get('flags',0)&1) for r in selected)==accepted and sum(bool(r.get('flags',0)&2) for r in selected)==dropped and sum(bool(r.get('flags',0)&16) and not bool(r.get('flags',0)&32) for r in selected)==completed,'row/count reconciliation')
 for r in selected:
  f=r.get('flags',0);need(bool(f&1)!=bool(f&2) and not f&~63,'invalid terminal admission')
  need(not f&2 or f==2,'dropped record has later stages');need(not f&16 or f&4,'wire before framing');need(not f&8 or c.get('mode')==1 and f&4,'DMA mode/stage')
  if f&16 and c.get('mode')==1:need(bool(f&8),'wire missing DMA quiescence')
  if c.get('trace_enabled'):
   fields=['isr_cycle','publish_cycle']+(['frame_cycle'] if f&4 else [])+(['dma_cycle'] if f&8 else [])+(['tc_cycle'] if f&16 else [])
   ticks=[(r.get(k,0)-c.get('clock_start',0))&0xffffffff for k in fields];need(ticks==sorted(ticks) and all(t<=duration for t in ticks),'stage clock order or wrap')
 observations=d.get('observations',[]);warmup=d.get('warmup_sequences',[]);need(len(warmup)==len(set(warmup)) and all(type(x) is int and 0<=x<received for x in warmup),'warmup identities');seen=set();sources=set();latencies=[]
 uncertainty=instrument.get('uncertainty_ns',0)
 for o in observations:
  try:
   s=o['sequence'];source=o['source_ordinal'];assert type(s) is int and 0<=s<received and s not in seen and source not in sources and 0<=source<d['source_count'];seen.add(s);sources.add(source)
   r=rows[s];assert r['flags']&16 and not r['flags']&32 and o['correlation']=='unique'
   wire=bytes.fromhex(o['wire_hex']);assert len(wire)==18 and wire[:2]==b'\xa5\x01' and int.from_bytes(wire[2:4],'little')==s and wire[4:6]==b'\x08\0'
   assert int.from_bytes(wire[6:10],'little')==s and int.from_bytes(wire[10:12],'little')==100+s
   assert int.from_bytes(wire[14:18],'little')==zlib.crc32(wire[1:14])
   if c['trace_enabled']:assert int.from_bytes(wire[12:14],'little')==(r['isr_cycle']>>4)&65535
   assert 0<=o['arrival_ns']<=o['wire_end_ns']
   if c['trace_enabled']:
    assert o['wire_end_ns']<=o['tc_marker_ns']+uncertainty
    assert o['arrival_ns']<=o['isr_marker_ns']+uncertainty and o['isr_marker_ns']<=o['publish_marker_ns']+uncertainty and o['publish_marker_ns']<=o['frame_marker_ns']+uncertainty and o['frame_marker_ns']<=o['wire_end_ns']+uncertainty
    if c['mode']==1:assert o['frame_marker_ns']<=o['dma_marker_ns']+uncertainty and o['dma_marker_ns']<=o['tc_marker_ns']+uncertainty
   if c['mode']==2:
    # Adapter holds PC0 for at least 16000 cycles AFTER its sampled TC marker.
    # The clock is nominally 16 MHz; use the qualified upper frequency bound.
    fastest=d['measured_core_hz']*(1+d['clock_uncertainty_ppm']/1000000)
    assert o['deassert_ns']>=(o['tc_marker_ns'] if c['trace_enabled'] else o['wire_end_ns'])+16000*1000000000/fastest-2*uncertainty
   if s not in warmup:latencies.append(o['wire_end_ns']-o['arrival_ns'])
  except (KeyError,TypeError,ValueError,AssertionError):errors.append('inconsistent physical stage/correlation')
 need(len(observations)==completed and len(seen)==completed,'all completions represented');need(bool(latencies) or completed==0,'no nonwarmup latency')
 if latencies:metrics=dict(count=len(latencies),median_ns=statistics.median(latencies),spread_ns=max(latencies)-min(latencies),observed_max_ns=max(latencies),minimum_ns=min(latencies))
 return dict(passed=not errors,errors=errors,metrics=metrics,physical_claim=False,evidence_tier='synthetic' if synthetic else 'claimed physical capture; consistency only',limits=['Consistency and raw identity do not authenticate a claimed physical capture.','Observed maximum is not WCET. Lost/dropped/failed/outstanding events remain in the denominator.','ISR-entry timestamp is not acquisition time; external arrival and wire endpoint are separate observations.'])
def self_test():
 with tempfile.TemporaryDirectory() as temp:
  base=Path(temp);raw=base/'synthetic.txt';raw.write_text('synthetic checker input, not a board capture\n')
  row=dict(sequence=0,isr_cycle=105,publish_cycle=110,frame_cycle=120,dma_cycle=130,tc_cycle=140,flags=29,generation=1)
  zero={k:0 for k in row};c=dict(magic=0x50344331,version=1,trace_enabled=1,mode=1,ring_capacity=4,received=1,accepted=1,dropped=0,completed=1,state=2,clock_start=100,clock_end=150,first_fault=0,fault_generation=0,retained=0,retries=0,rows=[row]+[dict(zero) for _ in range(63)])
  d=dict(status='observed',kind='synthetic',board='synthetic',revision='synthetic',clock_id='synthetic',build_id='synthetic',compiler_flags='synthetic',setup_id='synthetic',pin_route='synthetic',correlation_argument='one source event, no ambiguity',limits=['synthetic only'],instrument=dict(sample_rate_hz=100000000,uncertainty_ns=10,threshold_loading='synthetic'),nominal_core_hz=16000000,measured_core_hz=16000000,clock_uncertainty_ppm=1,raw_files=[dict(path=raw.name,sha256=sha(raw))],image=dict(path=raw.name,sha256=sha(raw)),capture=c,source_count=2,missing_interrupts=1,outstanding=0,failed_operations=0,warmup_sequences=[],observations=[dict(sequence=0,source_ordinal=1,correlation='unique',arrival_ns=100,isr_marker_ns=120,publish_marker_ns=130,frame_marker_ns=200,dma_marker_ns=1500,tc_marker_ns=2010,wire_end_ns=2000)])
  wire=bytes.fromhex('a501000008000000000064000600');d['observations'][0]['wire_hex']=(wire+zlib.crc32(wire[1:]).to_bytes(4,'little')).hex()
  assert validate(d,base,True)['passed'];assert validate(d,base,True)['metrics']['observed_max_ns']==1900;assert not validate(d,base)['passed']
  positives=['DMA trace on']
  for mode in [0,1,2]:
   for trace in [0,1]:
    x=copy.deepcopy(d);x['capture']['mode']=mode;x['capture']['trace_enabled']=trace
    if mode!=1:x['capture']['rows'][0]['flags']=21
    if mode==2:x['observations'][0]['deassert_ns']=1002050
    if not trace:
     for key in list(x['observations'][0]):
      if key.endswith('_marker_ns'):del x['observations'][0][key]
     for row in x['capture']['rows']:
      for key in row:
       if key.endswith('_cycle'):row[key]=0
    assert validate(x,base,True)['passed'],(mode,trace,validate(x,base,True));positives.append(f'mode{mode} trace{trace}')
  x=copy.deepcopy(d);x['source_count']=4;x['missing_interrupts']=1;x['capture'].update(received=3,accepted=2,dropped=1,state=3,first_fault=2,retained=1)
  x['capture']['rows'][1]=dict(zero,sequence=1,isr_cycle=141,publish_cycle=142,flags=2)
  x['capture']['rows'][2]=dict(zero,sequence=2,isr_cycle=143,publish_cycle=144,flags=33)
  x['failed_operations']=1;assert validate(x,base,True)['passed'];positives.append('failed retained plus dropped event')
  controls=[]
  for kind in ['unobserved','invented-physical','raw-hash','image-hash','conservation','missing-irq','outstanding','trace-wrap','reordered','missing-sequence','lost-completion','dma-is-wire','late-timestamp','ambiguous','lost-warmup-all','halfduplex-early','dropped-published','wire-corruption','wire-sequence','wire-timestamp']:
   x=copy.deepcopy(d)
   if kind=='unobserved':x['status']='unobserved'
   elif kind=='invented-physical':x['kind']='physical'
   elif kind=='raw-hash':x['raw_files'][0]['sha256']='0'*64
   elif kind=='image-hash':x['image']['sha256']='0'*64
   elif kind=='conservation':x['capture']['accepted']=2
   elif kind=='missing-irq':x['missing_interrupts']=0
   elif kind=='outstanding':x['outstanding']=1
   elif kind=='trace-wrap':x['capture']['clock_end']=0x80000064
   elif kind=='reordered':x['capture']['rows'][0]['frame_cycle']=149
   elif kind=='missing-sequence':x['capture']['rows'][0]['sequence']=1
   elif kind=='lost-completion':x['observations']=[]
   elif kind=='dma-is-wire':x['observations'][0]['tc_marker_ns']=1500
   elif kind=='late-timestamp':x['observations'][0]['arrival_ns']=2001
   elif kind=='ambiguous':x['observations'][0]['correlation']='guessed'
   elif kind=='lost-warmup-all':x['warmup_sequences']=[0]
   elif kind=='halfduplex-early':x['capture']['mode']=2;x['capture']['rows'][0]['flags']=21;x['observations'][0]['deassert_ns']=2011
   elif kind=='dropped-published':x['capture']['rows'][0]['flags']=30
   else:
    wire=bytearray.fromhex(x['observations'][0]['wire_hex']);wire[{'wire-corruption':17,'wire-sequence':6,'wire-timestamp':12}[kind]]^=1
    if kind!='wire-corruption':wire[-4:]=zlib.crc32(wire[1:-4]).to_bytes(4,'little')
    x['observations'][0]['wire_hex']=wire.hex()
   rejected=not validate(x,base,True)['passed'];controls.append(dict(control=kind,rejected=rejected));assert rejected,kind
  return dict(passed=True,physical_claim=False,positive_controls=positives,negative_controls=controls)
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('capture',type=Path,nargs='?');p.add_argument('--self-test',action='store_true');p.add_argument('--synthetic',action='store_true');a=p.parse_args()
 d=self_test() if a.self_test else validate(json.loads(a.capture.read_text()),a.capture.parent,a.synthetic);print(json.dumps(d,indent=2));raise SystemExit(not d['passed'])
