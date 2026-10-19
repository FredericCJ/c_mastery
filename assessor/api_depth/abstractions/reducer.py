def run(c,root,args):
 c.group='abstractions-reducer';pub=root/'labs/api_depth/abstractions';private=root/'assessor/api_depth/abstractions';source=private/'reducer.c';common=['-std=c17','-Wall','-Wextra','-Werror','-I'+str(pub),'-I'+str(root/'labs/api_depth/contracts'),'-I'+str(root/'labs/api_depth/include'),'-I'+str(root/'labs/foundations/include')]
 for cc in ('gcc','clang'):
  for name,flags in [('debug',['-O0']),('release',['-O2']),('sanitize',['-O1','-fsanitize=address,undefined','-fno-pie','-no-pie'])]:
   exe=c.root/(cc+'-'+name+'-reducer');c.run([cc,*common,*flags,pub/'reducer_tests.c',source,'-o',exe]);c.run([exe],contains='REDUCER-PASS:cells=256;effect_histories=3;hot_backends=3;')
 for name,flag,selected in [('reducer-replay','AM_MUTANT_REPLAY',source),('reducer-retry','AM_MUTANT_RETRY_KEY',source),('reducer-learner','UNSOLVED',pub/'reducer.c')]:
  exe=c.root/name;c.run(['gcc',*common,'-O2','-D'+flag,pub/'reducer_tests.c',selected,'-o',exe]);c.run([exe],expected=1,contains='REDUCER-CHECK:')
 for cc in ('gcc','clang'):
  for optimization in ('-O0','-O2'):
   obj=c.root/(cc+optimization+'-reducer.o');c.run([cc,*common,optimization,'-c',source,'-o',obj]);c.run(['objdump','-dr',obj]);c.run(['size','-A',obj]);c.run(['nm','-u',obj])
 c.require(True,'pure dispatch relation and effect identity bounded',dict(cells=256,effect_histories=3,strategies=3,lease_bound=3,sequence_limit=7,sparse_states=[11,29,71,101]))
 for key,ordinary,changed in [('06.a','Pure state/output/effect requests replayidentically;separatebackend injectsbeforeeffectfailureandlostack thenfeedsfailureevent','Stable requestkey and bounded backenddedupe applynonidempotentincrementexactlyonceacrossretry;no crashdurability/unboundeddedupe claim'),('06.b','Switch,indexedtable andexactfunction dispatch agreewithdeclarative relation for256 valid/malformedcells;O0/O2objectcode retained','Sparse IDs11/29/71/101 map throughboundedlookup;unknown70rejectsbeforetableaccess'),('06.c','Static versusruntime decision namesvariationrequirement,ownership,instance/table/callcost andcounterexample','Threeactualbackends hotreplacewithpendingold/newleases;retiredownerreleasesonlyafteritslastlease;secondretiredgenerationcapacityrejectspreservingownership')]:
  c.observe('T19.'+key,ordinary,['abstractions-reducer']);c.observe('T19.'+key,changed,['abstractions-reducer'],True)
