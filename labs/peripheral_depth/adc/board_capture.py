"""Structural capture validation only; provenance remains a human obligation."""
import json,re,sys

def validate(d):
 errors=[]
 def need(ok,msg):
  if not ok:errors.append(msg)
 def number(x):return isinstance(x,(int,float)) and not isinstance(x,bool) and x==x and abs(x)!=float('inf')
 need(d.get('schema_version')==1,'schema')
 need(d.get('provenance') in ('physical','synthetic-test'),'provenance')
 need(d.get('board') in ('NUCLEO-F439ZI','NUCLEO-G474RE'),'board')
 need(bool(d.get('silicon_revision')),'silicon revision')
 need(bool(re.fullmatch('[0-9a-f]{64}',str(d.get('image_sha256')))),'image identity')
 artifacts=d.get('raw_artifacts',[]);need(bool(artifacts),'raw artifacts')
 for a in artifacts:need(bool(a.get('path')) and bool(re.fullmatch('[0-9a-f]{64}',str(a.get('sha256')))),'raw identity')
 supply=d.get('vd_da_mv');reference=d.get('reference_mv');need(number(supply) and 2700<=supply<=3600,'selected supply profile');need(number(reference) and number(supply) and 0<reference<=supply,'reference')
 t=d.get('timebase',{});tick=t.get('tick_ns');quant=t.get('quantization_ns');jitter=t.get('jitter_ns');need(number(tick) and tick>=1000 and number(quant) and 0<=quant<tick and number(jitter) and jitter>=0,'timebase premises')
 need(d.get('electrical_isolation_confirmed') is True,'electrical isolation')
 rows=d.get('observations',[]);need(bool(rows),'observations');last=-1
 for r in rows:
  ns=r.get('time_ns');need(number(ns) and ns>=0 and ns>=last,'monotonic capture')
  if number(ns):last=ns
  kind=r.get('kind');need(kind in ('adc','dac','comparator'),'observation kind')
  if kind in ('adc','dac'):
   code=r.get('code');need(type(code) is int and 0<=code<=4095,'12-bit code')
   mv=r.get('measured_mv');err=r.get('measurement_error_mv');need(number(mv) and number(supply) and 0<=mv<=supply and number(err) and err>=0,'independent voltage/uncertainty')
  else:
   need(d.get('board')=='NUCLEO-G474RE','unsupported comparator')
   need(r.get('direction') in ('rising','falling') and type(r.get('level')) is int and r['level'] in (0,1),'comparator level/direction')
   lo=r.get('input_low_mv');hi=r.get('input_high_mv');need(number(lo) and number(hi) and number(supply) and 0<=lo<=hi<=supply,'threshold bracket')
 return dict(passed=not errors,errors=errors,scope='Format/premise checks only; no authenticity or physical-accuracy verdict.')
if __name__=='__main__':
 r=validate(json.load(open(sys.argv[1],encoding='utf8')));print(json.dumps(r));raise SystemExit(not r['passed'])
