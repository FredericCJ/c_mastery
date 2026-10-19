"""Pin actual inspected OS, MPU and retained producer passages."""
import hashlib,json,pathlib,urllib.request
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 cache=pathlib.Path('/home/frede/cmastery-security-sources');cache.mkdir(exist_ok=True)
 external={}
 for name in ['open','execve','write','close_range','openat2']:
  p=cache/(name+'-6.19.html')
  if not p.exists():p.write_bytes(urllib.request.urlopen('https://man7.org/linux/man-pages/man2/'+name+'.2.html',timeout=30).read())
  external[str(p)]=sha(p)
 specs=[('OS','Linux man-pages6.19 descriptor, argv and resolution contracts','https://man7.org/linux/man-pages/man2/open.2.html',['open DESCRIPTION: open-file identity/access modes and O_CLOEXEC/O_EXCL/O_NOFOLLOW','execve DESCRIPTION: explicit argv and envp','write ERRORS: EBADF','close_range DESCRIPTION: CLOSE_RANGE_CLOEXEC','openat2 DESCRIPTION: RESOLVE_BENEATH/NO_SYMLINKS/NO_MAGICLINKS and ERRORS'],external),('MPU','ST PM0214 Cortex-M4 MPU selected permission design','https://www.st.com/resource/en/programming_manual/pm0214-stm32-cortexm4-mcus-and-mpus-programming-manual-stmicroelectronics.pdf',['PM0214 revision10 pp.195–199: AP/XN, update barriers, subregions and CPU/DMA distinction','PM0214 revision10 pp.200–205: region count, control/background, RBAR alignment and RASR'],{}),('PRODUCERS','Actual frozen pathname and MPU producers','https://github.com/FredericCJ/c_mastery',['labs/posix_lifecycle_depth/namespace/path.h and CONTRACTS.md: complete retained interface/contract','assessor/posix_lifecycle_depth/namespace/path.c: pn_open_confined complete implementation','assessor/firmware_lifecycle/targets/boot_handoff.c: complete region programming and handoff','labs/firmware_lifecycle/targets/boot_guest.S: original application startup and two MPU fault variants','research/v5/expansion/b06/boot_pair_validation.json: twelve actual CPU execute observations and current input hashes','assessor/hosted_systems/descriptor.c: complete hs_dup_owned'],{})]
 rows=[]
 for key,title,url,locators,pins in specs:
  inspection=dict(date='2026-10-18',method='Official primary documentation and direct implementation/receipt reads; private PM0214 pypdf extraction',scope='; '.join(locators),limits='Listed passages only; prior MPS2 execution is not new CPU/physical validation. No complete sandbox or final published C17/C23 claim.',external_source_sha256=pins)
  if key=='MPU':inspection['private_payload']='build/execution-sources/PM0214-r10.pdf';inspection['private_payload_sha256']=sha(ROOT/inspection['private_payload'])
  rows.append(dict(id='R-SD-'+key,title=title,authors=['STMicroelectronics'] if key=='MPU' else ['Linux man-pages contributors'] if key=='OS' else ['C Mastery authors'],edition='PM0214 revision10 March2020' if key=='MPU' else '6.19' if key=='OS' else 'Current frozen producers',year=2020 if key=='MPU' else 2026,kind='implementation-documentation',url=url,access='Listed actual passages inspected',domains=['T35'],role='primary',reason='Separate OS descriptor authority, application policy and selected CPU MPU enforcement',prerequisites=['T23 descriptors/processes','T35 threat boundaries'],locators=locators,assignable=True,inspection=inspection))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8');files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T35.05/.08 bounded author source inspection',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['Actual OS observations are local and narrow.','Prior MPU CPU observations are not physical STM32 evidence.','Independent review/learner and final published C17/C23 remain separate.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
