"""Independent mathematical/state oracles for B06 data (not a C execution receipt)."""
from pathlib import Path
import ast,json,operator,functools,zlib
from decimal import Decimal
ROOT=Path(__file__).resolve().parents[1]
def shrink_sum(values,threshold):
    result=list(values);i=0
    while i<len(result):
        candidate=result[:i]+result[i+1:]
        if sum(candidate)>=threshold:result=candidate
        else:i+=1
    return result
def iov_position(lengths,progress):
    for i,length in enumerate(lengths):
        if progress<length:return [i,progress]
        progress-=length
    if progress:raise ValueError('excess progress')
    return [len(lengths),0]
def round_ratio_even(n,d):
    sign=-1 if n<0 else 1;q,r=divmod(abs(n),d)
    return sign*(q+int(2*r>d or (2*r==d and q%2)))
def q15(a,b):
    product=a*b;value=(abs(product)+16384)//32768
    return min(32767,max(-32768,-value if product<0 else value))
def decimal_close(values):
    a,e,at,rt=map(Decimal,values)
    return int(abs(a-e)<=at+rt*abs(e))
def evaluate(expression,values):
    tree=ast.parse(expression,mode='eval')
    allowed=(ast.Expression,ast.Constant,ast.Name,ast.Load,ast.Store,ast.List,ast.Tuple,ast.Dict,ast.Subscript,ast.Slice,ast.BinOp,ast.UnaryOp,ast.BoolOp,ast.Compare,ast.IfExp,ast.Call,ast.GeneratorExp,ast.ListComp,ast.comprehension,ast.Add,ast.Sub,ast.Mult,ast.Div,ast.FloorDiv,ast.Mod,ast.Pow,ast.BitXor,ast.BitAnd,ast.BitOr,ast.LShift,ast.RShift,ast.Invert,ast.Not,ast.USub,ast.UAdd,ast.And,ast.Or,ast.Eq,ast.NotEq,ast.Lt,ast.LtE,ast.Gt,ast.GtE,ast.In,ast.NotIn,ast.Starred,ast.Attribute)
    for node in ast.walk(tree):
        if not isinstance(node,allowed):raise ValueError('oracle syntax '+type(node).__name__)
        if isinstance(node,ast.Attribute) and not (node.attr=='encode' and isinstance(node.value,ast.Name) and node.value.id=='input'):raise ValueError('oracle attribute')
        if isinstance(node,ast.Name) and node.id.startswith('_'):raise ValueError('oracle private name')
    env=dict(__builtins__={},input=values,list=list,sum=sum,len=len,range=range,enumerate=enumerate,zip=zip,min=min,max=max,int=int,str=str,all=all,any=any,sorted=sorted,abs=abs,crc32=zlib.crc32,xor_all=lambda x:functools.reduce(operator.xor,x,0),shrink_sum=shrink_sum,iov_position=iov_position,q15=q15,round_ratio_even=round_ratio_even,decimal_close=decimal_close)
    return eval(compile(tree,'<authored bounded oracle>','eval'),env)
def main():
    records=[]
    for path in sorted(ROOT.glob('fixtures/b06_t*/*.json')):
        data=json.loads(path.read_text(encoding='utf-8'))
        try:observed=evaluate(data['oracle'],data['input']);passed=observed==data['expected'];error=None
        except Exception as exc:observed=None;passed=False;error=str(exc)
        records.append(dict(path=path.relative_to(ROOT).as_posix(),passed=passed,expected=data['expected'],observed=observed,error=error))
    report=dict(passed=bool(records) and all(r['passed'] for r in records),count=len(records),checks=records,limits='Independent fixture derivation only; does not execute C, prove algorithms, or validate hardware.')
    print(json.dumps(report,indent=2));return 0 if report['passed'] else 1
if __name__=='__main__':raise SystemExit(main())
