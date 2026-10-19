"""Register actually read counter and timer passages, with exact capture identities."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
SCOPES={'timerfd_create':['DESCRIPTION: clock choices, nonblocking/CLOEXEC, absolute arm/disarm, periodic intervals, gettime and native counter reads','RETURN VALUE and ERRORS: operation failures and invalid nanoseconds; STANDARDS: Linux'], 'eventfd':['DESCRIPTION: native64-bit ordinary/semaphore consumption, short reads, saturation, invalid writes and descriptor lifetime','RETURN VALUE, ERRORS and VERSIONS: eventfd2/glibc wrapper; STANDARDS: Linux/GNU']}
def dump(p,v):p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def main():
 rows=json.loads(Path('C:/Users/frede/.cache/cmastery-sources/timers/capture.json').read_text());resources=[]
 for r in rows:
  raw=Path(r['path']).read_bytes();assert hashlib.sha256(raw).hexdigest()==r['sha256'] and len(raw)==r['bytes'];n=r['name']
  resources.append(dict(id='R-TIMER-'+n.upper(),title=n+' Linux counter interface',authors=['Linux man-pages project contributors'],year=2026,edition='Linux man-pages6.19, page date2026-02-16',kind='manual',url=r['url'],access='Actual named primary passages read',domains=['T24'],role='core',assignable=True,prerequisites=['T23 descriptor lifetime and monotonic waits','T24 readiness and T25 mutex predicates'],locators=SCOPES[n],reason='Separate OS counter behavior from application timer phase, callback identity and cancellation policy',inspection=dict(date='2026-10-17',method='Actual primary HTML DESCRIPTION/RETURN/ERRORS and selected VERSIONS/STANDARDS read; byte-identical private capture retained',scope=SCOPES[n],limits='No physical clock measurement, kernel internals or published ISO edition inspection inferred',source_sha256=r['sha256'],source_bytes=r['bytes'],private_capture=r['path'])))
 shared='research/v5/expansion/legacy_reactor/resources.json'
 dump(HERE/'resources.json',dict(schema_version=1,resources=resources,canonical_external_sources=[dict(path=shared,id=i) for i in ['R-REACTOR-EPOLL','R-REACTOR-EPOLL_CTL','R-REACTOR-EPOLL_WAIT','R-REACTOR-PTHREAD_COND_WAIT']]))
 source='research/v5/expansion/legacy_timers/resources.json'
 dump(HERE/'research_gate.json',dict(status='passage-inspected',scope='Named Linux counter passages and explicitly shared previously inspected epoll/POSIX condition-wait passages',research_input_sha256={p:hashlib.sha256((ROOT/p).read_bytes()).hexdigest() for p in [source,shared]}))
if __name__=='__main__':main()
