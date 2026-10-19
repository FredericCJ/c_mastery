"""Canonical additive source and learner-asset attachment for one original task."""
from pathlib import Path
import argparse,hashlib,json
ROOT=Path(__file__).resolve().parents[1];BASE='research/v5/expansion/alternate_close_depth';LAB='labs/alternate_close_depth'
LOCATORS={
'close':['Synopsis; Description; Returns','Errors: EBADF, EINTR, EIO, ENOSPC, ENOSYS','Safety: cancellation point'],
'cancel':['Synopsis; Description: disable retains pending requests','Returns; Safety'],
'qcc':['Options: -std, -pthread, -V and --version'],
'close_dup':['Prototype; Client functions; Messages','Description: default cleanup, synthesized rcvid0 and serialization'],
'default':['Synopsis; Description; Returns; Safety'],
'attach':['Synopsis and include order','Description: pointer lifetime and PATHSPACE ability','resmgr_attr_t; flags; Returns; Errors; single-threaded example'],
'dispatch':['Synopsis; Description: registration/context ordering and PUBLIC_CHANNEL','Returns; Errors; dispatch-loop example'],
'fstat':['Synopsis; Description','Returns; Errors EBADF and ENOSYS'],
'install':['Target system','Your myQNX account and software license','Installing QNX SDP on your development host'],
'qemu':['Get QNX; Install QEMU'],
'open':['Synopsis; Description: descriptor versus open file description','O_RDWR'],
'uname':['Synopsis; Description: utsname fields','Returns; Safety']}
LOCATORS.update({
'signalkill':['Synopsis and arguments: exact pid/tid, reserved0 and SI_USER','Description: same-user permission, exact-thread delivery and pending signal','Returns; Errors; Safety'],
'sigaction':['Synopsis; Description: process handlers and exact-thread delivery','Returns; Errors; Safety'],
'unblocking':['Received versus reply-blocked request and _NTO_CHF_UNBLOCK','I/O unblock handler and default-first dispatch','MsgInfo pending-flag race check and MsgError(rcvid,-1)'],
'msgerror':['Synopsis; Description: -1 only during unblock, kernel EINTR/ETIMEDOUT','Returns; Errors; Safety'],
'msginfo':['Synopsis: rcvid_t and _msg_info','Description; Returns; Errors; Safety'],
'msg_info':['Synopsis: current32/64 representation','Members pid,tid,scoid,coid and flags _NTO_MI_UNBLOCK_REQ'],
'channel':['Flag _NTO_CHF_UNBLOCK: reply-blocked signal pending until server reply'],
'io_funcs':['Synopsis: close_dup and unblock signatures','Default handlers and lock/unblock coupling'],
'unblock_default':['Synopsis; Description: default-first ancillary handling','Returns _RESMGR_DEFAULT and _RESMGR_NOREPLY; example'],
'sigaction_struct':['Members sa_handler,sa_mask,sa_flags and handler return','Blocked kernel call EINTR and ChannelCreate exception'],
'sigmask':['Synopsis; SIG_UNBLOCK mask operation','Description: asynchronous signals; Returns; Safety'],
'sigemptyset':['Synopsis; Description; Returns'],
'sigaddset':['Synopsis; Description; Returns; Errors']})
TITLES={'close':'close()','cancel':'pthread_setcancelstate()','qcc':'q++, qcc','close_dup':'Close duplicate OCB I/O handler','default':'iofunc_close_dup_default()','attach':'resmgr_attach()','dispatch':'dispatch_create()','fstat':'fstat()','install':'SDP8 installation and license prerequisites','qemu':'Quick Start Target Image for QEMU','open':'open()','uname':'uname()'}
TITLES.update(dict(signalkill='SignalKill()',sigaction='sigaction()',unblocking='Handling client unblocking due to signals or timeouts',msgerror='MsgError()',msginfo='MsgInfo()',msg_info='_msg_info',channel='ChannelCreate() unblock protocol',io_funcs='resmgr_io_funcs_t',unblock_default='iofunc_unblock_default()',sigaction_struct='struct sigaction',sigmask='pthread_sigmask()',sigemptyset='sigemptyset()',sigaddset='sigaddset()'))
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def dump(p,d):
 path=ROOT/p;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8')

