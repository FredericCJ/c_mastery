"""Record the actual bounded primary passage inspection, without redistributing pages."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[4]
HERE=Path(__file__).resolve().parent
SCOPES={
 'recvmsg':['DESCRIPTION: partial/nonblocking reception; input MSG_TRUNC, MSG_WAITALL and MSG_CMSG_CLOEXEC', 'recvmsg msghdr/control fields and returned MSG_TRUNC, MSG_CTRUNC and MSG_CMSG_CLOEXEC', 'RETURN VALUE, EAGAIN/EWOULDBLOCK/EINTR and NOTES: stream EOF versus consumption of zero-length datagrams'],
 'shutdown':['DESCRIPTION, RETURN VALUE, ERRORS, STANDARDS, NOTES and BUGS: directional shutdown and descriptor lifetime distinction'],
 'sendmsg':['DESCRIPTION: local completion is not delivery, nonblocking and per-call MSG_NOSIGNAL', 'sendmsg fields/control, RETURN VALUE and relevant EAGAIN/EINTR/EPIPE errors'],
 'unix':['Sockets API: buffer behavior and MSG_TRUNC', 'Ancillary messages SCM_RIGHTS: descriptor references, truncation, excess-right closure and RLIMIT_NOFILE', 'CMSG storage/length rules and Linux-specific limits']}

def dump(p,v):p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def main():
 capture=Path('C:/Users/frede/.cache/cmastery-sources/framing/capture.json')
 rows=json.loads(capture.read_text(encoding='utf8'));resources=[]
 for row in rows:
  data=Path(row['path']).read_bytes();assert hashlib.sha256(data).hexdigest()==row['sha256'] and len(data)==row['bytes']
  name=row['name'];resources.append(dict(id='R-FRAMING-'+name.upper(),title=name+' Linux man-pages interface',authors=['Linux man-pages project contributors'],year=2026,edition='man-pages 6.19; captured 2026-10-17; page-specific version/date retained',kind='manual',url=row['url'],access='Actual named primary passages read; complete linked corpus not inferred',domains=['T24'],role='core',assignable=True,prerequisites=['C17 representations, T18 ownership and T23 descriptor lifetime'],locators=SCOPES[name],reason='Exact framing, directional shutdown, local-send and ancillary-data contracts',inspection=dict(date='2026-10-17',method='Direct primary HTML passage reading followed by private original capture',scope=SCOPES[name],limits='No implied published ISO/POSIX text inspection, physical network validation or third-party target interaction',source_sha256=row['sha256'],source_bytes=row['bytes'],private_capture=row['path'])))
 dump(HERE/'resources.json',dict(schema_version=1,resources=resources))
 path='research/v5/expansion/legacy_framing/resources.json'
 dump(HERE/'research_gate.json',dict(status='passage-inspected',scope='Only named T24.05/.06 Linux primary passages; no linked-standard or network-stack survey claim',research_input_sha256={path:hashlib.sha256((ROOT/path).read_bytes()).hexdigest()}))
if __name__=='__main__':main()
