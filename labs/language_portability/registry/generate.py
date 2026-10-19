"""Validate one authoritative registry before producing all deterministic views."""
import argparse,hashlib,json,re
from pathlib import Path
HANDLERS={'start':1,'stop':4,'status':7,'reset':9}
def render(source):
 rows=source['rows']
 if not isinstance(rows,list) or not 1<=len(rows)<=32:raise ValueError('row count')
 ids=set();names=set();ordered=[];aliases=[]
 for row in rows:
  ident=row['id'];name=row['name'];handler=row['handler'];extra=row.get('aliases',[])
  if type(ident) is not int or not 0<=ident<=2147483647 or ident in ids:raise ValueError('ID admission')
  if handler not in HANDLERS or not isinstance(extra,list) or len(extra)>32:raise ValueError('handler/alias admission')
  ids.add(ident)
  for label in [name]+extra:
   if not isinstance(label,str) or not re.fullmatch('[a-z][a-z0-9_]{0,30}',label) or label in names:raise ValueError('name admission')
   names.add(label)
  ordered.append(dict(id=ident,name=name,handler=handler,aliases=sorted(extra)))
  aliases.extend((label,ident) for label in extra)
 if len(names)>64:raise ValueError('total names')
 ordered.sort(key=lambda r:r['id']);aliases.sort()
 enum='enum LrCommand {\n'+''.join(' LR_CMD_'+r['name'].upper()+'='+str(r['id'])+',\n' for r in ordered)+'};\n'
 names_text=''.join('{'+str(r['id'])+'u,"'+r['name']+'"},\n' for r in ordered)
 dispatch=''.join('{'+str(r['id'])+'u,lr_handle_'+r['handler']+'},\n' for r in ordered)
 alias_text=''.join('{"'+n+'",'+str(i)+'u},\n' for n,i in aliases) or '{(const char *)0,0u},\n'
 header='#define LR_COMMAND_COUNT '+str(len(ordered))+'u\n#define LR_ALIAS_COUNT '+str(len(aliases))+'u\n'+enum
 validation=dict(rows=ordered,canonical_order='ascending ID',aliases=aliases,direct_index_slots=max(ids)+1,search_entries=len(ordered))
 result={'ids.h':header,'names.inc':names_text,'dispatch.inc':dispatch,'aliases.inc':alias_text,'validation.json':json.dumps(validation,sort_keys=True,indent=2)+'\n'}
 result['manifest.json']=json.dumps({name:hashlib.sha256(text.encode()).hexdigest() for name,text in result.items()},sort_keys=True,indent=2)+'\n'
 return result
if __name__=='__main__':
 p=argparse.ArgumentParser();p.add_argument('registry');p.add_argument('output');p.add_argument('--check',action='store_true');a=p.parse_args()
 try:result=render(json.loads(Path(a.registry).read_text(encoding='utf8')))
 except (ValueError,TypeError,KeyError) as e:print('LR_REJECT:'+str(e));raise SystemExit(1)
 out=Path(a.output)
 if a.check:
  bad=[n for n,t in result.items() if not (out/n).is_file() or (out/n).read_bytes()!=t.encode()]
  print(json.dumps(dict(passed=not bad,stale=bad)));raise SystemExit(bool(bad))
 out.mkdir(parents=True,exist_ok=True)
 for name,text in result.items():(out/name).write_text(text,encoding='utf8',newline='\n')
