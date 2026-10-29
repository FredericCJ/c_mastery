"""Independent finite state expectations; no reference implementation import."""
import re
def inputs():return map(str,range(4096))
def expected():
 for history in range(4096):
  encoded=history;seq=ack=count=timed=total=calls=0
  for step in range(6):
   action=encoded%4;encoded//=4;out=999
   if action<2:
    result=2 if ack!=seq else 0
    if not result:seq+=1;count=1 if action==0 else 4;timed=0;out=seq
   elif action==2:
    result=1 if not seq else 5 if ack==seq else 0
    if not result:ack=seq;calls+=count;total+=3 if count==1 else 26
   else:
    result=1 if not seq else 0 if ack==seq else 7
    if result==7:timed=1
   yield (history,step,action,result,out,seq,ack,count,timed,total,calls)
def check_model(text):
 rows=[tuple(map(int,line.split())) for line in text.splitlines()]
 want=list(expected())
 if rows!=want:raise ValueError('Finite publication/timeout/work/ack trace disagrees')
 return {'histories':4096,'transitions':len(rows),'alphabet':['publish1','publish4','drain-current','timeout-current']}
def check_guest(text,cpus,mode):
 records=[]
 for line in text.splitlines():
  if line.startswith('SI_CPU '):
   pairs=dict(re.findall(r'(\w+)=([0-9:]+)',line))
   records.append({key:((int(value.split(':')[0])<<32)|int(value.split(':')[1]) if ':' in value else int(value)) for key,value in pairs.items()})
 ids=[0,1,2,4][:cpus]
 if len(records)!=cpus:raise ValueError('Missing/duplicate per-struct lr_cpu record')
 for i,row in enumerate(records):
  if row['arat'] not in (0,1,2) or mode and not row['cpuid1c']&(1<<21):raise ValueError('Missing actual feature observations')
  if row['index']!=i or row['apic']!=ids[i] or row['mode']!=mode or row['one']!=1 or row['periodic'] not in (3,4) or row['ipis']<8 or row['eois']!=row['ipis']+row['periodic']+1 or row['work']!=11 or row['sum']!=155+i or row['ack']!=8 or row['timeout']!=int(cpus>1 and i==cpus-1) or row['handler_ticks']<row['work_ticks'] or row['work_ticks']<=0:raise ValueError('Wrong identity/timer/work/ack/EOI/cost result')
  row['nonwork_handler_ticks']=row['handler_ticks']-row['work_ticks']
 if ('SI_TIMEOUT_RETAIN cpu='+str(cpus-1)+' generation=8 peers='+str(cpus-1) in text)!=(cpus>1):raise ValueError('Missing delayed-struct lr_cpu resource-retention observation')
 if 'SI_GUEST_PASS:' not in text or 'SI_GUEST_CHECK:' in text:raise ValueError('Guest did not complete')
 return {'cpus':cpus,'mode':mode,'records':records,'scope':'TSC ticks under emulator, not calibrated time/WCET; separate work interval from handler total.'}
