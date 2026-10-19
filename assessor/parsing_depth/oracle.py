"""Independent language/set, Earley, operator-stack and immutable PEG models."""
import argparse,functools,hashlib,itertools,json,subprocess

def words(alphabet,bound):
 return [bytes(v) for n in range(bound+1) for v in itertools.product(alphabet,repeat=n)]
def regex_text(a):
 if a[0]=='lit':return chr(a[1])
 if a[0]=='empty':return '~'
 if a[0]=='star':return '('+regex_text(a[1])+')*'
 return '('+regex_text(a[1])+('|' if a[0]=='alt' else '')+regex_text(a[2])+')'
def regex_accept(a,s):
 @functools.lru_cache(None)
 def ends(tree,start):
  kind=tree[0]
  if kind=='empty':return frozenset([start])
  if kind=='lit':return frozenset([start+1]) if start<len(s) and s[start]==tree[1] else frozenset()
  if kind=='alt':return ends(tree[1],start)|ends(tree[2],start)
  if kind=='cat':return frozenset(j for i in ends(tree[1],start) for j in ends(tree[2],i))
  seen={start};todo=[start]
  while todo:
   for j in ends(tree[1],todo.pop()):
    if j not in seen:seen.add(j);todo.append(j)
  return frozenset(seen)
 return int(len(s) in ends(a,0)),int(any(ends(a,i) for i in range(len(s)+1)))

GRAMMARS=[[(12,(8,)),(8,(8,1,9)),(8,(9,)),(9,(9,2,10)),(9,(10,)),(10,(3,8,4)),(10,(0,))],[(12,(8,)),(8,(8,1,8)),(8,(0,))],[(12,(8,)),(8,(9,1)),(9,(0,9)),(9,())]]
def earley(grammar,tokens):
 chart=[set() for _ in range(len(tokens)+1)];chart[0].add((0,0,0))
 for i in range(len(chart)):
  changed=True
  while changed:
   old=len(chart[i])
   for p,dot,start in list(chart[i]):
    lhs,rhs=grammar[p]
    if dot==len(rhs):
     for q,k,j in list(chart[start]):
      if k<len(grammar[q][1]) and grammar[q][1][k]==lhs:chart[i].add((q,k+1,j))
    elif rhs[dot]>=8:
     for q,(head,_) in enumerate(grammar):
      if head==rhs[dot]:chart[i].add((q,0,i))
    elif i<len(tokens) and rhs[dot]==tokens[i]:chart[i+1].add((p,dot+1,start))
   changed=len(chart[i])!=old
 return (0,1,0) in chart[-1]
def verify_lr(row,mode,tokens):
 values=list(map(int,row.split()[1:]));build,parse=values[:2]
 if mode==1:assert (build,parse)==(5,9);return
 wanted=0 if earley(GRAMMARS[mode],tokens) else 1
 assert (build,parse)==(0,wanted),(mode,tokens,build,parse,wanted)
 if parse:return
 count,root=values[2:4];cursor=4;nodes=[];yielded=[]
 for i in range(count):
  symbol,prod,n,begin,end=values[cursor:cursor+5];cursor+=5;children=values[cursor:cursor+n];cursor+=n
  assert 0<=begin<=end<=len(tokens)
  if prod==255:assert n==0 and end==begin+1 and tokens[begin]==symbol;result=[symbol]
  else:
   lhs,rhs=GRAMMARS[mode][prod];assert symbol==lhs and n==len(rhs) and all(0<=c<i for c in children)
   assert tuple(nodes[c][0] for c in children)==rhs
   result=[x for c in children for x in yielded[c]]
   if children:assert begin==nodes[children[0]][1] and end==nodes[children[-1]][2]
   else:assert begin==end
  nodes.append((symbol,begin,end,children));yielded.append(result)
 assert cursor==len(values) and nodes[root][0]==8 and yielded[root]==list(tokens)
 seen=set();todo=[root]
 while todo:
  i=todo.pop();assert i not in seen;seen.add(i);todo+=nodes[i][3]
 assert len(seen)==count

