from pathlib import Path
import copy,json,sys
sys.path.insert(0,str(Path(__file__).resolve().parents[3]/'labs/peripheral_depth/adc'))
from analog_analysis import settling,threshold_interval,timestamp_interval
from board_capture import validate as capture
def main():
 rows=[]
 # Independently bounded ln(16384)=14 ln2, 0.6931471805599453<ln2<...54.
 for r,expected in [(1000,2),(10000,16),(50000,78),(56000,87),(1000000,1553)]:
  result=settling(r,10,12,0.25,16000000);assert result['required_cycles']==expected
  rows.append(dict(kind='RC',resistance=r,**result))
 assert rows[0]['selected_cycles']==3 and rows[1]['selected_cycles']==28 and rows[2]['selected_cycles']==84 and rows[3]['selected_cycles']==112 and rows[4]['selected_cycles'] is None
 # Source resistance is only part of a physical path: conservative synthetic
 # 50k external +6k switch, 10pF declared ->112cycles, unlike 50k alone ->84.
 old=threshold_interval(1650,2,-20,3,11,47);new=threshold_interval(1650,2,-9,3,11,47)
 assert old['enclosure_mv']==[1581,1702] and new['enclosure_mv']==[1592,1702]
 assert timestamp_interval(100,103,1000,200,31)==[99769,104200]
 rejected=0
 for fn,args in [(settling,(0,10,12,.25,16000000)),(settling,(1,10,12,0,16000000)),(threshold_interval,(1650,-1,-9,3,11,47)),(threshold_interval,(1650,1,3,-9,11,47)),(timestamp_interval,(4,3,1000,0,31)),(timestamp_interval,(1,2,0,0,31))]:
  try:fn(*args)
  except ValueError:rejected+=1
 assert rejected==6
 sample=dict(schema_version=1,provenance='synthetic-test',board='NUCLEO-G474RE',silicon_revision='Y',image_sha256='1'*64,raw_artifacts=[dict(path='synthetic-not-a-board.csv',sha256='2'*64)],vd_da_mv=3300,reference_mv=1650,timebase=dict(tick_ns=1000,quantization_ns=999,jitter_ns=200),electrical_isolation_confirmed=True,observations=[dict(kind='adc',time_ns=0,code=2048,measured_mv=1650,measurement_error_mv=2),dict(kind='comparator',time_ns=1000,direction='rising',level=1,input_low_mv=1640,input_high_mv=1660)])
 assert capture(sample)['passed'];bad=[]
 for key,value in [('provenance',None),('image_sha256','bad'),('raw_artifacts',[]),('vd_da_mv',2500),('electrical_isolation_confirmed',False),('observations',[])]:
  x=copy.deepcopy(sample);x[key]=value;assert not capture(x)['passed'];bad.append(key)
 for field,value in [('tick_ns',999),('quantization_ns',1000),('jitter_ns',-1)]:
  x=copy.deepcopy(sample);x['timebase'][field]=value;assert not capture(x)['passed'];bad.append(field)
 for field,value in [('code',4096),('time_ns',-1),('measured_mv',float('nan'))]:
  x=copy.deepcopy(sample);x['observations'][0][field]=value;assert not capture(x)['passed'];bad.append(field)
 assert not capture(json.loads((Path(__file__).resolve().parents[3]/'labs/peripheral_depth/adc/capture_template.json').read_text()))['passed']
 return dict(passed=True,settling=rows,old_revision=old,new_revision=new,invalid_models_rejected=rejected,invalid_captures_rejected=len(bad)+1,limits=['Synthetic computations only; hysteresis min/max and offset are different sources of uncertainty.','31ns applies only to the stated comparator input step/overdrive/load, not arbitrary slow ramps.'])
if __name__=='__main__':print(json.dumps(main()))
