import sys
from pathlib import Path
rows=0
for line in Path(sys.argv[1]).read_text().splitlines():
 if not line.startswith('C '):continue
 page,address,n,rc,checksum,copied,status,partial_sum=map(int,line.split()[1:]);memory=[55]*64;partial=[55]*64
 if address>64 or n>64-address:want_rc=2;want_status=2;want_copied=0
 else:
  if n:memory[page*16:(page+1)*16]=[88]*16
  touched=set(range(address//16,(address+n-1)//16+1)) if n else set()
  want_rc=3 if page in touched else 0
  if want_rc==0:memory[address:address+n]=range(1,n+1)
  want_copied=0;want_status=0
  for i in range(n):
   if (address+i)//16==page:want_status=3;break
   partial[address+i]=i+1;want_copied+=1
 expected=(want_rc,sum((i+1)*v for i,v in enumerate(memory)),want_copied,want_status,sum((i+1)*v for i,v in enumerate(partial)))
 assert (rc,checksum,copied,status,partial_sum)==expected,(line,expected);rows+=1
assert rows==2376,rows
print('SX_ORACLE_PASS copy=2376 exact_page_bytes_prefix_and_callback_effects')
