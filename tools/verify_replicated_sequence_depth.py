"""Independent tagged-tree/Unicode and marker-token prefix-context oracles."""
import argparse,copy,hashlib,json,random,struct,subprocess
from collections import Counter

def valid(cp):return 0<=cp<=0x10ffff and not 0xd800<=cp<=0xdfff
class Tree:
 def __init__(self,mode):self.mode=mode;self.items={};self.wait=set()
 def step(self,op,a,b,c):
  if op=='A':
   if not a or b>=a or not valid(c):return 1
   if a in self.items and self.items[a][2]:return 0 if self.items[a][:2]==(b,c) else 3
   if a not in self.items and len(self.items)==16:return 2
   deleted=self.items.get(a,(0,0,0,0))[3] or a in self.wait
   self.items[a]=(b,c,1,int(deleted));self.wait.discard(a);return 0
  if not a:return 1
  if a in self.items:
   x=self.items[a];self.items[a]=(*x[:3],1);return 0
  if self.mode:
   if len(self.items)==16:return 2
   self.items[a]=(0,0,0,1);return 0
  if a in self.wait:return 0
  if len(self.wait)==16:return 2
  self.wait.add(a);return 0
 def output(self,status):
  data=bytearray(336);data[:4]=b'RPS1';data[4:12]=(1).to_bytes(8,'big');data[12:15]=bytes([self.mode,len(self.items),len(self.wait)])
  for n,(ident,(parent,cp,inserted,removed)) in enumerate(sorted(self.items.items())):data[16+16*n:30+16*n]=struct.pack('>IIIBB',ident,parent,cp,inserted,removed)
  for n,ident in enumerate(sorted(self.wait)):data[272+4*n:276+4*n]=ident.to_bytes(4,'big')
  def children(parent):
   result=[]
   for ident,x in sorted(self.items.items(),reverse=True):
    if x[2] and x[0]==parent:
     if not x[3]:result.append(chr(x[1]))
     result.extend(children(ident))
   return result
  return f'{status} '+(data+hashlib.sha256(data).digest()).hex()+' '+''.join(children(0)).encode('utf8').hex()

def tree_cases():
 lines=[];expected=[];counts=Counter()
 for history in range(96):
  rng=random.Random(0x765006+history);mode=history%2;t=Tree(mode);lines.append(f'I {mode} 0 0');expected.append(t.output(0))
  for _ in range(96):
   a=rng.randrange(1,25);op='D' if rng.randrange(3)==0 else 'A';b=rng.randrange(a);c=rng.choice([0,0x24,0x7f,0x80,0x7ff,0x800,0x20ac,0xd800,0xdfff,0xe000,0x10000,0x10ffff,0x110000]);r=t.step(op,a,b,c);counts[r]+=1;lines.append(f'{op} {a} {b} {c}');expected.append(t.output(r))
 return lines,expected,counts

def apply(doc,op,token):
 kind,pos,cp=op;doc=doc[:]
 if kind==1:doc.insert(pos,(token,cp))
 elif kind==2:doc.pop(pos)
 return doc
