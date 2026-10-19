"""Structural physical-capture check, not authentication or target qualification."""
from pathlib import Path
import argparse,copy,json,re
REQUIRED={'complete-after-cancel','stop-first','fifo-drain','stale-generation','software-reset-domain','cache-visibility'}
def errors(d):
    e=[]
    def need(x,m):
        if not x:e.append(m)
    need(d.get('status')=='observed','physical capture not observed')
    for k in ['board','silicon_revision','qualification_record','instrument','marker_mapping','observer']:need(bool(d.get(k)),k+' missing')
    need(type(d.get('uncertainty_ns')) is int and d['uncertainty_ns']>=0,'uncertainty missing')
    for k in ['source_sha256','raw_sha256']:
        v=d.get(k,{});need(bool(v) and all(isinstance(x,str) and re.fullmatch('[0-9a-f]{64}',x) for x in v.values()),k+' invalid')
    schedules=d.get('schedules',[]);need(len(schedules)==6 and {s.get('id') for s in schedules}==REQUIRED,'required schedules')
    for s in schedules:
        need(bool(s.get('raw_paths')) and set(s.get('raw_paths',[]))<=set(d.get('raw_sha256',{})),'unbound raw capture')
        need(bool(s.get('events')),'empty event trace');last=-1
        for row in s.get('events',[]):
            need(type(row.get('sample')) is int and row['sample']>last,'sample order');last=row.get('sample',last)
            for k in ['epoch','generation','pending_writes','terminal_count']:need(type(row.get(k)) is int and 0<=row[k]<=0xffffffff,k+' invalid')
            for k in ['enabled','visibility_done']:need(type(row.get(k)) is int and row[k] in [0,1],k+' invalid')
            need(row.get('phase') in ['quarantine','cpu','device','stopping','service'],'phase invalid')
            need(bool(row.get('observation_basis')) and bool(row.get('operation')),'unexplained event')
            if row.get('phase') in ['cpu','service']:need(row.get('enabled')==0 and row.get('pending_writes')==0 and row.get('visibility_done')==1,'premature ownership')
            need(row.get('terminal_count') in [0,1],'duplicate terminal')
    return e
def main():
    p=argparse.ArgumentParser();p.add_argument('capture',nargs='?');p.add_argument('--self-test',action='store_true');a=p.parse_args()
    if a.self_test:
        base=dict(status='observed',board='synthetic-not-physical',silicon_revision='test',qualification_record='test',instrument='test',marker_mapping={'test':'test'},observer='test',uncertainty_ns=1,source_sha256={'test':'a'*64},raw_sha256={'raw':'b'*64},schedules=[dict(id=i,raw_paths=['raw'],events=[dict(sample=0,epoch=1,generation=1,phase='service',enabled=0,pending_writes=0,visibility_done=1,terminal_count=1,operation='test',observation_basis='synthetic checker control')]) for i in sorted(REQUIRED)])
        assert not errors(base);controls=[]
        for kind in ['not-run','no-raw','bad-hash','missing-schedule','empty-trace','live-device','pending-write','invisible','duplicate-terminal','missing-basis','uncertainty']:
            x=copy.deepcopy(base);r=x['schedules'][0]['events'][0]
            if kind=='not-run':x['status']='not-run'
            elif kind=='no-raw':x['raw_sha256']={}
            elif kind=='bad-hash':x['source_sha256']['test']='invalid'
            elif kind=='missing-schedule':x['schedules'].pop()
            elif kind=='empty-trace':x['schedules'][0]['events']=[]
            elif kind=='live-device':r['enabled']=1
            elif kind=='pending-write':r['pending_writes']=1
            elif kind=='invisible':r['visibility_done']=0
            elif kind=='duplicate-terminal':r['terminal_count']=2
            elif kind=='missing-basis':r['observation_basis']=''
            else:x['uncertainty_ns']=None
            assert errors(x),kind;controls.append(kind)
        print(json.dumps(dict(passed=True,synthetic_controls=controls,physical_claim=False)));return 0
    d=json.loads(Path(a.capture).read_text());e=errors(d);print(json.dumps(dict(passed=not e,errors=e,limit='Structure only; not authentication, raw hash verification or physical validation.')));return int(bool(e))
if __name__=='__main__':raise SystemExit(main())
