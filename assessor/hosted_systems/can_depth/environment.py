"""Read-only CAN environment discovery; no sockets or interface mutations."""
import hashlib,json,pathlib,subprocess
command=['ip','-details','-json','link','show'];r=subprocess.run(command,capture_output=True,text=True)
if r.returncode:raise SystemExit(r.stderr)
links=json.loads(r.stdout)
vcan=[dict(name=x['ifname'],index=x['ifindex'],kind=x.get('linkinfo',{}).get('info_kind'),flags=x['flags']) for x in links if x.get('linkinfo',{}).get('info_kind')=='vcan']
headers={str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in [pathlib.Path('/usr/include/linux')/n for n in ['can.h','can/raw.h','can/error.h']]}
version=subprocess.run(['dpkg-query','-W','linux-libc-dev'],capture_output=True,text=True,check=True)
print(json.dumps(dict(discovery_command=command,exit=r.returncode,virtual_interfaces=vcan,selected_backend='original-frame-adapter',kernel_can_execution=False,headers_sha256=headers,header_package=version.stdout.strip(),limits='An interface listing alone does not establish ownership/isolation; no CAN socket opened.')))