class Model:
 def __init__(self):
  self.docs=[[]];self.vec=[[0,0,0]];self.original=[];self.commits=[];self.q=[None]*3;self.clients=[dict(rev=0,pending=0,doc=[],vec=[0,0,0]) for _ in range(3)]
 def prepare(self,site,kind,pos,cp):
  c=self.clients[site]
  if c['pending']:return 7
  if kind==1 and (not valid(cp) or pos>len(c['doc'])) or kind==2 and (cp or pos>=len(c['doc'])):return 1
  self.q[site]=dict(base=c['rev'],site=site,seq=c['vec'][site]+1,vec=c['vec'][:],op=(kind,pos,cp));c['pending']=1;return 0
 def transformed(self,q):
  kind,pos,cp=q['op'];base=q['base']
  if kind==2:
   target=self.docs[base][pos][0]
   where=[i for i,t in enumerate(self.docs[-1]) if t[0]==target]
   return (2,where[0],0) if where else (0,0,0)
  marker=object();tokens=self.docs[base][:];tokens.insert(pos,marker)
  for revision,commit in enumerate(self.commits[base:],base+1):
   k,p,value=commit['op']
   if not k:continue
   indices=[i for i,t in enumerate(tokens) if t is not marker]
   if k==2:tokens.pop(indices[p]);continue
   index=indices[p] if p<len(indices) else len(tokens)
   m=tokens.index(marker)
   if sum(t is not marker for t in tokens[:m])==p:index=m if commit['site']<q['site'] else m+1
   tokens.insert(index,(revision,value))
  return (1,tokens.index(marker),cp)
 def submit(self,site):
  q=self.q[site]
  for i,prior in enumerate(self.original):
   if (prior['site'],prior['seq'])==(site,q['seq']):return 0 if prior==q else 3
  if q['seq']!=self.vec[-1][site]+1 or self.vec[q['base']][site]!=self.vec[-1][site]:return 4
  if len(self.commits)==16:return 2
  op=self.transformed(q);rev=len(self.commits)+1;c=dict(rev=rev,site=site,seq=q['seq'],before=self.vec[-1][:],op=op);self.commits.append(c);self.original.append(copy.deepcopy(q));self.docs.append(apply(self.docs[-1],op,rev));v=self.vec[-1][:];v[site]+=1;self.vec.append(v);return 0
 def deliver(self,site,revision):
  c=self.clients[site];q=self.commits[revision-1]
  if revision<=c['rev']:return 0
  if revision!=c['rev']+1:return 4
  if q['site']==site and (not c['pending'] or q['seq']!=self.q[site]['seq']):return 3
  c['doc']=apply(c['doc'],q['op'],revision);c['vec']=q['before'][:];c['vec'][q['site']]+=1;c['rev']=revision
  if q['site']==site:c['pending']=0
  return 0
 def output(self,status):
  x=[status,len(self.commits),len(self.docs[-1])]+[t[1] for t in self.docs[-1]]
  for c in self.clients:x += [c['rev'],c['pending'],len(c['doc'])]+c['vec']+[t[1] for t in c['doc']]
  return ' '.join(map(str,x))

def ot_cases():
 lines=[];expected=[];counts=Counter();three=0
 for history in range(96):
  rng=random.Random(0x765106+history);m=Model();lines.append('I 0 0 0 0');expected.append(m.output(0))
  for step in range(160):
   site=rng.randrange(3);action=rng.randrange(10)
   if action<4 or not m.q[site]:
    n=len(m.clients[site]['doc']);kind=2 if n and rng.randrange(3)==0 else 1;pos=rng.randrange(n+(kind==1));cp=0 if kind==2 else rng.choice([0,0x61+site,0x20ac,0x1f600]);line=f'P {site} {kind} {pos} {cp}';status=m.prepare(site,kind,pos,cp)
   elif action<7 or not m.commits:line=f'S {site} 0 0 0';status=m.submit(site)
   else:
    revision=min(m.clients[site]['rev']+1,len(m.commits)) if rng.randrange(4) else rng.randrange(1,len(m.commits)+1);line=f'D {site} {revision} 0 0';status=m.deliver(site,revision)
   lines.append(line);expected.append(m.output(status));counts[status]+=1
  if len(m.commits)>=3:three+=1
 return lines,expected,counts,three

def execute(driver,mode,lines,expected):
 p=subprocess.run([driver,mode],input='\n'.join(lines)+'\n',text=True,capture_output=True);assert p.returncode==0,p.stderr
 observed=p.stdout.splitlines();assert len(observed)==len(expected),(len(observed),len(expected))
 for i,(a,b) in enumerate(zip(observed,expected)):
  assert a.rstrip()==b.rstrip(),(mode,i,lines[i],a,b)
 return hashlib.sha256(p.stdout.encode()).hexdigest()
if __name__=='__main__':
 ap=argparse.ArgumentParser();ap.add_argument('--driver',required=True);a=ap.parse_args();t,te,tc=tree_cases();o,oe,oc,three=ot_cases();th=execute(a.driver,'tree_oracle',t,te);oh=execute(a.driver,'ot_oracle',o,oe);print(json.dumps(dict(passed=True,tree_histories=96,tree_rows=len(t),tree_status_counts=dict(tc),tree_output_sha256=th,ot_histories=96,ot_rows=len(o),ot_status_counts=dict(oc),histories_with_three_operations=three,ot_output_sha256=oh,limits=['Original fixed-prefix sequencer model, not a general decentralized OT theorem.','Python uses tagged token targets and an explicit gap marker; no C transform function is imported.'])))
