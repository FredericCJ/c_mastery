"""Decode a stopped capture and check an explicitly supplied wire observation.
Passing validates consistency, never provenance or actual physical execution.
"""
import argparse,copy,hashlib,json,math,struct
from pathlib import Path
FIELDS='magic version done status stage polls brr baud cr1 cr2 cr3 presc ccipr moder afrl isr bytes transfer_status count_ticks part_id ready released mco_cfgr gpio_afrh'.split()
def check(raw,meta,elf):
 errors=[]
 def number(x):return type(x) in [int,float] and math.isfinite(x)
 def need(x,m):
  if not x:errors.append(m)
 if len(raw)!=96:return ['capture length']
 d=dict(zip(FIELDS,struct.unpack('<24I',raw)))
 need(d['magic']==0x47345532 and d['version']==2,'capture identity');need(d['done']==1 and d['ready']==1 and d['released']==1,'completed capture and instrument barrier')
 need(d['status']==0 and d['stage']==5 and 2<=d['polls']<=200000,'successful initialization');need(d['baud'] in [9600,115200,1000000],'selected baud')
 need(d['brr']=={9600:1667,115200:139,1000000:16}.get(d['baud']),'independent divisor');need(d['cr1']==9 and d['cr2']==d['cr3']==d['presc']==0,'selected mode')
 need(d['ccipr']&12==8 and d['moder']&48==32 and d['afrl']&0xf00==0x700,'kernel clock and PA2');need(d['isr']&(1<<21) and d['isr']&64,'acknowledged and wire complete')
 need(d['bytes']==5 and d['transfer_status']==0 and 5<=d['count_ticks']<=500010,'bounded five-byte transfer');need(d['mco_cfgr']&0x7f000000==0x43000000 and d['gpio_afrh']&15==0 and d['moder']&(3<<16)==2<<16,'MCO HSI16/16 PA8 AF0')
 need(meta.get('elf_sha256')==hashlib.sha256(elf).hexdigest(),'ELF identity');need(meta.get('capture_sha256')==hashlib.sha256(raw).hexdigest(),'capture identity hash')
 need(meta.get('stopped') is True and meta.get('board')=='NUCLEO-G474RE' and meta.get('revision') and meta.get('part_id')==d['part_id'],'as-built identity and stopped snapshot')
 need(meta.get('route_verified') is True and meta.get('bridge_configuration') and meta.get('probe_route')=='PA2 AF7 USART2_TX','as-built route')
 need(meta.get('wire_bytes')==[85,0,255,165,90] and meta.get('framing_errors')==0,'wire decode')
 need(number(meta.get('supply_volts')) and 1.71<=meta['supply_volts']<=3.6 and number(meta.get('ambient_c')),'recorded conditions')
 need(number(meta.get('release_delay_us')) and meta['release_delay_us']>=1000,'host barrier delay')
 intervals=meta.get('bit_period_us',[]);interval_ok=isinstance(intervals,list) and len(intervals)>=8 and all(number(x) and x>0 for x in intervals);need(interval_ok,'measured periods')
 hz=meta.get('measured_hsi_hz');budget=meta.get('receiver_error_budget_fraction');rx=meta.get('receiver_clock_error_fraction');unc=meta.get('period_uncertainty_us')
 valid=all(number(x) for x in [hz,budget,rx,unc]) and 15000000<hz<17000000 and 0<budget<0.1 and 0<=rx<budget and unc>=0
 need(valid,'qualified oscillator, receiver budget and uncertainty')
 mco=meta.get('measured_mco_hz');mco_unc=meta.get('mco_uncertainty_hz');mco_ok=number(mco) and number(mco_unc) and 900000<mco<1100000 and 0<mco_unc<10000
 need(mco_ok and number(hz) and abs(hz-16*mco)<0.001,'independent MCO measurement and uncertainty')
 need(meta.get('clock_probe_route')=='PA8 AF0 MCO HSI16/16 CN9.8' and meta.get('raw_clock_capture_sha256') and meta.get('clock_instrument'),'independent clock route/instrument/evidence')
 if valid and interval_ok and mco_ok and d['brr']:
  need(abs(hz/d['brr']/d['baud']-1)+16*mco_unc/d['brr']/d['baud']+rx<budget,'combined asynchronous error budget')
  predicted=1e6*d['brr']/hz
  need(all(abs(x-predicted)<=unc for x in intervals),'wire rate consistent with independently measured HSI')
 need(meta.get('instrument') and meta.get('raw_wire_capture_sha256') and meta.get('hsi_measurement_method'),'instrument and raw evidence references')
 return errors
