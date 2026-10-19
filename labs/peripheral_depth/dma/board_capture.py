"""Declared capture premises and ownership ordering, never authenticity proof."""
import json,re,sys

def validate(d):
 e=[]
 def need(x,s):
  if not x:e.append(s)
 need(d.get('schema_version')==1,'schema');need(d.get('provenance') in ('physical','synthetic-test'),'provenance');need(d.get('board') in ('F439','H745'),'board');need(bool(d.get('silicon_revision')),'revision');need(bool(re.fullmatch('[a-f0-9]{64}',str(d.get('image_sha256')))),'image');need(type(d.get('counter_hz')) is int and d['counter_hz']>0,'timebase');need(d.get('cache_policy') in ('none','write-back','noncacheable'),'cache policy')
 need(d.get('board')!='F439' or d.get('cache_policy')=='none','F439 has no selected D-cache');need(d.get('board')!='H745' or d.get('cache_policy')!='none','H745 attributes missing')
 artifacts=d.get('raw_artifacts',[]);need(bool(artifacts),'raw evidence')
 for r in artifacts:need(bool(r.get('path')) and bool(re.fullmatch('[a-f0-9]{64}',str(r.get('sha256')))),'raw identity')
 rows=d.get('observations',[]);need(bool(rows),'observations');last=-1;state={}
 for r in rows:
  tick=r.get('tick');token=r.get('token');kind=r.get('kind');valid=type(tick)is int and tick>=last and type(token)is int and 0<token<2**32
  need(valid,'ordered time/token')
  if not valid:continue
  last=tick;s=state.get(token,'absent')
  if kind=='submit':need(s=='absent','reused generation');state[token]='active'
  elif kind=='cancel':need(s=='active','cancel outside lease')
  elif kind=='quiescent':need(s=='active' and r.get('enabled')==0 and r.get('fifo_empty')==1 and (r.get('flags',0)&32)!=0,'quiescence premises');state[token]='ready'
  elif kind=='finalize':
   total=r.get('total');complete=r.get('complete');possible=r.get('possible');need(s=='ready' and all(type(x)is int for x in [total,complete,possible]) and 0<=complete<=possible<=total<=16384,'result interval');need(r.get('guards_unchanged') is True,'guard bytes');state[token]='owned'
  elif kind=='publish':need(s=='owned' and d.get('board')=='H745','publish before finalize');state[token]='lent'
  elif kind=='take':need(s=='lent','take ordering');state[token]='reading'
  elif kind=='ack':need(s=='reading','ack ordering');state[token]='owned'
  elif kind=='reuse':need(s=='owned','premature reuse');state[token]='released'
  else:need(False,'event')
 need(all(v in ('released','owned') for v in state.values()),'unclosed lease capture');return dict(passed=not e,errors=e,scope='Format and declared single-extent event ordering; raw authenticity, silicon semantics and multi-extent traces require review.')
if __name__=='__main__':
 r=validate(json.load(open(sys.argv[1],encoding='utf8')));print(json.dumps(r));raise SystemExit(not r['passed'])
