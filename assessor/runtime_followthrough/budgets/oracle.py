"""Independent nine-instruction cost derivation, not an interpreter or collector."""
import pathlib,json,sys
rows=[]
for fuel in range(11):
 for gc in range(21):
  for host in range(3):
   # INT,INT,PAIR,STORE,GC,LOAD,FIRST,HOST,RET. GC has one local root,
   # two scalar fields and an all-or-none16-cell sweep charge.
   used_gc=used_host=collections=effects=0
   allocations=int(fuel>=3)
   if fuel<5:status,used_fuel=7,fuel
   elif gc<19:status,used_fuel,used_gc=9,5,min(gc,3)
   else:
    used_gc,collections=19,1
    if fuel<8:status,used_fuel=7,fuel
    elif host==0:status,used_fuel=10,8
    else:
     used_host=effects=1
     status,used_fuel=(7,8) if fuel==8 else (0,9)
   value=18 if status==0 else 91
   rows.append(' '.join(map(str,[fuel,gc,host,status,used_fuel,used_gc,used_host,allocations,collections,effects,value])))
pathlib.Path(sys.argv[1]).write_text('\n'.join(rows)+'\n')
print(json.dumps(dict(cases=len(rows),scope='All11fuel x21GC x3host budgets around a fixed independently derived nine-instruction path; original status numbers explicitly encoded.')))