def pratt_model(text,right):
 # Independent shunting-yard construction, followed by integer evaluation.
 tokens=[];p=0
 while p<len(text):
  c=text[p]
  if c in ' \t\n':p+=1;continue
  start=p
  if c.isascii() and c.isdigit():
   while p<len(text) and text[p].isascii() and text[p].isdigit():p+=1
   v=int(text[start:p])
   if v>2147483647:return [3,9,9,123]
   tokens.append(('n',v,start))
  elif c in '+*()-^':tokens.append((c,0,p));p+=1
  else:return [1,9,9,123]
 if len(tokens)>64:return [3,9,9,123]
 precedence={'+':10,'-':10,'*':20,'u':30,'^':40};ops=[];stack=[];nodes=[];need=True
 def emit(op):
  kind,_,offset=op
  if kind=='u':
   if not stack:raise ValueError()
   node=(5,stack.pop(),0,0,offset)
  else:
   if len(stack)<2:raise ValueError()
   b,a=stack.pop(),stack.pop();node=({'+':1,'-':8,'*':2,'^':6}[kind],a,b,0,offset)
  stack.append(len(nodes));nodes.append(node)
 try:
  for token in tokens:
   kind,value,offset=token
   if kind=='n':
    if not need:raise ValueError()
    stack.append(len(nodes));nodes.append((0,0,0,value,offset));need=False
   elif kind=='(':
    if not need:raise ValueError()
    ops.append(token)
   elif kind==')':
    if need:raise ValueError()
    while ops and ops[-1][0]!='(':emit(ops.pop())
    if not ops:raise ValueError()
    ops.pop();need=False
   elif need:
    if kind!='-':raise ValueError()
    ops.append(('u',0,offset))
   else:
    while ops and ops[-1][0]!='(' and (precedence[ops[-1][0]]>precedence[kind] or precedence[ops[-1][0]]==precedence[kind] and not(kind=='^' and right)):emit(ops.pop())
    ops.append(token);need=True
  if need:raise ValueError()
  while ops:
   op=ops.pop()
   if op[0]=='(':raise ValueError()
   emit(op)
  if len(stack)!=1:raise ValueError()
 except ValueError:return [0,1,9,123]
 if len(nodes)>64:return [0,3,9,123]
 values=[];rc=0;out=123
 for kind,a,b,value,_ in nodes:
  if kind==0:values.append(value);continue
  x=values[a];y=0 if kind==5 else values[b]
  if kind==6 and (y<0 or y>31):rc=8;break
  v=x+y if kind==1 else x-y if kind==8 else x*y if kind==2 else -x if kind==5 else x**y
  if not -(1<<31)<=v<(1<<31):rc=3;break
  values.append(v)
 if not rc:out=values[-1]
 return [0,0,rc,out,len(nodes),*[x for node in nodes for x in node]]

def lit(c):return (0,ord(c))
def rng(a,b):return (1,ord(a),ord(b))
ANY=(2,);EMPTY=(3,)
def seq(*nodes):
 if len(nodes)==1:return nodes[0]
 return (4,nodes[0],seq(*nodes[1:]))
def choice(a,b):return (5,a,b)
def star(a):return (6,a)
def neg(a):return (7,a)
def cap(a,tag):return (8,a,tag)
def word(s):return seq(*(lit(c) for c in s))
def plus(a):return seq(a,star(a))
def encode(tree):
 ordered=[];seen=set()
 def visit(t):
  if t in seen:return
  if t[0]>=4:
   visit(t[1])
   if t[0] in [4,5]:visit(t[2])
  seen.add(t);ordered.append(t)
 visit(tree);ordered.reverse();indices={t:i for i,t in enumerate(ordered)};out=[]
 for t in ordered:
  k=t[0];out.append([k,indices[t[1]] if k>=4 else 0,indices[t[2]] if k in [4,5] else 0,t[1] if k in [0,1] else t[2] if k==8 else 0,t[2] if k==1 else 0])
 return out
def peg_model(tree,s,eof,budget):
 fuel=[budget]
 def run(t,p):
  if not fuel[0]:return 6,p,[]
  fuel[0]-=1;k=t[0]
  if k<=2:
   if p==len(s):return (1 if eof else 2),p,[]
   yes=k==2 or k==0 and s[p]==t[1] or k==1 and t[1]<=s[p]<=t[2]
   return (0,p+1,[]) if yes else (1,p,[])
  if k==3:return 0,p,[]
  if k==6:
   end=p;captures=[]
   while True:
    rc,new,extra=run(t[1],end)
    if rc==1:return 0,end,captures
    if rc:return rc,new,[]
    if new==end:return 7,new,[]
    captures+=extra;end=new
  rc,end,captures=run(t[1],p)
  if k==7:return (1 if rc==0 else 0 if rc==1 else rc),p,[]
  if k==8:
   if rc:return rc,end,[]
   if len(captures)==32:return 3,p,[]
   return 0,end,captures+[(t[2],p,end)]
  if k==5:return run(t[2],p) if rc==1 else (rc,end,captures)
  if rc:return rc,end,[]
  other,last,extra=run(t[2],end)
  return (other,last,[]) if other else (0,last,captures+extra)
 rc,end,captures=run(tree,0)
 if rc:return [rc,999,0]
 return [0,end,len(captures),*[x for c in captures for x in c]]

