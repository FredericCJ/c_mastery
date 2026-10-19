"""Independent bounded raw-octet and curl peers against an owned loopback child."""
import argparse,datetime,email.utils,json,os,select,socket,subprocess,time
from pathlib import Path
def main(binary,capacity,date_mode,report):
 report=Path(report);report.parent.mkdir(parents=True,exist_ok=True);err=report.with_suffix('.stderr.txt')
 records=[]
 with err.open('wb') as errors:
  child=subprocess.Popen([binary,str(capacity),str(date_mode)],stdout=subprocess.PIPE,stderr=errors,text=True)
  try:
   assert select.select([child.stdout],[],[],5)[0],'No owned server readiness'
   ready=child.stdout.readline().strip();authority=ready.split()[1];host,port=authority.split(':');endpoint=(host,int(port))
   def conn():
    s=socket.create_connection(endpoint,3);s.settimeout(3);return s
   def req(method='GET',path='/health',fields=b'',body=b''):
    return f'{method} {path} HTTP/1.1\r\nHost: {authority}\r\n'.encode()+fields+b'\r\n'+body
   class Reader:
    def __init__(self,s):self.s=s;self.buffer=b'';self.wire=b''
    def read(self,n):
     while len(self.buffer)<n:
      b=self.s.recv(4096);assert b,'Unexpected EOF';self.buffer+=b;self.wire+=b
     b,self.buffer=self.buffer[:n],self.buffer[n:];return b
    def response(self,head=False):
     header=b''
     while not header.endswith(b'\r\n\r\n'):header+=self.read(1);assert len(header)<=1600
     lines=header[:-4].split(b'\r\n');status=int(lines[0].split()[1]);fields=dict(x.split(b': ',1) for x in lines[1:]);n=0 if head or status==100 else int(fields[b'Content-Length']);body=self.read(n)
     if status!=100:
      if date_mode==0:assert b'Date' not in fields
      elif date_mode==2:assert fields[b'Date']==b'Sun, 01 Jan 2023 00:00:00 GMT'
      else:assert abs(email.utils.parsedate_to_datetime(fields[b'Date'].decode()).timestamp()-time.time())<5
     return status,fields,body
   def save(name,r,expected):
    records.append({'case':name,'response_wire_hex':r.wire.hex(),'expected':expected,'passed':True})
   with conn() as s:
    r=Reader(s);payload=req()+req('HEAD','/unknown')+req('HEAD','/echo',b'Connection: close\r\n');s.sendall(payload)
    assert r.response()[2]==b'ok\n';status,fields,body=r.response(True);assert (status,fields[b'Content-Length'],body)==(404,b'10',b'')
    status,fields,body=r.response(True);assert (status,fields[b'Content-Length'],fields[b'Allow'],body)==(405,b'19',b'POST',b'');save('head-pipeline',r,[200,404,405])
   with conn() as s:
    r=Reader(s);binary=b'\x00\x80\xff\r\n';s.sendall(req()+req('POST','/echo',b'Expect: 100-continue\r\nContent-Length: 5\r\n'))
    assert r.response()[2]==b'ok\n';assert r.response()[0]==100;s.sendall(binary);assert r.response()[2]==binary;save('ordered-100-binary',r,[200,100,200])
   for complete in (False,True):
    with conn() as s:
     r=Reader(s);s.sendall(req('POST','/echo',b'Content-Length: 5\r\n',b'abcde' if complete else b'abcd'));s.shutdown(socket.SHUT_WR)
     status,_,body=r.response();assert status==(200 if complete else 400);assert not complete or body==b'abcde';save('half-close-'+str(complete),r,[status])
   for name,wire,status in [('te-cl',req('POST','/echo',b'Content-Length: 0\r\nTransfer-Encoding: chunked\r\n')+req(),400),('upgrade',req(fields=b'Upgrade: demo\r\n')+req(),501),('connect',f'CONNECT {authority} HTTP/1.1\r\nHost: {authority}\r\n\r\n'.encode()+req(),501),('duplicate',req('POST','/echo',b'Content-Length: 0\r\nContent-Length: 0\r\nExpect: 100-continue\r\n')+req(),400)]:
    with conn() as s:
     r=Reader(s);s.sendall(wire);assert r.response()[0]==status;assert r.buffer==b'' and s.recv(4096)==b'';save(name+'-reject-and-close',r,[status])
   with conn() as s:
    r=Reader(s);s.sendall(req()+req(fields=b'Connection: close\r\n')+req());assert r.response()[0]==200 and r.response()[0]==200;assert not r.buffer and s.recv(4096)==b'';save('close-second-drops-third',r,[200,200])
   with conn() as s:
    r=Reader(s);s.sendall(req()*5)
    for i in range(4):
     status,fields,body=r.response();assert status==200 and body==b'ok\n';assert fields[b'Connection']==(b'close' if i==3 else b'keep-alive')
    assert not r.buffer and s.recv(4096)==b'';save('maximum-four',r,[200]*4)
   # Small read buffer requires partial header retention; binary chunk bytes sent
   # individually are still independent of actual TCP segmentation.
   with conn() as s:
    r=Reader(s);wire=req('POST','/echo',b'Transfer-Encoding: chunked\r\n',b'3;x="a\\\"b"\r\na\x00b\r\n0\r\nX-End: ignored\r\n\r\n')
    for b in wire:s.sendall(bytes([b]))
    assert r.response()[2]==b'a\0b';save('chunk-byte-writes',r,[200])
   # Independent mature client: raw headers are preserved as actual artifacts.
   for name,args,body in [('get',[],b'ok\n'),('head',['--head'],None),('binary',['--data-binary','@-'],b'a\0\xff'),('expect',['-H','Expect: 100-continue','--data-binary','@-'],b'a\0\xff')]:
    url='http://'+authority+('/echo' if name in ('binary','expect') else '/health');cmd=['curl','--silent','--show-error','--max-time','5','--http1.1','--include',*args,url]
    run=subprocess.run(cmd,input=b'a\0\xff' if name in ('binary','expect') else None,capture_output=True,timeout=8);assert run.returncode==0,(name,run.stderr)
    if body is not None:assert run.stdout.endswith(body)
    if name=='expect':assert b'HTTP/1.1 100 Continue\r\n\r\nHTTP/1.1 200 OK' in run.stdout
    records.append({'case':'curl-'+name,'command':cmd,'exit_code':run.returncode,'response_wire_hex':run.stdout.hex(),'stderr':run.stderr.decode(),'passed':True})
   # Let earlier peer EOF notifications finish, then measure four live slots.
   time.sleep(.1);held=[conn() for _ in range(4)]
   try:
    for s in held:s.sendall(b'GET ')
    time.sleep(.05)
    with conn() as fifth:assert fifth.recv(1)==b''
    records.append({'case':'fifth-connection-refused','passed':True})
   finally:
    for s in held:s.close()
   time.sleep(.1)
   with conn() as s:
    r=Reader(s);s.sendall(req(fields=b'Connection: close\r\n'));assert r.response()[0]==200;assert s.recv(1)==b'';before=len(list(Path(f'/proc/{child.pid}/fd').iterdir()))
    start=time.monotonic();time.sleep(1.15);after=len(list(Path(f'/proc/{child.pid}/fd').iterdir()));assert after<before,(before,after)
    records.append({'case':'actual-drain-deadline','descriptor_before':before,'descriptor_after':after,'elapsed_seconds':time.monotonic()-start,'passed':True})
  finally:
   child.terminate()
   try:child.wait(timeout=5)
   except subprocess.TimeoutExpired:child.kill();child.wait();raise
 result={'ready':ready,'capacity':capacity,'date_mode':date_mode,'records':records,'owned_child_reaped':True,'exit_code':child.returncode,'stderr_path':str(err),'passed':all(r['passed'] for r in records)}
 report.write_text(json.dumps(result,indent=2)+'\n');print(json.dumps({'passed':result['passed'],'records':len(records),'capacity':capacity,'date_mode':date_mode}));return 0
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('--binary',required=True);p.add_argument('--capacity',type=int,default=2);p.add_argument('--date-mode',type=int,default=1);p.add_argument('--report',required=True);a=p.parse_args();raise SystemExit(main(a.binary,a.capacity,a.date_mode,a.report))
