"""Canonical additive source and learner-asset attachment for one original task."""
from pathlib import Path
import hashlib,json
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
TITLES={'close':'close()','cancel':'pthread_setcancelstate()','qcc':'q++, qcc','close_dup':'Close duplicate OCB I/O handler','default':'iofunc_close_dup_default()','attach':'resmgr_attach()','dispatch':'dispatch_create()','fstat':'fstat()','install':'SDP8 installation and license prerequisites','qemu':'Quick Start Target Image for QEMU','open':'open()','uname':'uname()'}
def sha(p):return hashlib.sha256((ROOT/p).read_bytes()).hexdigest()
def dump(p,d):
 path=ROOT/p;path.parent.mkdir(parents=True,exist_ok=True);path.write_text(json.dumps(d,ensure_ascii=False,indent=2)+'\n',encoding='utf8')
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
 dump(BASE+'/research_gate.json',dict(status='passage-inspected-author-only',resource_ids=[r['id'] for r in resources],research_input_sha256={p:sha(p) for p in paths},limits=['Target source is authored, not QNX-compiled or executed.','Published C17/C23 access gaps and baseline receipts remain unchanged.']))
 print('PASS12 primary source records; unchanged original criterion and delayed task; public attachment only')
if __name__=='__main__':main()
