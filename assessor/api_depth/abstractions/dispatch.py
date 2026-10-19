import json,re,statistics
def run(c,root,args):
 c.group='abstractions-dispatch';pub=root/'labs/api_depth/abstractions';private=root/'assessor/api_depth/abstractions';source=private/'dispatch.c';backend=private/'dispatch_backend.c';provider=root/'assessor/api_depth/stream.c';common=['-std=c17','-Wall','-Wextra','-Werror','-Wno-misleading-indentation','-I'+str(pub),'-I'+str(root/'labs/api_depth/stream')]
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+name+'-dispatch');c.run([cc,*common,*flags,pub/'dispatch_tests.c',source,backend,provider,'-o',exe]);c.run([exe],contains='DISPATCH-PASS:conformance=455;fault_paths=12;third_backend=all;')
 for name,flag,selected,back in [('dispatch-table','AD_MUTANT_DISPATCH',source,backend),('dispatch-bytes','AD_MUTANT_BYTE',source,backend),('dispatch-learner','UNSOLVED',pub/'dispatch.c',backend),('backend-learner','UNSOLVED',source,pub/'dispatch_backend.c')]:
  exe=c.root/name;c.run(['gcc',*common,'-O2','-D'+flag,pub/'dispatch_tests.c',selected,back,provider,'-o',exe]);c.run([exe],expected=1,contains='DISPATCH-CHECK:')
 measurements=[];code=[]
 for cc in ('gcc','clang'):
  for opt in ('-O0','-O2'):
   for lto in (False,True):
    for shared in (False,True):
     label=cc+opt+('-lto' if lto else '')+('-shared' if shared else '-static');flags=[opt,*(['-flto'] if lto else [])];exe=c.root/(label+'-dispatch-cost');link=[backend]
     if shared:
      lib=c.root/('lib'+label.replace('-','_')+'.so');c.run([cc,*common,opt,'-fPIC','-shared',backend,'-Wl,-soname,'+lib.name,'-o',lib]);link=[lib,'-Wl,-rpath,$ORIGIN'];c.run(['readelf','-Ws',lib]);c.run(['objdump','-dr',lib])
     c.run([cc,*common,*flags,private/'dispatch_cost.c',source,provider,*link,'-o',exe]);dis=c.run(['objdump','-dr',exe]);sections=c.run(['size','-A',exe]);dynamic=c.run(['readelf','-d',exe]);symbols=c.run(['nm',exe]);c.require('sf_pump' in symbols or lto,'actual provider visible or LTO enabled '+label,symbols)
     if shared:c.require(lib.name in dynamic,'actual shared backend NEEDED '+label,dynamic)
     code.append(dict(build=label,indirect_call_lines=[line for line in dis.splitlines() if re.search(r'\bcall\w*\s+\*',line)],sections=sections,backend_visibility='separate non-LTO shared object' if shared else 'same link unit'))
     for form,mix in [(0,0),(1,0),(2,0),(0,1),(1,1),(0,2),(1,2)]:
      for n in (0,64):
       raw=c.run([exe,str(form),str(mix),str(n)]);rows=[list(map(int,line.split(','))) for line in raw.splitlines()];c.require(len(rows)==10 and [r[0] for r in rows]==list(range(10)),'warmup and9measured batches '+label,[r[0] for r in rows]);total=n+2;expected_calls=expected_offered=0
       for i in range(3000):
        kind=i%(mix+1) if mix else 0;q=[1,4,66][kind];k=(total+q-1)//q;expected_calls+=k;expected_offered+=k*total-q*k*(k-1)//2
       checksum=sum((i*17+3)%256 for i in range(n))%256
       c.require(all(r[1:4]==[form,mix,n] and r[4]>0 and r[5:9]==[expected_calls,expected_offered,3000*total,3000*checksum] for r in rows),'independent operation/output oracle '+label,dict(form=form,mix=mix,payload=n,expected_calls=expected_calls,expected_offered=expected_offered))
       times=[r[4] for r in rows[1:]];measurements.append(dict(build=label,form=form,mix=mix,payload=n,rounds=3000,warmups=1,samples_ns=times,minimum_ns=min(times),median_ns=statistics.median(times),maximum_ns=max(times),calls=expected_calls,offered=expected_offered,accepted=3000*total,representation_bytes=dict(direct=rows[0][9],erased=rows[0][10],fixed=rows[0][11],shared_table=rows[0][12]),target_allocations=0,logical_payload_to_frame_bytes=3000*n,logical_frame_to_sink_bytes=3000*total))
 c.write('dispatch_measurements.json',json.dumps(dict(rows=measurements,code=code,limits=['Elapsed full correctness-checked frame workload, not isolated dispatch latency.','One process per form/build; scheduling, order, thermal state and concurrent host load are uncontrolled.','No hardware branch/cache counter claim; indirect-call text is inspection aid, not executed branch count.','Only the selected control is initialized per frame; shared stream framing, byte verification and timer harness are common work.']),indent=2)+'\n')
 c.require(len(measurements)==224,'full build/workload matrix',len(measurements))
 for key,ordinary,changed in [('10.a','Actualsf_start/sf_pump unchangedprovider imported bydirect/table adapters withsamebackendvalidation;16O0/O2±LTO/static/shared builds retainbinaries,sections,disassembly','Eightactualsharedbackend builds haveELFNEEDED andnon-LTOsharedobject symbol/disassembly evidence;runtime loader loadsbackendatprocessstart,not copiedinline implementation'),('10.b','224build/workloadcells each1warmup+9measurements andindependentcalls/offered/outputoracle;homogeneous/two/threebackendtraces;actualpersistent/tablebytes andzero targetallocations','Payload0versus64 measuredwithsameconformance;logicalcopy/callbackcounts separatedfromelapsed wholeworkloadandunavailable hardwarecounters'),('10.c','Fixedone-byte specialization removeskind/tablefields onlyforfixedbackend contract andpassesall65framecases;decisionpacketretainspublicfailure/ownershipobligations','Thirdall-bytesbackend participatesin455conformancecases andmixedthreebackendmeasurements;privatecomparisonstateswherefixedspecialization cannotreplace runtimevariation')]:c.observe('T19.'+key,ordinary,['abstractions-dispatch']);c.observe('T19.'+key,changed,['abstractions-dispatch'],True)
