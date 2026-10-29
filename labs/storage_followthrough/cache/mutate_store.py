"""Materialize scoped original-source controls in the build tree only."""
import argparse,pathlib,re
def spans(text,anchor,start=0):
 tokens=re.findall(r'[A-Za-z_]\w*|\d+|[^\s]',anchor);pattern=r'\s*'.join(re.escape(x) for x in tokens);found=[]
 if tokens[0][0].isalnum() or tokens[0][0]=='_':pattern=r'(?<!\w)'+pattern
 if tokens[-1][-1].isalnum() or tokens[-1][-1]=='_':pattern+=r'(?!\w)'
 for m in re.finditer(pattern,text):
  if m.start()<start:continue
  lo,hi=m.span()
  while anchor[:1] in (' ','\t') and text[lo-1:lo] in (' ','\t'):lo-=1
  if anchor.endswith('\n'):
   n=re.compile(r'[ \t]*\r?\n').match(text,hi);hi=n.end() if n else hi
  found.append((lo,hi))
 return found
def once_span(text,anchor):
 found=spans(text,anchor)
 if len(found)!=1:raise SystemExit(f'C anchor must occur exactly once ({len(found)}): '+anchor[:80])
 return found[0]
def once(text,old,new):
 lo,hi=once_span(text,old);return text[:lo]+new+text[hi:]
p=argparse.ArgumentParser();p.add_argument('--input',required=True);p.add_argument('--output',required=True);p.add_argument('--which',type=int,required=True);a=p.parse_args()
s=pathlib.Path(a.input).read_text(encoding='utf8')
assert len(spans(s,'invalidate(s);'))==2
for view,which in [(0,6),(1,7)]:
 token=f's->wal_active = {view};\n  invalidate(s);'
 assert len(spans(s,token))==1
 if a.which in [which,8]:s=once(s,token,f's->wal_active = {view};\n  /* original teaching control: omit this invalidation */')
# The helper itself remains used in the single-removal controls. Both removals
# must compile under Werror too; retain a harmless invalidation of a private
# zeroed temporary rather than suppressing unused-function diagnostics.
if a.which==8:s=once(once(s,'int store_page_read(store *s,', 'static void keep_control_helper(void){store temp={0};invalidate(&temp);}\nint store_page_read(store *s,'),'  if (!s || !out || id >= STORE_DATA_PAGES', '  keep_control_helper();\n  if (!s || !out || id >= STORE_DATA_PAGES')
pathlib.Path(a.output).write_text(s,encoding='utf8',newline='\n')
