"""Register bounded actual primary inspection; no bibliography-as-reading claim."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[4];HERE=pathlib.Path(__file__).resolve().parent
SOURCES=[
('FORK','fork.2','DESCRIPTION: process context, address space, descriptors and multithreaded child restrictions'),
('EXEC','execve.2','DESCRIPTION: explicit argv/envp, image replacement, retained and removed attributes'),
('DUP','dup.2','DESCRIPTION: shared open file description and separate descriptor flags'),
('SPAWN','posix_spawn.3','DESCRIPTION/file actions; RETURN VALUE/ERRORS; implementation notes'),
('SAFETY','signal-safety.7','DESCRIPTION and async-signal-safe table: close/dup2/execve/fchdir/write/_exit'),
('FSYNC','fsync.2','DESCRIPTION; RETURN VALUE; ERRORS, including EIO and pipe EINVAL'),
('FLOCK','flock.2','DESCRIPTION; NOTES; Linux/network-filesystem differences'),
('CLOCK','clock_gettime.2','DESCRIPTION: MONOTONIC, BOOTTIME, process CPU clock and discontinuities'),
('SLEEP','clock_nanosleep.2','DESCRIPTION; RETURN VALUE; NOTES: absolute deadlines, interruption and dispatch delay'),
('LIMIT','getrlimit.2','DESCRIPTION: soft/hard limits and unprivileged changes'),
('AFFINITY','sched_getaffinity.2','DESCRIPTION; ERRORS EPERM/EINVAL; NOTES: current permitted intersection'),
('FEATURES','feature_test_macros.7','DESCRIPTION: before-header definition and selected feature visibility'),
('PIPE','pipe.2','DESCRIPTION: CLOEXEC and flags; STANDARDS/HISTORY: pipe2 in POSIX.1-2024'),
('OPEN','open.2','DESCRIPTION: open file description, flags and atomic close-on-exec creation'),
('EPOLL','epoll_ctl.2','DESCRIPTION: interest, level/edge flags, read readiness and HUP'),
('POLL','poll.2','DESCRIPTION: readiness, timeout, HUP, invalid descriptors and wrapper distinction'),
('MMAP','mmap.2','DESCRIPTION: mapping lifetime, shared/private and anonymous semantics'),
('THREAD','pthread_create.3','DESCRIPTION; RETURN VALUE; NOTES: creation, inheritance and joinable lifetime'),
('WAIT','waitpid.2','DESCRIPTION: owned child selection and exit/signal interpretation; RETURN VALUE/ERRORS: EINTR, ECHILD and EINVAL; wrapper notes'),
('CLONE','clone.2','SYNOPSIS and introductory DESCRIPTION: wrapper, raw syscall and sharing controls')]
def main():
 rows=[]
 for ident,leaf,locator in SOURCES:
  section=leaf.rsplit('.',1)[1];url='https://man7.org/linux/man-pages/man'+section+'/'+leaf+'.html'
  rows.append(dict(id='R-PCONTEXT-'+ident,title=leaf+' selected contract',authors=['Linux man-pages maintainers'],edition='Linux man-pages6.19 HTML inspected2026-10-17',year=2026,kind='manual',url=url,access='Actual listed passages read',domains=['T23'],role='primary',reason='Process, persistence, clock and capability distinctions in original criteria',prerequisites=['T23 retained topic teaching'],locators=[locator],assignable=True,inspection=dict(date='2026-10-17',method='Actual primary HTML passages read',scope=locator,limits='Bounded passages only; exact published POSIX/ISO text and unexecuted targets remain separate.')))
 rows.append(dict(id='R-PCONTEXT-GLIBC',title='glibc spawn file actions and child error propagation',authors=['GNU C Library maintainers'],edition='Ubuntu source package2.43-2ubuntu2.4; upstream2.42 comparison',year=2026,kind='source',url='https://packages.ubuntu.com/resolute-updates/glibc-source',access='Actual local extracted source and upstream comparison passages read',domains=['T23'],role='primary',reason='Resolve source/runtime versus generic manual failure-reporting distinction',prerequisites=['fork/exec and shared-error lifetime'],locators=['sysdeps/unix/sysv/linux/spawni.c216-307 and351-482: actions, exec failure, clone flags/fallback and parent error collection'],assignable=True,inspection=dict(date='2026-10-17',method='Direct local source read; corresponding upstream2.42 online sections compared',scope='Only named function sections',limits='No complete runtime audit or reproducible source-to-vendor-binary proof.',source_path='/home/frede/cmastery-startup-tools/source/glibc-2.43/sysdeps/unix/sysv/linux/spawni.c',source_sha256='c03cf7328a31d21af4a98b1ed840a414b4d916831fb55ce542b19d9bd8f696de')))
 (HERE/'resources.json').write_text(json.dumps(dict(resources=rows),indent=2)+'\n',encoding='utf8')
 files=[HERE/'resources.json',HERE/'sources.md',pathlib.Path(__file__)]
 (HERE/'research_gate.json').write_text(json.dumps(dict(status='passage-inspected',scope='T23 bounded author support; not independent content acceptance',resource_ids=[r['id'] for r in rows],research_input_sha256={p.relative_to(ROOT).as_posix():hashlib.sha256(p.read_bytes()).hexdigest() for p in files},limitations=['Exact Open Group2024 pages returned403; exact ISO C17 and non-Linux target text remain gaps.','No actual older-kernel, hardware suspend or physical power/cache-loss experiment.','Source and actual vendor runtime identities are recorded separately.']),indent=2)+'\n',encoding='utf8')
if __name__=='__main__':main()
