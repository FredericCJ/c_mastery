"""Materialize scoped original-source controls in the build tree only."""
import argparse,pathlib
p=argparse.ArgumentParser();p.add_argument('--input',required=True);p.add_argument('--output',required=True);p.add_argument('--which',type=int,required=True);a=p.parse_args()
s=pathlib.Path(a.input).read_text(encoding='utf8')
assert s.count('invalidate(s);')==2
for view,which in [(0,6),(1,7)]:
 token=f's->wal_active = {view};\n  invalidate(s);'
 assert s.count(token)==1
 if a.which in [which,8]:s=s.replace(token,f's->wal_active = {view};\n  /* original teaching control: omit this invalidation */')
# The helper itself remains used in the single-removal controls. Both removals
# must compile under Werror too; retain a harmless invalidation of a private
# zeroed temporary rather than suppressing unused-function diagnostics.
if a.which==8:s=s.replace('int store_page_read(store *s,', 'static void keep_control_helper(void){store temp={0};invalidate(&temp);}\nint store_page_read(store *s,').replace('  if (!s || !out || id >= STORE_DATA_PAGES', '  keep_control_helper();\n  if (!s || !out || id >= STORE_DATA_PAGES')
pathlib.Path(a.output).write_text(s,encoding='utf8',newline='\n')
