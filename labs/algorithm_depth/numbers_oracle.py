"""Independent Python arbitrary-precision conversion oracle; host only."""
import json,random,subprocess,sys
DIGITS='0123456789abcdef'
def representation(value,base):
 sign='-' if value<0 else '';value=abs(value);digits=[]
 while value: value,digit=divmod(value,base);digits.append(DIGITS[digit])
 return sign+(''.join(reversed(digits)) or '0')
def parse(text,base,prefix,whole,signed):
 if not text:return 1,77,88
 at=0;negative=False
 if text[0] in b'+-':
  negative=text[0]==45
  if negative and not signed:return 1,77,88
  at+=1
 if prefix and base==16 and text[at:at+2].lower()==b'0x':at+=2
 begin=at;value=0;limit=((1<<31) if negative else (1<<31)-1) if signed else (1<<32)-1
 while at<len(text):
  digit=DIGITS.find(chr(text[at]).lower())
  if digit<0 or digit>=base:break
  value=value*base+digit
  if value>limit:return 3,77,88
  at+=1
 if at==begin or (whole and at!=len(text)):return 1,77,88
 return 0,-value if negative else value,at
def main():
 rng=random.Random(1511);commands=[];expected=[]
 values=[-(1<<31),(1<<31)-1,-1,0,1]+[rng.randrange(-(1<<31),1<<31) for _ in range(128)]
 for base in range(2,17):
  for value in values:commands.append(f'F {value} {base}');text=representation(value,base);expected.append(f'0 {len(text)} {text}')
 texts=[b'',b'+',b'-',b'+0',b'-0',b'0x',b'0X10',b'2147483647',b'2147483648',b'-2147483648',b'-2147483649',b'4294967295',b'4294967296',b'12x',b' 1',b'1 ',b'1\x002',b'ff',b'101020']
 texts += [bytes(rng.choice(b'+-0123456789abcdefxX \x00') for _ in range(rng.randrange(1,21))) for _ in range(256)]
 for mode in ['I','U']:
  for base in [2,10,16]:
   for prefix in [0,1]:
    for whole in [0,1]:
     for text in texts:
      commands.append(f'{mode} {base} {prefix} {whole} {len(text)} {text.hex() or "-"}')
      expected.append(' '.join(map(str,parse(text,base,prefix,whole,mode=='I'))))
 result=subprocess.run([sys.argv[1]],input='\n'.join(commands)+'\n',text=True,capture_output=True,check=True)
 observed=result.stdout.splitlines();assert len(observed)==len(expected),(len(observed),len(expected))
 for command,want,got in zip(commands,expected,observed):assert want==got,(command,want,got)
 print(json.dumps(dict(status='pass',seed=1511,cases=len(commands),oracle='Python unbounded accumulation and divmod formatting; no target conversion library')))
if __name__=='__main__':main()
