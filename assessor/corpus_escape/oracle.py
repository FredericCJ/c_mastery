"""Independent table-language model; no calls to the C implementation."""
import argparse,itertools,json,subprocess
p=argparse.ArgumentParser();p.add_argument('--driver');a=p.parse_args()
table={38:b'&amp;',60:b'&lt;',62:b'&gt;',34:b'&quot;',39:b'&apos;'}
html=dict(table);html[39]=b'&#39;'
def admitted(c):return 32<=c<=126 or c in (9,10)
def model(op,s,cap):
 if cap>384:return 1,None
 if len(s)>(64 if op==0 else 384):return 2,None
 if op==0:
  if not all(map(admitted,s)):return 3,None
  result=b''.join(table.get(c,bytes([c])) for c in s)
 else:
  replacements={v:k for k,v in (html if op==2 else table).items()};result=bytearray();i=0
  while i<len(s):
   c=s[i]
   if c==38:
    end=s.find(b';',i)
    if end<0 or s[i:end+1] not in replacements:return 3,None
    c=replacements[s[i:end+1]];i=end+1
   else:
    if not admitted(c) or c in table:return 3,None
    i+=1
   if len(result)==64:return 2,None
   result.append(c)
  if op==2:result=b''.join(table.get(c,bytes([c])) for c in result)
 if len(result)>cap:return 4,None
 return 0,result
cases=[]
def add(op,s,cap):cases.append((op,s,cap))
alphabet=b"a&<'"
for n in range(6):
 for chars in itertools.product(alphabet,repeat=n):
  s=bytes(chars);encoded=b''.join(table.get(c,bytes([c])) for c in s);generated=b''.join(html.get(c,bytes([c])) for c in s)
  for cap in sorted({0,len(encoded),max(0,len(encoded)-1),384}):add(0,s,cap)
  for cap in sorted({0,len(s),max(0,len(s)-1),64}):add(1,encoded,cap)
  add(2,generated,384)
for c in range(256):
 add(0,bytes([c]),384);add(1,bytes([c]),64)
for word in list(table.values())+[b'&#39;',b'&#x27;',b'&unknown;',b'&AMP;',b'&amp;lt;x&amp;gt;',b'&amp;#39;',b'&amp;&']:
 for n in range(len(word)+1):
  for op in (1,2):add(op,word[:n],384)
for n in (63,64,65):
 for c in (b'a',b'"'):
  add(0,c*n,384);add(1,b''.join(table.get(x,bytes([x])) for x in c*n),64)
for op in range(3):add(op,b'a',385);add(op,b'a'*385,384)
commands=[];expected=[]
for op,s,cap in cases:
 commands.append(' '.join(map(str,[op,cap,len(s),*s])))
 status,result=model(op,s,cap)
 expected.append(' '.join(map(str,[status,1] if status else [0,len(result),*result])))
if a.driver:
 result=subprocess.run([a.driver],input='\n'.join(commands)+'\n',capture_output=True,text=True,timeout=60);assert result.returncode==0,result.stderr
 actual=result.stdout.splitlines();assert len(actual)==len(expected),(len(actual),len(expected))
 for i,(x,y) in enumerate(zip(actual,expected)):assert x==y,(cases[i],x,y)
print(json.dumps(dict(passed=True,cases=len(cases),scope='All strings through length5 over four-byte alphabet, all byte values, entity prefixes, exact/cut capacities and length bounds. Single-pass table model; no browser or general XML equivalence.')))
