"""Register only actual bounded primary passages read for PR-T36-02."""
import hashlib,json,pathlib,urllib.request,shutil
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 cache=pathlib.Path('/home/frede/cmastery-algorithm-abi-sources');cache.mkdir(exist_ok=True);rows=[]
 specs=[('SYSV','System V AMD64 ABI Draft0.99.6','https://refspecs.linuxfoundation.org/elf/x86_64-abi-0.99.pdf',['3.2.1 registers, printed page15','3.2.2 stack frame, printed page16','3.2.3 integer argument sequence and register table, printed pages20–21'],'AMD64 ABI authors','Draft0.99.6 2012-07-02',2012,'sysv.pdf'),('ARM','Procedure Call Standard for the Arm Architecture','https://raw.githubusercontent.com/ARM-software/abi-aa/2025Q1/aapcs32/aapcs32.rst',['Core registers lines793–820','Stack constraints lines975–1001','Subroutine calls and linker r12 lines1069–1118','Base parameter allocation rules A.1/A.3/C.4/C.6/C.8 lines1198–1310'],'Arm','AAPCS32 2025Q1',2025,'arm.rst'),('MS','x64 calling convention','https://learn.microsoft.com/en-us/cpp/build/x64-calling-convention?view=msvc-170',['Calling convention defaults and parameter passing','Unwindability: leaf functions','Return values: scalar return','Caller/callee saved registers'],'Microsoft','Microsoft Learn page accessed2026-10-18',2026,'microsoft.html')]
 for key,title,url,locators,author,edition,year,name in specs:
  p=cache/name
  if key=='SYSV':
   source=ROOT/'build/source-cache/amd64-0996.pdf';assert sha(source)=='69e117a13b560b1b957c15f05a61b130ca56828f05dca951e27cb8e2ada14283';shutil.copyfile(source,p)
  elif not p.exists():p.write_bytes(urllib.request.urlopen(url,timeout=45).read())
  rows.append(dict(id='R-AB-'+key,title=title,authors=[author],edition=edition,year=year,kind='primary-specification',url=url,access='Actual listed passages inspected',domains=['T36'],role='primary',reason='Specific scalar ABI register, stack, callback and changed-convention obligations',prerequisites=['C object lifetime and lower-bound invariant'],locators=locators,assignable=True,inspection=dict(date='2026-10-18',method='Actual primary HTML/RST and local pinned PDF text read',scope='; '.join(locators),limits='Only listed passages; no full-document inspection, newest ABI equivalence or Windows OS execution. Direct Arm PDF inaccessible and SysV web PDF timed out; actual accessible alternatives pinned.',external_source_sha256={str(p):sha(p)})))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8');files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='Six exact T36.03/.04 author ordinary/changed tasks; not independent acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['Final published C17/C23 uninspected.','Native Microsoft ABI call is not Windows OS execution; emulator not physical observation.','No independent learner or external assessment.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
