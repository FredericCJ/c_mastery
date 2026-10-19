"""Independent grammar recursion and checked exact-integer expression oracle."""
import json,random,re,subprocess,sys
SEED=150018
class Syntax(Exception):pass
class Arithmetic(Exception):
    def __init__(self,code):self.code=code
def checked(x):
    if not -2**31<=x<2**31:raise Arithmetic(3)
    return x
def tree(text,right):
    tokens=re.findall(r'\d+|\S',text);pos=0
    def take(k):
        nonlocal pos
        if pos<len(tokens) and tokens[pos]==k:pos+=1;return True
        return False
    def atom():
        nonlocal pos
        if take('('):
            v=add()
            if not take(')'):raise Syntax()
            return v
        if pos==len(tokens) or not tokens[pos].isdigit():raise Syntax()
        n=int(tokens[pos]);pos+=1
        if n>2**31-1:raise Syntax()
        return n
    # Unary and power mutually recurse, rather than operator-stack reduction.
    # Left-power mode still admits a signed right operand whose negation binds
    # below power: 2^-3^2 means 2^(-(3^2)), matching the specified Pratt contract.
    def unary():
        if take('-'):return ('neg',unary())
        return power()
    def power_rhs():
        if take('-'):return ('neg',unary())
        return atom()
    def power():
        v=atom()
        if right:
            if take('^'):v=('^',v,unary())
        else:
            while take('^'):v=('^',v,power_rhs())
        return v
    def mul():
        v=unary()
        while take('*'):v=('*',v,unary())
        return v
    def add():
        v=mul()
        while True:
            if take('+'):v=('+',v,mul())
            elif take('-'):v=('-',v,mul())
            else:return v
    out=add()
    if pos!=len(tokens):raise Syntax()
    return out
def evaluate(t):
    if isinstance(t,int):return t
    a=evaluate(t[1])
    if t[0]=='neg':return checked(-a)
    b=evaluate(t[2])
    if t[0]=='+':return checked(a+b)
    if t[0]=='-':return checked(a-b)
    if t[0]=='*':return checked(a*b)
    if not 0<=b<=31:raise Arithmetic(8)
    # Independent Python exact exponentiation, not the C repeated multiply.
    return checked(a**b)
def corpus():
    r=random.Random(SEED)
    cases=['2^3^2','-2^2','2^-3^2','2^--3','2^-2^-2','9-4-2','0^0','2147483647+1','-2147483647-1','(-2147483647-1)*(-1)','2^31','(-2)^31','1^32']
    def expression(depth):
        if depth==0 or r.randrange(4)==0:return str(r.choice([0,1,2,3,4,9,31,2147483647]))
        if r.randrange(5)==0:return '-'+expression(depth-1)
        return '('+expression(depth-1)+r.choice(['+','-','*','^'])+expression(depth-1)+')'
    cases += [expression(3) for _ in range(1800)]
    cases += [f'{a}{op}{b}{op2}{c}' for a in [0,2,3] for b in [0,2,3] for c in [0,2,3] for op in ['+','-','*','^'] for op2 in ['+','-','*','^']]
    cases += ['', '()', '1 2','2(3)','(1','1)','1+','*2','2^^3','(-)','(1)(2)']
    return [(mode,s) for mode in [0,1] for s in cases]
def main():
    values=corpus();raw=''.join(f'{mode} {s}\n' for mode,s in values)
    p=subprocess.run([sys.argv[-1]],input=raw,capture_output=True,text=True,timeout=120)
    assert p.returncode==0,p.stderr
    lines=p.stdout.splitlines();assert len(lines)==len(values)
    valid=invalid=0
    for (mode,s),line in zip(values,lines):
        lex,parsed,pratt,ev,stream,same,result=map(int,line.split())
        try:t=tree(s,mode)
        except Syntax:
            assert lex==0 and parsed==4 and pratt!=0,(mode,s,line);invalid+=1;continue
        valid+=1
        try:want=evaluate(t);status=0
        except Arithmetic as e:want=123;status=e.code
        assert (lex,parsed,pratt,ev,stream,same,result)==(0,0,0,status,0,1,want),(mode,s,line,status,want)
    print(json.dumps(dict(passed=True,seed=SEED,observations=len(values),valid=valid,malformed=invalid,scope='Independent recursive grammar/exact integer oracle plus actual prior Pratt/evaluator and transactional streaming comparison.')))
if __name__=='__main__':main()
