"""Observe isolated ASan/LSan faulty children beside repaired controls."""
import argparse,json,os,subprocess
p=argparse.ArgumentParser();p.add_argument('--faulty',required=True);p.add_argument('--repaired',required=True);a=p.parse_args()
expected={'overflow':'heap-buffer-overflow','uaf':'heap-use-after-free','leak':'LeakSanitizer','stack-buffer':'stack-buffer-overflow'}
env=dict(os.environ,ASAN_OPTIONS='detect_leaks=1:halt_on_error=1',LSAN_OPTIONS='exitcode=23')
for name,diagnostic in expected.items():
 case_env=dict(env)
 default=None
 if name=='leak':
  default=subprocess.run([a.faulty,name],capture_output=True,text=True,timeout=5,env=env)
  # This closed child intentionally discards its only owner. Conservative stale
  # stack/register bits can retain the allocation. Compare, do not hide, that
  # outcome with a deliberately narrower root scan. Never generalize this setting
  # to an application with legitimate live stack/register roots.
  case_env['LSAN_OPTIONS']='exitcode=23:use_stacks=0:use_registers=0'
 fixed=subprocess.run([a.repaired,name],capture_output=True,text=True,timeout=5,env=case_env)
 bad=subprocess.run([a.faulty,name],capture_output=True,text=True,timeout=5,env=case_env)
 if fixed.returncode or bad.returncode==0 or diagnostic not in bad.stderr:
  raise SystemExit(json.dumps({'case':name,'fixed_exit':fixed.returncode,'bad_exit':bad.returncode,'fixed':fixed.stderr,'bad':bad.stderr}))
 print(json.dumps({'case':name,'repaired_exit':fixed.returncode,'faulty_exit':bad.returncode,'observed_diagnostic_family':diagnostic,'default_leak_exit':default.returncode if default else None,'lsan_options':case_env['LSAN_OPTIONS'],'claim':'particular instrumented child; reduced-root leak comparison is not a general ownership proof or portable native symptom'}))
