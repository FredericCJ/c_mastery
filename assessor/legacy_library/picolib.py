import hashlib,json,re
from pathlib import Path

def run(c,root,args):
 c.group='library-picolib';pub=root/'labs/foundations/library_depth';private=root/'assessor/legacy_library';effects=Path('/home/frede/cmastery-effects-tools');upstream=effects/'picolibc-1.8.10';custom=effects/'install-picolibc-custom/lib/picolibc';posix=effects/'install-picolibc-double/lib/picolibc'
 opts=(upstream/'meson_options.txt').read_text();names=set(re.findall(r"option\('([^']+)'",opts));c.require('posix-io' not in names and {'tinystdio','posix-console','tests-enable-posix-io'}<=names,'exact tagged options contradict old posix-io premise',sorted(n for n in names if 'posix' in n or n=='tinystdio'))
 c.run(['python3',effects/'usr-root/usr/bin/meson','setup',c.root/'pico-invalid-option',upstream,'-Dposix-io=false'],expected=1,contains='Unknown option: "posix-io"',env={'PYTHONPATH':str(effects/'usr-root/usr/lib/python3/dist-packages')})
 for cc in ('gcc','clang'):
  for profile,opt in [('debug','-O0'),('release','-O2')]:
   exe=c.root/(cc+'-'+profile+'-picolib');m=c.root/(exe.name+'.map');c.run([cc,'-std=c17',opt,'-fno-builtin','-Wall','-Wextra','-Werror','-I'+str(pub),'-isystem',custom/'include',pub/'picolib_tests.c',private/'picolib.c',custom/'lib/libc.a','-lm','-Wl,-Map='+str(m),'-o',exe]);c.run([exe]);c.require(str(custom/'lib/libc.a') in m.read_text(),'actual custom-console archive '+cc+'-'+profile);c.run(['nm','-u',exe]);c.run(['readelf','-d',exe])
 for name,source,flag,code in [('picolib-drop',private/'picolib.c','LP_MUTANT_DROP',13),('picolib-learner',pub/'picolib.c','UNSOLVED',11)]:
  exe=c.root/name;c.run(['gcc','-std=c17','-O2','-fno-builtin','-D'+flag,'-I'+str(pub),'-isystem',custom/'include',pub/'picolib_tests.c',source,custom/'lib/libc.a','-lm','-o',exe]);c.run([exe],expected=code)
 exe=c.root/'picolib-file';m=c.root/'picolib-file.map';base=['gcc','-std=c17','-O2','-fno-builtin','-isystem',posix/'include',private/'picolib_file.c',posix/'lib/libc.a','-lm'];c.run([*base,'-Wl,-Map='+str(m),'-o',exe]);fixture=c.root/'picolib-synthetic.txt';c.run([exe,fixture]);c.require(fixture.read_bytes()==b'synthetic','actual seekable file roundtrip',dict(bytes=9,content='synthetic'));imports=c.run(['nm','-u',exe]);c.require(all(re.search(r'\b'+n+r'(@|\s|$)',imports) for n in ('open','close','read','write','lseek')),'POSIX file backend runtime closure',imports);c.run(['readelf','-d',exe]);c.require(str(posix/'lib/libc.a') in m.read_text(),'actual POSIX-console archive linked')
 for missing in ('read','lseek'):
  c.run([*base,'-Wl,--wrap='+missing,'-o',c.root/('picolib-missing-'+missing)],expected='nonzero',contains='__wrap_'+missing)
 paths=[upstream/p for p in ('doc/os.md','doc/build.md','meson_options.txt','newlib/libc/tinystdio/fseek.c')]+[custom/'lib/libc.a',posix/'lib/libc.a',effects/'build-picolibc-custom/resume_commands.json',effects/'build-picolibc-double/commands.json'];c.require(True,'exact Picolibc sources builds and libraries',{str(p):hashlib.sha256(p.read_bytes()).hexdigest() for p in paths})
 for field,build in [('false','build-picolibc-custom/resume_commands.json'),('true','build-picolibc-double/commands.json')]:
  rows=json.loads((effects/build).read_text());c.require(all(r['exit_code']==0 for r in rows) and '-Dposix-console='+field in rows[0]['command'],'retained supported configuration '+field,rows)
 c.observe('T12.16.b','Versioned correction rejects unsupported tagged posix-io option; actual custom-console and POSIX-console Picolibc1.8.10 archives execute, with object/link closure and missing read/lseek rejection',['library-picolib'])
 c.observe('T12.16.b','UART-like original memory console supplies concrete FILE callbacks, bounded output failure and synthetic input; seek refuses ESPIPE; native surrogate does not invent UART registers or board observations',['library-picolib'],True)