def cases():
 rows=[];checks=[];counts={}
 def add(kind,line,expected):rows.append(line);checks.append((kind,expected));counts[kind]=counts.get(kind,0)+1
 atoms=[('lit',97),('lit',98),('empty',)];expressions=atoms+[('star',a) for a in atoms]+[(k,a,b) for k in ['alt','cat'] for a in atoms for b in atoms]
 expressions += [('star',a) for a in expressions]+[('cat',('star',('alt',atoms[0],atoms[1])),('cat',atoms[0],atoms[1]))]
 for ast in expressions:
  pat=regex_text(ast).encode()
  for text in words([97,98,99],4):
   whole,search=regex_accept(ast,text);add('R',' '.join(map(str,['R',len(pat),*pat,len(text),*text,16])),[0,whole,search,0,whole])
 for mode in [0,2]:
  alphabet=[0,1,2,3,4] if mode==0 else [0,1,2]
  for tokens in words(alphabet,5):add('L',' '.join(map(str,['L',mode,len(tokens),*tokens,1000])),(mode,tokens))
 add('L','L 1 3 0 1 0 1000',(1,bytes([0,1,0])))
 expressions=['','1+','(1','(1+)','1 2','2147483648','2147483647+1','2^-1','2^31','-2^2','2^3^2','0^0','-2147483647-1','--1','1-2-3',' \n12+3']
 bases=['0','1','2','3','-1','(1+2)']
 expressions += [a+op+b for a,b,op in itertools.product(bases,bases,['+','-','*','^'])]
 for a,b,c in itertools.product(['0','1','2'],repeat=3):
  for op,op2 in itertools.product(['+','-','*','^'],repeat=2):expressions.append(a+op+b+op2+c)
 for s in expressions:
  for right in [0,1]:add('P',' '.join(map(str,['P',right,len(s.encode()),*s.encode()])),pratt_model(s,right))
 letter=rng('a','z');digit=rng('0','9');space=lit(' ');boundary=neg(letter)
 identifier=seq(neg(seq(word('let'),boundary)),plus(letter))
 declaration=seq(word('let'),boundary,plus(space),cap(identifier,1))
 grammar=seq(choice(declaration,cap(identifier,1)),star(space),lit('='),star(space),cap(plus(digit),2),neg(ANY))
 rollback=seq(choice(seq(cap(lit('a'),1),lit('x')),seq(lit('a'),lit('b'))),neg(ANY))
 ordering=seq(choice(lit('a'),word('ab')),neg(ANY))
 predicates=seq(neg(cap(lit('b'),7)),cap(lit('a'),9),neg(ANY))
 samples=[b'let x=12',b'letx=2',b'let=2',b'x = 001',b'let xy = 2',b'let x=',b'abc=9x',b'ab',b'ax',b'a',b'b',b'']
 for tree in [grammar,rollback,ordering,predicates,star(EMPTY)]:
  nodes=encode(tree);assert len(nodes)<=64
  texts=set(words([97,98,120],3))|set(samples)
  texts|={s[:i] for s in samples for i in range(len(s)+1)}
  for text in sorted(texts):
   for eof in [0,1]:
    line=['S',len(nodes),*[x for n in nodes for x in n],len(text),*text,eof,1000]
    add('S',' '.join(map(str,line)),peg_model(tree,text,eof,1000))
 return rows,checks,counts
def main():
 p=argparse.ArgumentParser();p.add_argument('--driver');args=p.parse_args();rows,checks,counts=cases();source='\n'.join(rows)+'\n'
 if args.driver:
  proc=subprocess.run([args.driver],input=source,capture_output=True,text=True,timeout=120);assert proc.returncode==0,proc.stderr
  actual=proc.stdout.splitlines();assert len(actual)==len(checks),(len(actual),len(checks))
  for i,(line,(kind,expected)) in enumerate(zip(actual,checks)):
   try:
    assert line[0]==kind
    if kind=='L':verify_lr(line,*expected)
    else:assert list(map(int,line.split()[1:]))==expected
   except (AssertionError,IndexError) as e:raise AssertionError((i,rows[i],line,expected)) from e
 print(json.dumps(dict(passed=True,observations=len(rows),cases=counts,input_sha256=hashlib.sha256(source.encode()).hexdigest(),scope='Finite independent language sets, Earley recognition plus derivation certificates, operator-stack syntax trees, immutable PEG capture semantics.')))
if __name__=='__main__':main()
