"""Pin bounded, actually read Linux and POSIX Issue-8 primary passages."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
SCOPES={'epoll':['DESCRIPTION: interest/ready lists, LT/ET example, nonblocking drain and ONESHOT', 'Questions and answers: open-file descriptions, duplication, removal and cached events', 'Possible pitfalls: starvation, explicit ready list and stale event cache'],
 'epoll_ctl':['DESCRIPTION through EPOLLONESHOT: ADD/MOD/DEL, saved event data, readiness/HUP and rearm'],
 'epoll_wait':['DESCRIPTION, RETURN VALUE, ERRORS and NOTES: ready list, MONOTONIC timeout, returned event data, EINTR and round-robin reporting'],
 'pthread_cond_wait':['POSIX Issue8 title/version and complete DESCRIPTION, RETURN VALUE and ERRORS: atomic release/wait, mutex reacquisition, predicate recheck, dynamic binding, cancellation boundary, clock attributes and timeout race']}
def dump(p,v):p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def main():
 capture=Path('C:/Users/frede/.cache/cmastery-sources/reactor/capture.json');rows=json.loads(capture.read_text(encoding='utf8'));resources=[]
 for r in rows:
  data=Path(r['path']).read_bytes();assert hashlib.sha256(data).hexdigest()==r['sha256'] and len(data)==r['bytes'];name=r['name'];posix=name=='pthread_cond_wait'
  resources.append(dict(id='R-REACTOR-'+name.upper(),title='POSIX condition waits' if posix else name+' Linux interface',authors=['IEEE and The Open Group'] if posix else ['Linux man-pages project contributors'],year=2024 if posix else 2026,edition='The Open Group Base Specifications Issue8 / IEEE Std1003.1-2024' if posix else 'Linux man-pages6.19; page-specific date retained',kind='manual',url=r['url'],access='Actual named primary passages read',domains=['T24'],role='core',assignable=True,prerequisites=['C17 lifetime, T18 ownership, T23 descriptor and T25 predicate basics'],locators=SCOPES[name],reason='Distinguish OS notification/lifetime and POSIX synchronization from application scheduling and cancellation policy',inspection=dict(date='2026-10-17',method='Direct primary HTML and selected private-capture text reading; POSIX web-tool403 resolved by ordinary public urllib retrieval, then actual passage inspection',scope=SCOPES[name],limits='No implied linked-standard, whole-kernel, physical target or independent learner inspection',source_sha256=r['sha256'],source_bytes=r['bytes'],private_capture=r['path'])))
 dump(HERE/'resources.json',dict(schema_version=1,resources=resources))
 path='research/v5/expansion/legacy_reactor/resources.json';dump(HERE/'research_gate.json',dict(status='passage-inspected',scope='Named readiness/epoll and POSIX Issue8 condition-wait passages only',research_input_sha256={path:hashlib.sha256((ROOT/path).read_bytes()).hexdigest()}))
if __name__=='__main__':main()
