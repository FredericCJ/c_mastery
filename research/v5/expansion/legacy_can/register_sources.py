"""Exact inspected SocketCAN documentation and installed ABI header scopes."""
from pathlib import Path
import hashlib,json
ROOT=Path(__file__).resolve().parents[4];HERE=Path(__file__).resolve().parent
SCOPES={'socketcan':['SocketCAN Concept: local loopback and network problem notifications','How to use SocketCAN: classical frame layout and raw DLC; CAN FD comparison; returned message flags','RAW socket defaults and FILTER, ERR_FILTER, LOOPBACK, RECV_OWN_MSGS and FD_FRAMES sections'], 'can.h':['Installed header lines45–195: ID flags/masks, payload/DLC constants and classical/FD structures'],'can/raw.h':['Installed header lines45–110: SOL_CAN_RAW, socket-option enumeration and comments'],'can/error.h':['Installed header lines45–130: error length/class flags and diagnostic byte meanings']}
def dump(p,v):p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')
def main():
 rows=json.loads(Path('C:/Users/frede/.cache/cmastery-sources/socketcan/capture.json').read_text());resources=[]
 for r in rows:
  raw=Path(r['path']).read_bytes();assert hashlib.sha256(raw).hexdigest()==r['sha256'] and len(raw)==r['bytes'];n=r['name'];doc=n=='socketcan'
  resources.append(dict(id='R-CAN-'+n.upper().replace('/','-').replace('.','-'),title='SocketCAN documentation' if doc else 'Installed Linux UAPI '+n,authors=['Linux kernel contributors'],year=2026,edition='docs.kernel.org captured2026-10-17; rolling documentation' if doc else 'linux-libc-dev:amd64 7.0.0-38.38; not the running WSL6.18.33.1 kernel source',kind='manual' if doc else 'source',url=r['url'],access='Actual bounded primary passages read',domains=['T24'],role='core',assignable=True,prerequisites=['Byte spans, integer flags and descriptor ownership'],locators=SCOPES[n],reason='Distinguish classical CAN ABI, frame interpretation, filters and modeled loopback from actual physical evidence',inspection=dict(date='2026-10-17',method='Direct primary HTML and actual installed-header passage reading',scope=SCOPES[n],limits='No published ISO11898, controller/transceiver datasheet, selected physical board or kernel-CAN execution inferred',source_sha256=r['sha256'],source_bytes=r['bytes'],private_capture=r['path'])))
 dump(HERE/'resources.json',dict(schema_version=1,resources=resources))
 name='research/v5/expansion/legacy_can/resources.json';dump(HERE/'research_gate.json',dict(status='passage-inspected',scope='Explicit documentation/header sections only',research_input_sha256={name:hashlib.sha256((ROOT/name).read_bytes()).hexdigest()}))
if __name__=='__main__':main()
