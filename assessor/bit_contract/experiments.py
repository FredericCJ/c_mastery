"""Actual packing, intrinsic and independently compiled plugin investigations."""
import json,re
def run(c,root):
 lab=root/'labs/bit_contract';ref=root/'assessor/bit_contract';integer=root/'assessor/integer_kernel/integer.c';include=root/'labs/integer_kernel'
 common=['-std=c17','-Wall','-Wextra','-Werror','-I'+str(lab),'-I'+str(include)]
 sources=[ref/'bits.c',integer];layouts=[]
 for cc in ['gcc','clang']:
  c.group=cc+':analysis-only';c.run([cc,*common,'-O2','-c',ref/'analysis_only.c','-o',c.root/(cc+'-analysis.o')])
  for mutant in [False,True]:
   c.group=cc+':intrinsic:'+str(mutant);exe=c.root/(cc+'-intrinsic-'+str(mutant))
   c.run([cc,*common,'-O2',*(['-DFB_INTRINSIC_ZERO'] if mutant else []),*sources,ref/'intrinsics.c','-o',exe])
   c.run([exe],expected=int(mutant),contains='FB_INTRINSIC_FAIL width=1 value=0' if mutant else 'FB_INTRINSIC_PASS cases=1048576 zero_guard logical_width')
  for opt in ['0','3']:
   for pack in [0,1]:
    c.group=cc+':layout-O'+opt+':'+str(pack);exe=c.root/(cc+'-layout-'+opt+'-'+str(pack))
    c.run([cc,*common,'-O'+opt,'-DFB_PACK='+str(pack),*sources,ref/'layout.c','-o',exe]);text=c.run([exe],contains='FB_LAYOUT_PASS values=5,17,257 no_padding_goldens no_bitfield_address')
    rows={m[0]:dict(size=int(m[1]),alignment=int(m[2]),bytes=m[3]) for m in re.findall(r'LAYOUT (\w+) size=(\d+) align=(\d+) bytes=([0-9a-f]+)',text)}
    c.require(set(rows)=={'fields359','wide359','record','reordered','packed'} and all(len(r['bytes'])==2*r['size'] for r in rows.values()),'actual tagged representation observations')
    c.require('PACKED_WIRE_OBS value=2018915346 count=512 portable_decoder=305419896' in text,'actual little-endian packed interpretation differs from wire contract')
    layouts.append(dict(compiler=cc,options=common+['-O'+opt,'-DFB_PACK='+str(pack)],layouts=rows,raw=text,limits='Padding bytes and bit-field placement are observations, not portable golden outputs; actual compiler/ABI/options required.'))
    c.run(['objdump','-drwC',exe])
 for cc in ['gcc','clang']:
  for opt in ['0','3']:
   normal=next(x for x in layouts if x['compiler']==cc and '-O'+opt in x['options'] and '-DFB_PACK=0' in x['options'])
   packed=next(x for x in layouts if x['compiler']==cc and '-O'+opt in x['options'] and '-DFB_PACK=1' in x['options'])
   c.require(normal['layouts']['record']['size']!=packed['layouts']['record']['size'] and normal['layouts']['fields359']['size']!=normal['layouts']['wide359']['size'],'actual incompatible layout assumptions exposed')
 c.write('layout-manifest.json',json.dumps(dict(observations=layouts,wire_record=dict(version=1,size=8,fields=[dict(name='version',offset=0,octets=1),dict(name='flags',offset=1,octets=1),dict(name='size',offset=2,octets=2,endian='big'),dict(name='value',offset=4,octets=4,endian='big')]),plugin=dict(entry='size_t fb_plugin_copy(unsigned char *,size_t)',abi='recorded Linux x86-64 SysV C ABI; no cross-ISA guarantee',descriptor_version1=dict(size=8,version_offset=0,size_offset=2,features_offset=4,endian='big'),descriptor_version2=dict(minimum_size=12,max_items_offset=8,endian='big'),consumer_validation=['available prefix','declared size within available object','version','required field minimum','feature mask'],unverifiable=['actual plugin code safety','truthfulness of foreign buffer extent','compiler ABI compatibility from version alone'])),indent=2)+'\n')
 for provider in ['gcc','clang']:
  for version in [1,2,3,4]:
   c.group=provider+':plugin:'+str(version);soname='libfb-'+provider+'-'+str(version)+'.so';lib=c.root/soname
   c.run([provider,*common,'-O2','-fPIC','-shared','-fpack-struct=1','-DFB_PLUGIN_VERSION='+str(version),ref/'plugin.c','-Wl,-soname,'+soname,'-o',lib]);c.run(['readelf','-d',lib],contains=soname);symbols=c.run(['nm','-D','--defined-only',lib],contains='fb_plugin_copy');c.require(sum(' T ' in x for x in symbols.splitlines())==1,'single typed plugin entry')
   for consumer in ['gcc','clang']:
    c.group=provider+':plugin:'+str(version)+':'+consumer;exe=c.root/(provider+'-'+str(version)+'-'+consumer+'-client')
    c.run([consumer,*common,'-O1','-g','-fno-pie','-no-pie','-fsanitize=address,undefined',*sources,ref/'plugin_client.c','-L'+str(c.root),'-l:'+soname,'-Wl,-rpath,'+str(c.root),'-o',exe])
    c.run([exe,str(version)],contains='FB_PLUGIN_PASS variant='+str(version)+' exact_available='+str(8 if version in [1,4] else 12 if version==2 else 4)+' negotiated_without_tail_read',env={'ASAN_OPTIONS':'detect_leaks=1:abort_on_error=1','UBSAN_OPTIONS':'halt_on_error=1'})
