"""Capture consistency and raw identity, never physical authenticity."""
from pathlib import Path
import argparse,copy,hashlib,json,tempfile
REQUIRED={'singleton','queue-full','delayed-service','rollover','cancel-first','complete-first','stop-first','stale-generation','reset-domain'}
MAX=(1<<64)-1
def interval(c,b,e):
    assert 0<=c['anchor_tick']<=b<=e<=MAX
    assert 0<=c['lower_ns']<=c['upper_ns']<=MAX and 0<c['quantum_ns']<=0xffffffff
    assert 0<=c['rate_error_ppm']<=1000000 and 0<c['epoch']<=0xffffffff
    ns=[(t-c['anchor_tick'])*c['quantum_ns'] for t in [b,e]]
    assert max(ns)<=MAX
    error=[(n*c['rate_error_ppm']+999999)//1000000 for n in ns]
    result=[c['lower_ns']+ns[0]-error[0],c['upper_ns']+ns[1]+error[1]]
    assert 0<=result[0]<=result[1]<=MAX
    return result
def validate(d,base):
    errors=[];analysis=[]
    def need(ok,message):
        if not ok:errors.append(message)
    need(d.get('status')=='observed','physical capture not observed')
    for k in ['board','revision','qualification','marker_definition']:need(bool(d.get(k)),k+' absent')
    h=d.get('firmware_sha256','');need(len(h)==64 and all(x in '0123456789abcdef' for x in h),'firmware identity')
    need(d.get('marker') in ['entry','deferred'],'marker quantity')
    i=d.get('instrument',{});need(bool(i.get('model')) and i.get('sample_rate_hz',0)>0 and i.get('uncertainty_ns',0)>0 and bool(i.get('threshold_and_loading')),'instrument qualification')
    need(REQUIRED<=set(d.get('schedules',[])),'missing schedule')
    raw=d.get('raw_files',[]);need(bool(raw),'raw files absent')
    for r in raw:
        p=(base/r.get('path','')).resolve();need(p.is_file(),'missing raw file')
        if p.is_file():need(hashlib.sha256(p.read_bytes()).hexdigest()==r.get('sha256'),'raw file identity')
    a=d.get('accounting',{});values=[a.get(k,-1) for k in ['source','accepted','dropped','outstanding']]
    need(all(type(n) is int and n>=0 for n in values),'invalid counts')
    need(values[0]==sum(values[1:]),'event conservation')
    rows=d.get('rows',[]);need(bool(rows) and len(rows)==a.get('accepted'),'accepted row count');seen=set()
    for row in rows:
        try:
            identity=(row['epoch'],row['sequence']);assert identity not in seen;seen.add(identity)
            assert row['epoch']==d['calibration']['epoch'] and 0<=row['sequence']<=0xffffffff
            sample=interval(d['calibration'],row['begin_tick'],row['end_tick'])
            d0,d1=row['delay_ns'];assert 0<=d0<=d1<=sample[0]
            event=[sample[0]-d1,sample[1]-d0];o0,o1=row['event_ns'];assert 0<=o0<=o1<=MAX
            assert o1-o0>=2*i['uncertainty_ns']
            assert max(event[0],o0)<=min(event[1],o1)
            analysis.append(dict(sequence=row['sequence'],sample_ns=sample,event_ns=event,observation_ns=[o0,o1]))
        except (KeyError,TypeError,ValueError,AssertionError):errors.append('invalid/inconsistent row')
    need(bool(d.get('limits')),'limits absent');return errors,analysis
def self_test():
    with tempfile.TemporaryDirectory() as t:
        base=Path(t);p=base/'synthetic.txt';p.write_text('synthetic checker control, not a physical capture\n')
        d=dict(status='observed',board='synthetic',revision='synthetic',qualification='synthetic',firmware_sha256='1'*64,marker='entry',marker_definition='synthetic',instrument=dict(model='synthetic',sample_rate_hz=100000000,uncertainty_ns=1,threshold_and_loading='synthetic'),raw_files=[dict(path=p.name,sha256=hashlib.sha256(p.read_bytes()).hexdigest())],calibration=dict(anchor_tick=100,lower_ns=1000,upper_ns=1010,quantum_ns=10,rate_error_ppm=10000,epoch=7),accounting=dict(source=4,accepted=1,dropped=3,outstanding=0),schedules=sorted(REQUIRED),rows=[dict(epoch=7,sequence=6,begin_tick=105,end_tick=105,delay_ns=[2,9],event_ns=[1042,1048])],limits=['synthetic only'])
        assert validate(d,base)[0]==[];assert validate(d,base)[1][0]['event_ns']==[1040,1059]
        controls=[]
        for kind in ['unobserved','raw-hash','conservation','epoch','reversed','underflow','calibration','missing-schedule','marker','instrument','overflow','duplicate']:
            x=copy.deepcopy(d)
            if kind=='unobserved':x['status']='unobserved'
            elif kind=='raw-hash':x['raw_files'][0]['sha256']='0'*64
            elif kind=='conservation':x['accounting']['source']=3
            elif kind=='epoch':x['rows'][0]['epoch']=8
            elif kind=='reversed':x['rows'][0]['end_tick']=104
            elif kind=='underflow':x['rows'][0]['delay_ns']=[0,2000]
            elif kind=='calibration':x['rows'][0]['event_ns']=[0,2]
            elif kind=='missing-schedule':x['schedules'].remove('reset-domain')
            elif kind=='marker':x['marker']='unspecified'
            elif kind=='instrument':x['instrument']['uncertainty_ns']=0
            elif kind=='overflow':x['calibration']['upper_ns']=MAX
            else:x['rows'].append(x['rows'][0]);x['accounting'].update(accepted=2,source=5)
            rejected=bool(validate(x,base)[0]);controls.append(dict(control=kind,rejected=rejected));assert rejected,kind
        return dict(passed=True,physical_claim=False,negative_controls=controls)
def main():
    p=argparse.ArgumentParser();p.add_argument('capture',nargs='?',type=Path);p.add_argument('--self-test',action='store_true');a=p.parse_args()
    if a.self_test:d=self_test()
    else:
        assert a.capture;errors,rows=validate(json.loads(a.capture.read_text()),a.capture.parent);d=dict(passed=not errors,errors=errors,analysis=rows,limits=['Structural consistency and raw hashes do not authenticate physical observations or claimed bounds.'])
    print(json.dumps(d,indent=2));return int(not d['passed'])
if __name__=='__main__':raise SystemExit(main())
