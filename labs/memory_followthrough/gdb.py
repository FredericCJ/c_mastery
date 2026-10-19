import gdb,json
rows=[]
def run(s):
 try:out=gdb.execute(s,to_string=True)
 except gdb.error as e:out='UNAVAILABLE:'+str(e)
 rows.append({'command':s,'output':out})
for s in ('set pagination off','set confirm off','set disable-randomization off','set debuginfod enabled off','break mw_marker if stage == 1','run','up','watch cells[1]','continue','print cells[1]','info breakpoints','delete breakpoints','break mw_marker if stage == 3','continue','up','watch *p','info breakpoints','delete breakpoints','break mw_marker if stage == 4','continue','up','print generation','watch *p','continue','print *p','info breakpoints','delete breakpoints','continue'):run(s)
print('MW_GDB:'+json.dumps({'records':rows,'heap':'only-selected-live-object','omitted_memory':'unknown','ASLR':'preserved','native_core':False}))
