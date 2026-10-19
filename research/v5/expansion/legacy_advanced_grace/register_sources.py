"""Record actual retained contracts and pinned Linux RCU passage inspection."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
KERNEL=pathlib.Path('/home/frede/cmastery-b06-tools/debug-kernel/linux-source-7.0.0')
LINUX_FILES=['Documentation/RCU/whatisRCU.rst','Documentation/RCU/rcubarrier.rst','include/linux/rcupdate.h','include/linux/hrtimer.h']
def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def main():
 rows=[]
 for key,title,url,locators,method in [
 ('LINUX','Pinned Linux RCU, grace periods, callbacks and hard-interrupt timers','https://docs.kernel.org/RCU/whatisRCU.html',['Documentation/RCU/whatisRCU.rst: core APIs, lines150–250 and390–435','include/linux/rcupdate.h: rcu_read_lock contract, lines800–845','Documentation/RCU/rcubarrier.rst: unloadable module callback ownership, lines1–65','include/linux/hrtimer.h: HARD and PINNED mode contract, lines23–55'],'Direct exact local Linux source passage reads; online URL is discovery, exact inspected content pinned below'),
 ('CMX','Retained CMX epoch and coarse RCU component contracts','https://github.com/FredericCJ/c_mastery',['labs/execution/README.md: global ownership and T55 paragraph','assessor/execution/concurrent.c: cmx_epoch_init/enter/leave/advance','assessor/execution/advanced.c: cmx_rcu_init/read/leave/publish/quiescent','labs/execution/targets/README.md: strict build and original byte support paragraph','assessor/execution/support.c: original byte loops and AEABI aliases'],'Direct repository contract and complete producer function reading'),
 ('API','Actual callback registry ownership and deferred drop producer','https://github.com/FredericCJ/c_mastery',['labs/api_depth/CONTRACTS.md: api_depth.registry and Assessment and limitations','assessor/api_depth/api_depth.c: api_registry_init, add/remove/dispatch/destroy and release_slot'],'Direct repository contract and complete registry implementation reading')]:
  rows.append(dict(id='R-AG-'+key,title=title,authors=['Linux contributors'] if key=='LINUX' else ['C Mastery curriculum authors'],edition='Linux7.0.14-cmastery-b06 selected source' if key=='LINUX' else 'Current frozen retained producer',year=2026,kind='implementation-documentation' if key=='LINUX' else 'repository-source',url=url,access='Exact listed passages inspected',domains=['T26'],role='primary',reason='Separate publication, grace, references, callback completion and execution contexts',prerequisites=['T25 synchronization','T19 callback ownership','T26 retirement'],locators=locators,assignable=True,inspection=dict(date='2026-10-17',method=method,scope='; '.join(locators),limits='Selected source/configuration only; user-space model is not Linux RCU; no final C17/C23 source or physical observation claimed.')))
 rows[0]['inspection']['external_source_sha256']={str(KERNEL/p):sha(KERNEL/p) for p in LINUX_FILES}
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'audit.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T26.07/.08 author source inspection only',resource_ids=[x['id'] for x in rows],research_input_sha256={p.relative_to(ROOT).as_posix():sha(p) for p in files},limitations=['Exact selected Linux configuration and provider contracts govern these experiments.','No independent learner/review/physical evidence or final C17/C23 inspection.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