def self_test():
 elf=b'original synthetic test image';values=[0x47345532,2,1,0,5,2,139,115200,9,0,0,0,8,32|(2<<16),0x700,(1<<21)|64,5,0,5,0x10000469,1,1,0x43000000,0];raw=struct.pack('<24I',*values)
 meta=dict(elf_sha256=hashlib.sha256(elf).hexdigest(),capture_sha256=hashlib.sha256(raw).hexdigest(),stopped=True,board='NUCLEO-G474RE',revision='synthetic only',part_id=values[19],route_verified=True,bridge_configuration='synthetic SB17/SB23 on; others off',probe_route='PA2 AF7 USART2_TX',wire_bytes=[85,0,255,165,90],framing_errors=0,supply_volts=3.0,ambient_c=30,release_delay_us=1000,bit_period_us=[8.6875]*8,measured_hsi_hz=16000000,receiver_error_budget_fraction=.02,receiver_clock_error_fraction=.001,period_uncertainty_us=.01,instrument='synthetic',raw_wire_capture_sha256='a'*64,hsi_measurement_method='synthetic independent oscillator measurement')
 meta.update(measured_mco_hz=1000000,mco_uncertainty_hz=10,clock_probe_route='PA8 AF0 MCO HSI16/16 CN9.8',raw_clock_capture_sha256='b'*64,clock_instrument='synthetic external calibrated counter')
 assert not check(raw,meta,elf)
 controls=[]
 changes={'done':(2,0),'mode':(8,9|(1<<29)),'clock':(12,0),'pin':(14,0x800),'divisor':(6,138),'transfer':(16,4),'ack':(15,64),'barrier':(21,0),'MCO-pin':(23,1),'MCO-divider':(22,0x03000000),'version':(1,1)}
 for name,(index,v) in changes.items():
  a=values.copy();a[index]=v;b=struct.pack('<24I',*a);m=copy.deepcopy(meta);m['capture_sha256']=hashlib.sha256(b).hexdigest();assert check(b,m,elf);controls.append(name)
 for name,key,value in [('stopped','stopped',False),('route','route_verified',False),('delay','release_delay_us',999),('rate','bit_period_us',[10]*8),('budget','receiver_error_budget_fraction',.0001),('wire','wire_bytes',[0]*5),('identity','elf_sha256','x'),('finite','measured_hsi_hz',float('nan')),('evidence','raw_wire_capture_sha256',''),('bad-array','bit_period_us',None),('bad-period','bit_period_us',['x']*8),('bad-voltage','supply_volts','x'),('circular-clock','measured_mco_hz',999000),('no-clock-error','mco_uncertainty_hz',0),('clock-evidence','raw_clock_capture_sha256','')]:
  m=copy.deepcopy(meta);m[key]=value;assert check(raw,m,elf);controls.append(name)
 assert check(raw[:-1],meta,elf);controls.append('length')
 return dict(passed=True,positive=1,negative_controls=controls,physical_observation=False,limits=['Synthetic consistency tests only; no board execution or capture authenticity.'])
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--self-test',action='store_true');p.add_argument('--capture',type=Path);p.add_argument('--metadata',type=Path);p.add_argument('--elf',type=Path);a=p.parse_args()
 if a.self_test:d=self_test()
 else:
  if not all([a.capture,a.metadata,a.elf]):p.error('capture, metadata and ELF are required')
  e=check(a.capture.read_bytes(),json.loads(a.metadata.read_text()),a.elf.read_bytes());d=dict(passed=not e,errors=e,physical_observation='requires separately verified operator evidence')
 print(json.dumps(d,indent=2));raise SystemExit(not d['passed'])
