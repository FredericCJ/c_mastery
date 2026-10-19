"""Independent tree semantics, byte decoder/control analysis and dispatch model.

This host oracle intentionally uses a different recursive tree interpreter;
it is not target code or evidence that the C compiler may recurse.
"""
import json,pathlib,re,subprocess,sys
class Refusal(Exception):
 def __init__(self,code):self.code=code
class BreakValue(Exception):
 def __init__(self,value):self.value=value
def checked(n):
 if not -(1<<31)<=n<(1<<31):raise Refusal(4)
 return n
def boolean(v):
 if type(v)!=bool:raise Refusal(3)
 return v
def operation(op,a,b):
 if op=='==':return type(a)==type(b) and a==b
 if type(a)!=int or type(b)!=int:raise Refusal(3)
 if op=='<':return a<b
 if op=='+':return checked(a+b)
 if op=='-':return checked(a-b)
 if op=='*':return checked(a*b)
 if not b or (a==-(1<<31) and b==-1):raise Refusal(4)
 return (abs(a)//abs(b))*(-1 if (a<0)!=(b<0) else 1)
def parse(source):
 tokens=re.findall(r'\(|\)|[^\s()]+',source);index=0
 def one():
  nonlocal index
  token=tokens[index];index+=1
  if token!='(':return int(token) if re.fullmatch(r'-?\d+',token) else token
  out=[]
  while tokens[index]!=')':out.append(one())
  index+=1;return out
 out=[]
 while index<len(tokens):out.append(one())
 return out
def source_value(source):
 defs={d[1]:(d[2],d[3]) for d in parse(source)};steps=10000
 def evaluate(x,scopes):
  nonlocal steps
  steps-=1
  if steps<0:raise Refusal(7)
  if type(x)==int:return x
  if isinstance(x,str):
   if x in ['true','false','nil']:return {'true':True,'false':False,'nil':None}[x]
   for scope in reversed(scopes):
    if x in scope:return scope[x]
   raise AssertionError('unbound oracle input '+x)
  op,*args=x
  if op=='let':
   name,init,body=args;v=evaluate(init,scopes);return evaluate(body,scopes+[{name:v}])
  if op=='set':
   name,expr=args;v=evaluate(expr,scopes)
   for scope in reversed(scopes):
    if name in scope:scope[name]=v;return v
   raise AssertionError('unbound assignment')
  if op in ['do','block']:
   for arg in args:v=evaluate(arg,scopes)
   return v
  if op=='if':return evaluate(args[1] if boolean(evaluate(args[0],scopes)) else args[2],scopes)
  if op=='while':
   while boolean(evaluate(args[0],scopes)):
    try:evaluate(args[1],scopes)
    except BreakValue as b:return b.value
   return None
  if op=='break':raise BreakValue(evaluate(args[0],scopes))
  if op=='and':return boolean(evaluate(args[1],scopes)) if boolean(evaluate(args[0],scopes)) else False
  if op=='or':return True if boolean(evaluate(args[0],scopes)) else boolean(evaluate(args[1],scopes))
  if op=='call':
   params,body=defs[args[0]];values=[evaluate(a,scopes) for a in args[1:]];return evaluate(body,[dict(zip(params,values))])
  if op=='pair':return (evaluate(args[0],scopes),evaluate(args[1],scopes))
  if op in ['first','second']:return evaluate(args[0],scopes)[op=='second']
  if op=='gc':return None
  a=evaluate(args[0],scopes);b=evaluate(args[1],scopes);return operation(op,a,b)
 return evaluate(defs['main'][1],[{}])
def decode(code,functions):
 widths={1:5,11:2,12:2,14:3,15:3,16:3,22:2};all_instructions={};maxheight=0
 for entry,length,arity,locals_ in functions:
  pc=entry;instructions={}
  while pc<entry+length:
   op=code[pc];assert 1<=op<=22;size=widths.get(op,1);assert pc+size<=entry+length;instructions[pc]=(op,size)
   if op in [11,12]:assert code[pc+1]<locals_
   if op==16:assert code[pc+1]<len(functions) and code[pc+2]==functions[code[pc+1]][2]
   pc+=size
  assert pc==entry+length
  for pc,(op,size) in instructions.items():
   if op in [14,15]:assert int.from_bytes(code[pc+1:pc+3],'little') in instructions
  pending=[entry];states={entry:(0,(1<<arity)-1)}
  while pending:
   pc=pending.pop();height,initialized=states[pc];op,size=instructions[pc]
   need=2 if op in [5,6,7,8,9,10,18] else 1 if op in [12,13,15,17,19,20,22] else code[pc+2] if op==16 else 0
   assert height>=need
   delta=1 if op in [1,2,3,4,11] else -1 if op in [5,6,7,8,9,10,12,13,15,17,18] else 1-code[pc+2] if op==16 else 0
   if op==11:assert initialized&(1<<code[pc+1])
   if op==12:initialized|=1<<code[pc+1]
   if op==17:assert height==1;continue
   height+=delta;assert 0<=height<=64;maxheight=max(maxheight,height)
   destinations=[int.from_bytes(code[pc+1:pc+3],'little')] if op==14 else [pc+size,int.from_bytes(code[pc+1:pc+3],'little')] if op==15 else [pc+size]
   for dest in destinations:
    assert dest in instructions
    if dest in states:
     prior,init=states[dest];assert prior==height;merged=initialized&init
     if merged!=init:states[dest]=(height,merged);pending.append(dest)
    else:states[dest]=(height,initialized);pending.append(dest)
  all_instructions.update(instructions)
 return maxheight,len(all_instructions)
def machine(code,functions,fuel):
 stack=[];frames=[[functions[0][0],0,{}]];heap=[];remaining=fuel;steps=0
 try:
  while frames:
   if not remaining:raise Refusal(7)
   remaining-=1;steps+=1;frame=frames[-1];pc=frame[0];op=code[pc];frame[0]+=1
   if op==1:stack.append(int.from_bytes(code[pc+1:pc+5],'little',signed=True));frame[0]+=4
   elif op in [2,3,4]:stack.append({2:True,3:False,4:None}[op])
   elif op in [5,6,7,8,9,10]:b=stack.pop();a=stack.pop();stack.append(operation({5:'+',6:'-',7:'*',8:'/',9:'==',10:'<'}[op],a,b))
   elif op==11:stack.append(frame[2][code[pc+1]]);frame[0]+=1
   elif op==12:frame[2][code[pc+1]]=stack.pop();frame[0]+=1
   elif op==13:stack.pop()
   elif op==14:frame[0]=int.from_bytes(code[pc+1:pc+3],'little')
   elif op==15:
    value=boolean(stack.pop());frame[0]=pc+3 if value else int.from_bytes(code[pc+1:pc+3],'little')
   elif op==16:
    if len(frames)==16:raise Refusal(6)
    fn,argc=code[pc+1:pc+3];frame[0]+=2;base=len(stack)-argc;values=stack[base:];del stack[base:];frames.append([functions[fn][0],base,dict(enumerate(values))])
   elif op==17:
    result=stack.pop();del stack[frame[1]:];frames.pop()
    if frames:stack.append(result)
    else:return 0,result,remaining,steps
   elif op==18:b=stack.pop();a=stack.pop();pair=(a,b);heap.append(pair);stack.append(pair)
   elif op in [19,20]:stack.append(stack.pop()[op==20])
   elif op==21:pass
   else:raise AssertionError('unsupported oracle execution '+str(op))
 except Refusal as e:return e.code,-777,remaining,steps
def tag(v):return 0 if v is None else 2 if type(v)==bool else 1 if type(v)==int else 3
def integer(v):return 0 if v is None else int(v) if type(v) in [int,bool] else 0
def cases():
 out=[]
 for a,b in [(-3,2),(0,0),(5,-2),(2147483647,1),(-2147483648,-1)]:
  for op in ['+','-','*','/','<','==']:out.append('(fn main () ('+op+' '+str(a)+' '+str(b)+'))')
 for n in [0,1,2,5,9]:out.append(f'(fn main () (let x 0 (do (while (< x {n}) (set x (+ x 1))) x)))')
 for a,b in [(2,40),(-7,8),(0,-1)]:
  out.extend([f'(fn main () (let x {a} (+ (let x {b} x) x)))',f'(fn main () (let x {a} (+ (let x {b} (set x (+ x 1))) x)))'])
 out.extend(['(fn main () (and false (/ 1 0)))','(fn main () (or true (/ 1 0)))','(fn main () (and true 1))','(fn main () (or false 1))','(fn main () (if 1 2 3))','(fn main () (if false (if true 1 2) 3))','(fn main () (+ 100 (while true (+ 10 (let x 4 (break x))))))','(fn main () (while true (while (break 9) nil)))','(fn main () (first (while true (let x 7 (break (pair x 8))))))','(fn main () (let p (let x 11 (pair x 2)) (block (let x 9 x) (first p))))','(fn main () (call sum 3 4)) (fn sum (a b) (+ a b))','(fn main () (while false nil))'])
 return out
def main():
 binary=pathlib.Path(sys.argv[1]).resolve();dest=pathlib.Path(sys.argv[2]);packets=[];cases_count=0
 def run(source,fuel,fold,legacy=False):
  args=[str(binary),'--legacy' if legacy else '--program',source,str(fuel),'10000',str(fold)];p=subprocess.run(args,stdout=subprocess.PIPE,stderr=subprocess.STDOUT,text=True,timeout=10);assert p.returncode==0,p.stdout;packets.append(dict(command=args,returncode=p.returncode,stdout=p.stdout));lines=p.stdout.splitlines();assert lines[0]=='VC_COMPILE 0',p.stdout
  code=bytes.fromhex(next(s for s in lines if s.startswith('VC_CODE')).split()[2]);fns=[tuple(map(int,s.split()[1:])) for s in lines if s.startswith('VC_FN')];result=tuple(map(int,next(s for s in lines if s.startswith('VC_RESULT')).split()[1:]));return code,fns,result
 for source in cases():
  try:expected=source_value(source);status=0
  except Refusal as e:expected=-777;status=e.code
  for fold in [0,1]:
   code,fns,result=run(source,10000,fold);decode(code,fns);rc,value,left,steps=machine(code,fns,10000);assert(rc,value)==(status,expected),(source,rc,value,status,expected);assert result==(rc,tag(value),integer(value),left),(source,result,rc,value,left)
   if fold==0 and not re.search(r'\((and|or|break|block)\s',source):
    old,oldf,oldr=run(source,10000,0,True);assert (old,oldf,oldr)==(code,fns,result),'retained baseline compiler compatibility '+source
   for fuel in [max(0,steps-1),steps,steps+1]:
    raw,func,actual=run(source,fuel,fold);assert raw==code and func==fns;rc,value,left,_=machine(code,fns,fuel);assert actual==(rc,tag(value),integer(value),left),(source,fuel,actual,rc,value,left)
   cases_count+=1
 dest.write_text(json.dumps(dict(packets=packets,source_cases=len(cases()),lowering_cases=cases_count),indent=2)+'\n');print(json.dumps(dict(status='pass',source_cases=len(cases()),lowering_cases=cases_count,packets=len(packets),oracles=['independent tree evaluation','instruction decoding/stack/init joins','exact dispatch fuel machine'])))
if __name__=='__main__':main()
