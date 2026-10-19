"""Bounded ASCII identifier admission before any C publication; no token-pasting parser."""
import argparse,json,re
from pathlib import Path
def generate(d):
 prefix=d['prefix'];names=d['names']
 if prefix!='item_' or not isinstance(names,list) or not 1<=len(names)<=32:raise ValueError('registry shape or fixed publication namespace')
 tokens=[]
 for name in names:
  if not isinstance(name,str) or not re.fullmatch('[A-Za-z][A-Za-z0-9_]*',prefix) or not re.fullmatch('[A-Za-z][A-Za-z0-9_]*',name):raise ValueError('ASCII publication naming policy')
  token=prefix+name
  if token in tokens:raise ValueError('duplicate identifier')
  tokens.append(token)
 return '\n'.join('static const int '+n+'='+str(i)+';' for i,n in enumerate(tokens))+'\n'
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('input');p.add_argument('output');a=p.parse_args()
 try:text=generate(json.loads(Path(a.input).read_text(encoding='utf8')))
 except (ValueError,KeyError,TypeError) as e:print('LM_REJECT:'+str(e));raise SystemExit(1)
 Path(a.output).write_text(text,encoding='utf8')
