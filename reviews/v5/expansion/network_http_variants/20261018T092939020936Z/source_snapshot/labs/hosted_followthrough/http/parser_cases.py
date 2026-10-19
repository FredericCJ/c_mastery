"""Original byte cases with independently specified outcomes, not C-derived oracles."""
import json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[3]
A='localhost:8080'
def request(method='GET',target='/health',fields=b'',body=b'',authority=A):
 return f'{method} {target} HTTP/1.1\r\nHost: {authority}\r\n'.encode()+fields+b'\r\n'+body
def cases():
 rows=[]
 def add(name,data,status=200,body=b'',result=1,used=None,eof=0,authority=A,all_splits=True):
  rows.append(dict(name=name,wire=data.hex(),status=status,body=body.hex() if body is not None else None,result=result,used=used,eof=eof,authority=authority,all_splits=all_splits))
 get=request();assert len(get)==46
 add('headers46',request(authority='127.0.0.1:8080'),authority='127.0.0.1:8080')
 add('coalesced',get+get,used=len(get))
 binary=bytes([0,0x80,0xff,13,10]);add('binary',request('POST','/echo',b'Content-Length: 5\r\n',binary),body=binary)
 for host in ('localhost:8080','api.example:91','192.0.2.9:8081'):
  add('origin-'+host.replace(':','-'),request(authority=host),authority=host)
  add('absolute-'+host.replace(':','-'),request(target='http://'+host+'/health',authority='other.example'),authority=host)
 for host in ('bad@host','[::1]:80','host:','host:x','', 'other.example'):
  add('host-reject-'+str(len(rows)),request(authority=host),400,result=-1)
 for name,field in [('space-colon',b'Host : localhost:8080'),('fold',b' folded'),('nul',b'X: a\0b'),('control',b'X: a\x01b'),('duplicate-host',b'Host: localhost:8080')]:
  add(name,request(fields=field+b'\r\n'),400,result=-1)
 for numeral,status in [('1025',413),('4294967295',413),('4294967296',400),('1025x',400),('-1',400),('+1',400),('',400)]:
  add('length-'+(numeral or 'empty'),request('POST','/echo',b'Content-Length: '+numeral.encode()+b'\r\n'),status,result=-1)
 for name,fields,status in [('duplicate',b'Content-Length: 0\r\nContent-Length: 0\r\n',400),('te-cl',b'Transfer-Encoding: chunked\r\nContent-Length: 0\r\n',400),('te-overcap',b'Content-Length: 1025\r\nTransfer-Encoding: chunked\r\n',400),('gzip',b'Transfer-Encoding: gzip\r\n',400),('chunked-gzip',b'Transfer-Encoding: chunked,gzip\r\n',400),('gzip-chunked',b'Transfer-Encoding: gzip,chunked\r\n',501),('repeated',b'Transfer-Encoding: chunked,chunked\r\n',400)]:
  add('framing-'+name,request('POST','/echo',fields)+get,status,result=-1)
 for method,target,status in [('GET','/health',200),('GET','/echo',405),('POST','/unknown',404)]:
  wire=request(method,target,b'Content-Length: 5\r\n',b'abcde');add('method-'+method+target.replace('/','-'),wire+get,status,b'abcde',used=len(wire))
  for n in range(5):add('eof-'+method+target.replace('/','-')+str(n),request(method,target,b'Content-Length: 5\r\n',b'abcde'[:n]),400,None,-1,eof=1)
 add('none-post',request('POST','/echo')+get,body=b'',used=len(request('POST','/echo')))
 chunk_header=request('POST','/echo',b'Transfer-Encoding: chunked\r\n')
 chunks=b'3;x=y\r\nabc\r\n2\r\nde\r\n0\r\nX-Debug: ignored\r\n\r\n'
 add('chunk-ledger',chunk_header+chunks,body=b'abcde')
 add('trailer-isolation',chunk_header+b'1;x="a\\\"b\\\\c"\r\nx\r\n0\r\nHost: other.example\r\nContent-Length: 999\r\n\r\n',body=b'x')
 for extension_length in (128,129):
  extension=b';x="'+b'a'*(extension_length-9)+b'\\\"\\\\"';assert len(extension)==extension_length
  add('extension-'+str(extension_length),chunk_header+b'1'+extension+b'\r\nx\r\n0\r\n\r\n',200 if extension_length==128 else 400,b'x' if extension_length==128 else None,1 if extension_length==128 else -1)
 for n in (1024,1025):add('chunk-body-'+str(n),chunk_header+f'{n:x}\r\n'.encode()+b'x'*n+b'\r\n0\r\n\r\n',200 if n==1024 else 413,b'x'*n if n==1024 else None,1 if n==1024 else -1,all_splits=False)
 for n in (1024,1025):
  trailers=b'X:'+b'a'*(n-6)+b'\r\n\r\n';assert len(trailers)==n
  add('trailer-bytes-'+str(n),chunk_header+b'0\r\n'+trailers,200 if n==1024 else 400,b'',1 if n==1024 else -1,all_splits=False)
 for n in (8,9):add('trailer-fields-'+str(n),chunk_header+b'0\r\n'+b'X:y\r\n'*n+b'\r\n',200 if n==8 else 400,b'',1 if n==8 else -1)
 # Change only framing delimiters. Decoded data stays abcde in the input.
 for index,octet in enumerate(chunks):
  if octet in (13,10):
   bad=chunks[:index]+b'X'+chunks[index+1:];add('chunk-framing-mutation-'+str(index),chunk_header+bad,400,None,-1,eof=1)
 for n in range(1,len(chunks)):
  add('chunk-eof-'+str(n),chunk_header+chunks[:n],400,None,-1,eof=1,all_splits=False)
 for method,target,status in [('GET','/health',200),('HEAD','/health',200),('HEAD','/unknown',404),('HEAD','/echo',405)]:add('response-'+method+target.replace('/','-'),request(method,target),status)
 add('expect-valid',request('POST','/echo',b'Content-Length: 1\r\nExpect: 100-continue\r\n',b'x'),body=b'x')
 add('expect-invalid',request('POST','/echo',b'Content-Length: 1\r\nContent-Length: 1\r\nExpect: 100-continue\r\n'),400,result=-1)
 for name,wire in [('connect',b'CONNECT localhost:8080 HTTP/1.1\r\nHost: localhost:8080\r\n\r\n'),('upgrade',request(fields=b'Upgrade: local-demo\r\n'))]:add(name+'-optimistic',wire+get,501,None,-1)
 add('date-omitted',get,eof=2)
 # Independent memory limits: line storage includes NUL, wire header cap does not.
 for n in (8192,8193):
  base=request(fields=b'X:\r\n');wire=request(fields=b'X:'+b'a'*(n-len(base))+b'\r\n');assert len(wire)==n
  add('header-bytes-'+str(n),wire,200 if n==8192 else 431,None,1 if n==8192 else -1,all_splits=False)
 for n in (32,33):add('header-fields-'+str(n),request(fields=b'X:y\r\n'*(n-1)),200 if n==32 else 431,None,1 if n==32 else -1,all_splits=False)
 for n in (1024,1025):add('target-'+str(n),request(target='/'+'a'*(n-1)),404 if n==1024 else 414,None,1 if n==1024 else -1,all_splits=False)
 semantics=json.loads((ROOT/'fixtures/network_http/05_semantics.json').read_text())
 for i,(source,expected) in enumerate(zip(semantics['input']['cases'],semantics['expected']['responses'])):
  body=bytes.fromhex(source['body_hex']);fields=f'Content-Length: {len(body)}\r\n'.encode() if body else b''
  add('fixture05-'+str(i),request(source['method'],source['target'],fields,body),expected['status'],body)
  rows[-1]['expected_representation']=expected
 fixture=json.loads((ROOT/'fixtures/network_http/09_wire.json').read_text())
 for expected in fixture['expected']['messages']:
  method=expected['method'];body=bytes.fromhex(fixture['input']['echo_body_hex']) if method=='POST' else b''
  fields=b'Connection: close\r\n'+(f'Content-Length: {len(body)}\r\n'.encode() if body else b'')
  add('fixture09-'+method,request(method,'/echo' if method=='POST' else '/health',fields,body),body=body)
  rows[-1]['literal_response']=(expected['header'].encode()+bytes.fromhex(expected['body_hex'])).hex()
 return rows
