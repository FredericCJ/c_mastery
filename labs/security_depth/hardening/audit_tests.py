"""Public synthetic audit boundaries, not a substitute for actual binaries."""
import copy,importlib.util,json,pathlib,sys
def main():
 p=pathlib.Path(sys.argv[1]);s=importlib.util.spec_from_file_location('candidate',p);m=importlib.util.module_from_spec(s);s.loader.exec_module(m)
 f=json.loads(pathlib.Path(sys.argv[2]).read_text());v=f['input']
 def check(ok,label):
  if not ok:print('SH_ASSERT:'+label);raise SystemExit(41)
 check(m.classify(v)==f['expected'],'declared_fixture')
 for field,label in [('pie_flag','DYN_alone_not_PIE'),('bind_now','RELRO_not_eager')]:
  x=copy.deepcopy(v);x[field]=False;r=m.classify(x);check(r['pie_marked' if field=='pie_flag' else 'eager_binding'] is False,label)
 x=copy.deepcopy(v);x['symbols']=[];r=m.classify(x);check(not r['guard_site_dependency'] and not r['checked_call_present'],'symbol_absence')
 x=copy.deepcopy(v);x['headers'][0]['flags']='RWE';x['headers'][1]['flags']='RWE';r=m.classify(x);check(not r['nx_stack_marked'] and r['wx_load_segment'],'executable_permissions')
 print('OBS declared_audit five independent predicate controls')
if __name__=='__main__':main()
