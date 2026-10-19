"""Bounded policy counterexamples and changed-premise calculations."""
import json
def main():
 wire=b'POST /echo HTTP/1.1\r\nContent-Length: 4\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\nGET /health HTTP/1.1\r\n\r\n'
 start=wire.index(b'\r\n\r\n')+4
 # Two intentionally tiny original interpreters. These are local byte-string
 # models, not a proxy or remote exploit. They differ only in framing policy.
 def length_first(w):return w.index(b'\r\n\r\n')+4+4
 def chunk_first(w):return w.index(b'\r\n\r\n')+4+len(b'0\r\n\r\n')
 a,b=length_first(wire),chunk_first(wire);assert a==start+4 and b==start+5 and a!=b
 def repaired(w):
  h=w[:w.index(b'\r\n\r\n')].lower();return (400,'close',0) if b'content-length:' in h and b'transfer-encoding:' in h else (200,'continue',1)
 assert repaired(wire)==(400,'close',0)
 # A deliberately flawed continuation counts optimistic bytes after rejection.
 optimistic=b'CONNECT local:80 HTTP/1.1\r\n\r\nGET /health HTTP/1.1\r\n\r\n'
 flawed=sum(x.startswith((b'CONNECT',b'GET')) for x in optimistic.split(b'\r\n\r\n'))
 fixed=0;assert flawed==2 and fixed==0
 # size_t width is not the wire numeric domain. On a hypothetical16-bit size_t,
 # u32 still parses first; never convert a >1024 number to that smaller type.
 numbers=['1025','4294967295','4294967296','1025x'];decision=[]
 for s in numbers:
  decision.append(400 if not s.isdecimal() or int(s)>2**32-1 else 413 if int(s)>1024 else 0)
 assert decision==[413,413,400,400]
 # Cost changes do not authorize changing response association: completion2
 # first waits for1's sequence. This model retains both slots, then commitsFIFO.
 completed={2:b'fast',1:b'slow'};wire_order=[completed[i] for i in (1,2)];assert wire_order==[b'slow',b'fast']
 result={'disagreement':{'wire_hex':wire.hex(),'length_boundary':a,'chunk_boundary':b,'repaired':repaired(wire)},'optimistic':{'flawed_effects':flawed,'repaired_effects':fixed,'partitions':len(optimistic)+1},'size_t16_premise':{'actual_host_execution':False,'u32_then_cap':decision,'conversion_only_when_at_most':1024},'completion_order':[2,1],'wire_order':[1,2],'clock_rollback':{'wall':[1672531200,1672531190],'monotonic_ms':[1000,2000],'elapsed':1000},'delayed_learner':'not-observed; minimum7days and unaided reviewer remain required'}
 print(json.dumps(result,sort_keys=True));print('HTTP_MODELS_PASS:framing-disagreement,optimistic-reject,u32-before-size_t,cost-order,wall-not-monotonic');return 0
if __name__=='__main__':raise SystemExit(main())