def batch():
 rows=[];lines=[]
 for case in cases():
  data=bytes.fromhex(case['wire']);splits=range(len(data)+1) if case['all_splits'] else sorted({0,1,len(data)//2,len(data)-1,len(data)})
  partitions=[(split,len(data)) for split in splits]
  if case['name']=='chunk-ledger':
   start=data.index(b'\r\n\r\n')+4;partitions += [(i,i+1) for i in range(start,len(data)-1) if data[i:i+2]==b'\r\n']
  for split,second in partitions:
   row=dict(case,split=split,second=second,id=str(len(rows)));rows.append(row);lines.append(f"{row['id']} {split} {second} {case['eof']} {case['authority']} {case['wire']}")
 return rows,'\n'.join(lines)+'\n'
def verify(expected,actual):
 assert len(expected)==len(actual)
 groups={}
 for e,a in zip(expected,actual):
  name=e['name'];assert a['id']==e['id'] and a['result']==e['result'] and a['status']==e['status'],(name,e['split'],a)
  if e['body'] is not None:assert a['body_hex']==e['body'],(name,'body',a)
  if e['used'] is not None:assert a['used']==e['used'],(name,'used',a)
  if e['result']==1:
   assert a['used']==(e['used'] or len(bytes.fromhex(e['wire'])))
   assert a['authority'].lower()==e['authority'].lower()
  wire=bytes.fromhex(a['response_hex']);assert a['serializer']==0 and wire.startswith(f"HTTP/1.1 {e['status']} ".encode())
  header,body=wire.split(b'\r\n\r\n',1);fields=dict(line.split(b': ',1) for line in header.split(b'\r\n')[1:]);length=int(fields[b'Content-Length'])
  ishead=bytes.fromhex(e['wire']).startswith(b'HEAD ')
  assert len(body)==(0 if ishead else length),(name,length,body)
  if name=='response-HEAD-unknown':assert length==10
  if name=='response-HEAD-echo':assert length==19 and fields[b'Allow']==b'POST'
  if name=='date-omitted':assert b'Date' not in fields
  else:assert fields[b'Date']==b'Sun, 01 Jan 2023 00:00:00 GMT'
  if name=='headers46':assert len(bytes.fromhex(e['wire']))==46 and (e['split']==46 or a['prefix_result']==0)
  if 'literal_response' in e:assert wire.hex()==e['literal_response'],(name,'literal original wire')
  if 'expected_representation' in e:
   original=e['expected_representation'];assert body.hex()==original['body_hex'] and length==original['length'],(name,'original representation')
   if 'allow' in original:assert fields[b'Allow'].decode()==original['allow']
  groups[name]=groups.get(name,0)+1
 return groups
if __name__=='__main__':print(json.dumps({'cases':cases()},indent=2))
