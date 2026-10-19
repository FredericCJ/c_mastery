"""Register only the primary passages actually read for the predicate wave."""
import hashlib,json,pathlib
HERE=pathlib.Path(__file__).resolve().parent;ROOT=HERE.parents[3]
SOURCES=[
 ('COND','Condition predicates and cancellation','https://man7.org/linux/man-pages/man3/pthread_cond_wait.3p.html','DESCRIPTION32–120; ERRORS150–167; RATIONALE193–237,244–285,325–357'),
 ('CLOCK','Condition clock attribute','https://man7.org/linux/man-pages/man3/pthread_condattr_getclock.3p.html','DESCRIPTION32–47; RETURN VALUE49–57; ERRORS60–65'),
 ('DESTROY','Mutex initialization and destruction lifetime','https://man7.org/linux/man-pages/man3/pthread_mutex_destroy.3p.html','DESCRIPTION32–70; RATIONALE106–183'),
 ('CANCEL','Cancellation state and asynchronous safety','https://man7.org/linux/man-pages/man3/pthread_setcancelstate.3.html','DESCRIPTION29–68; ATTRIBUTES89–96; NOTES110–148'),
 ('BARRIER','GNU libc POSIX barriers','https://sourceware.org/glibc/manual/latest/html_node/POSIX-Barriers.html','36.2.5 complete section: initialization, repeated batches, wait return and destruction'),
 ('ONCE','One-time initialization contract','https://man7.org/linux/man-pages/man3/pthread_once.3p.html','DESCRIPTION30–50; APPLICATION USAGE71–77'),
 ('RWLOCK','Read/write exclusion and scheduling qualifications','https://man7.org/linux/man-pages/man3/pthread_rwlock_rdlock.3p.html','DESCRIPTION30–81; ERRORS94–112'),
 ('SEMAPHORE','Semaphore admission and failure','https://man7.org/linux/man-pages/man3/sem_trywait.3p.html','DESCRIPTION29–45; RETURN VALUE48–53; ERRORS56–68'),
 ('INTERCEPTOR','LLVM21.1 cancellation state interceptor','https://raw.githubusercontent.com/llvm/llvm-project/llvmorg-21.1.0/compiler-rt/lib/sanitizer_common/sanitizer_common_interceptors.inc','Lines6381–6400: pthread_setcancelstate/type interceptors and registration'),
 ('TSANATTR','Clang ThreadSanitizer instrumentation attributes','https://clang.llvm.org/docs/ThreadSanitizer.html','Sections no_sanitize(thread) and disable_sanitizer_instrumentation, lines215–220')]
def main():
 rows=[]
 for key,title,url,locator in SOURCES:
  posix=key not in ['CANCEL','BARRIER','INTERCEPTOR','TSANATTR']
  author='IEEE and The Open Group' if posix else 'LLVM Project contributors' if key in ['INTERCEPTOR','TSANATTR'] else 'GNU C Library maintainers' if key=='BARRIER' else 'Linux man-pages maintainers'
  rows.append(dict(id='R-TWAIT-'+key,title=title,authors=[author],edition='POSIX.1-2017 reprint, Issue7/2018' if posix else 'LLVM21.1.0' if key=='INTERCEPTOR' else 'Primary HTML inspected2026-10-17',year=2017 if posix else 2026,kind='standard-reprint' if posix else 'source-code' if key=='INTERCEPTOR' else 'manual',url=url,access='Actual listed primary passages read',domains=['T25'],role='primary',reason='Predicate, synchronization lifetime and cancellation contract needed by the original tasks',prerequisites=['T25.01 thread lifetime','T25.02 shared ownership'],locators=[locator],assignable=True,inspection=dict(date='2026-10-17',method='Direct primary HTML/source passage reading',scope=locator,limits='Reprint and selected Linux/GNU/LLVM scope; not final ISO C editions or POSIX.1-2024. No independent reviewer claim.')))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'sources.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T25.03–T25.05 author source evidence, not content acceptance or independent review',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Exact published ISO C17/C23 and POSIX.1-2024 are not claimed inspected.','Failed alternate URLs are listed in sources.md; source access is not implementation execution.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