def refresh_gate():
 """Refresh only current attachment identities; retain inspected source rows."""
 resources=json.loads((ROOT/BASE/'resources.json').read_text())['resources']
 assert len(resources)==25
 for r in resources:
  c=r['inspection']['capture'];assert sha(c['path'])==c['sha256']
 card='curriculum/v5/expansion/legacy_assessments/PR-T23-02.json'
 d=json.loads((ROOT/card).read_text());v=next(c for c in d['criteria'] if c['objective']=='T23.04.b')['variants'][0]
 assert v['id']=='AX-PR-T23-02-T23.04.b-DELAYED' and v['minimum_days']==7
 previous=json.loads((ROOT/BASE/'completion_dispositions.json').read_text())
 assert next(r for r in previous['updates'] if r['id']==v['id'])['original_task']==v['task']
 gate=json.loads((ROOT/BASE/'research_gate.json').read_text())
 assert gate['resource_ids']==[r['id'] for r in resources]
 assert all(sha(p)==h for p,h in gate['research_input_sha256'].items())
 assert all(p==card or sha(p)==h for p,h in gate['input_sha256'].items())
 gate['input_sha256'][card]=sha(card)
 dump(BASE+'/research_gate.json',gate)
 print('PASS unchanged task/source inspection; current assessment-card binding only')
def main():
 captured=json.loads((ROOT/'build/alternate-close-sources/fetch.json').read_text());resources=[]
 for name,locators in LOCATORS.items():
  c=captured[name];assert c['status']=='fetched' and sha(c['path'])==c['sha256']
  resources.append(dict(id='R-ALT-QNX8-'+name.upper().replace('_','-'),title=TITLES[name],url=c['url'],authors=['QNX'],edition='QNX SDP8 documentation; page updated '+('2026-09-24' if name=='qemu' else '2026-06-11'),year=2026,kind='documentation',access='Named primary passages actually inspected',domains=['T23'],role='primary',assignable=True,reason='Selected alternate close ownership, cancellation, original target adapter and legitimate toolchain boundary.',prerequisites=['T03','T23'],locators=locators,inspection=dict(date='2026-10-18',method='Actual vendor HTML opened through web and downloaded; selected main-content passages read',scope='; '.join(locators),limits='Selected API contract only, not source-code inspection or QNX execution; no older SDK ABI assumed.',capture=dict(path=c['path'],sha256=c['sha256']))))
 dump(BASE+'/resources.json',dict(resources=resources))
 public=[p.relative_to(ROOT).as_posix() for p in sorted((ROOT/LAB).iterdir()) if p.is_file()]
 card='curriculum/v5/expansion/legacy_assessments/PR-T23-02.json';d=json.loads((ROOT/card).read_text());original=json.dumps(d['criteria'],sort_keys=True)
 d['alternate_close_author_support']=dict(criterion='AX-PR-T23-02-T23.04.b-DELAYED',inventory_id='IW-e2cdc87b21fb7b31',path=BASE+'/completion_dispositions.json',scope='Exclusive-owner QNX8 close adaptation and host controls; actual licensed target build/execution remains blocked',source_register=BASE+'/resources.json',supplied_artifacts=public)
 d['supplied_artifacts']=list(dict.fromkeys(d['supplied_artifacts']+public));d['shared_contracts']=list(dict.fromkeys(d['shared_contracts']+[LAB+'/CONTRACTS.md',LAB+'/QNX_TARGET.md']))
 assert json.dumps(d['criteria'],sort_keys=True)==original;dump(card,d)
 paths=[BASE+'/resources.json',LAB+'/CONTRACTS.md',LAB+'/QNX_TARGET.md','assessor/alternate_close_depth/ANALYSIS.md',card]
 dump(BASE+'/research_gate.json',dict(status='passage-inspected',resource_ids=[r['id'] for r in resources],research_input_sha256={BASE+'/resources.json':sha(BASE+'/resources.json')},input_sha256={p:sha(p) for p in paths},limits=['Target source is authored, not QNX-compiled or executed.','Published C17/C23 access gaps and baseline receipts remain unchanged.']))
 print('PASS'+str(len(resources))+' primary source records; unchanged original criterion and delayed task; public attachment only')
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--refresh-gate',action='store_true');a=p.parse_args()
 refresh_gate() if a.refresh_gate else main()
