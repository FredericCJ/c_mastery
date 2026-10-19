"""Replay identity and stable tri-state oracle; all subprocesses are bounded."""
import copy,ctypes,hashlib,json,os,resource,subprocess,sys
from pathlib import Path
def sha(p):return hashlib.sha256(Path(p).read_bytes()).hexdigest()
def manifest(source,binary,symbols,input_text,config,cwd,mode,seed,timeout=.15):
 return {'source':sha(source),'binary':sha(binary),'symbols':sha(symbols),'input':hashlib.sha256(input_text.encode()).hexdigest(),'config':hashlib.sha256(json.dumps(config,sort_keys=True).encode()).hexdigest(),'cwd':str(Path(cwd).resolve()),'mode':mode,'seed':seed,'timeout':timeout}
def classify(binary,text,mode='normal',seed='17',timeout=.15):
 env=dict(os.environ,DX_MODE=mode,DX_SEED=seed);results=[]
 def bounded_core():resource.setrlimit(resource.RLIMIT_CORE,(0,0))
 for _ in range(2):
  try:r=subprocess.run([str(binary),'parser',text],env=env,capture_output=True,text=True,timeout=timeout,preexec_fn=bounded_core);state='FAIL' if r.returncode==1 and 'TRANSITION:admission:seed='+seed+':' in r.stdout and 'violations=0' not in r.stdout else 'PASS' if r.returncode==0 and 'violations=0' in r.stdout else 'UNRESOLVED';results.append((state,r.returncode,r.stdout))
  except subprocess.TimeoutExpired:results.append(('UNRESOLVED','timeout',''))
 return results[0][0] if results[0]==results[1] else 'UNRESOLVED',results
def main(binary,source,symbols,library,new_source,new_binary,new_symbols):
 cwd=Path.cwd();m=manifest(source,binary,symbols,'[NAN]',{'cap':4},cwd,'normal','17');checks={}
 for name,changes in [('stale-image',{'binary':'0'*64}),('environment',{'mode':'small'}),('seed',{'seed':'18'}),('symbols',{'symbols':'1'*64}),('cwd',{'cwd':str(cwd/'other')}),('configuration',{'config':'2'*64})]:checks[name]=dict(m,**changes)!=m
 expected=manifest(new_source,new_binary,new_symbols,'[NAN]',{'cap':4,'DX_FIXED':1},cwd,'normal','17');stale=manifest(new_source,binary,symbols,'[NAN]',{'cap':4,'DX_FIXED':1},cwd,'normal','17');checks['actual-new-source-old-executable']=expected!=stale and expected['source']==stale['source'] and expected['binary']!=stale['binary'] and sha(new_source)!=sha(source)
 result={text:{'class':classify(binary,text)[0],'replays':classify(binary,text)[1]} for text in ('[NAN]','[NN]','[?]','[T]','[C]')}
 assert [result[s]['class'] for s in result]==['FAIL','PASS','UNRESOLVED','UNRESOLVED','UNRESOLVED'] and all(checks.values())
 # Grammar-aware alternative removes whole balanced children only; flat token
 # deletion can create malformed delimiters. It has a different search space.
 tree=['[N]','[A]','[NN]'];candidates=['['+''.join(tree[:i]+tree[i+1:])+']' for i in range(3)]
 assert all(s.count('[')==s.count(']') for s in candidates) and '[[N][NN]]' in candidates
 # Changed grammar: lists retain nesting and children are lists or A/N leaves.
 # The frontend serializes only validated leaves into the original C parser.
 # Deleting a whole child cannot splice away its parent delimiters.
 def render(t):return '['+''.join(render(c) if isinstance(c,list) else c for c in t)+']'
 def leaves(t):return ''.join(leaves(c) if isinstance(c,list) else c for c in t)
 def paths(t,prefix=()):
  for i,c in enumerate(t):
   yield prefix+(i,)
   if isinstance(c,list):yield from paths(c,prefix+(i,))
 def remove(t,path):
  t=copy.deepcopy(t);node=t
  for i in path[:-1]:node=node[i]
  del node[path[-1]];return t
 tree=[['N'],['A'],['N','N']];tree_trials=[];tree_cache={}
 def tree_oracle(t):
  encoded=render(t)
  if encoded not in tree_cache:
   state,replays=classify(binary,'['+leaves(t)+']');tree_cache[encoded]=state;tree_trials.append({'tree':encoded,'parser_input':'['+leaves(t)+']','state':state,'replays':replays})
  return tree_cache[encoded]
 assert tree_oracle(tree)=='FAIL'
 for _ in range(64):
  for path in paths(tree):
   candidate=remove(tree,path)
   if tree_oracle(candidate)=='FAIL':tree=candidate;break
  else:break
 else:raise AssertionError('tree budget exhausted')
 neighbors=[remove(tree,p) for p in paths(tree)]
 assert render(tree)=='[[A]]' and all(tree_oracle(t)!='FAIL' for t in neighbors)
 class Reduction(ctypes.Structure):_fields_=[('mask',ctypes.c_uint16),('trials',ctypes.c_uint),('hits',ctypes.c_uint),('exhausted',ctypes.c_uint)]
 callback=ctypes.CFUNCTYPE(ctypes.c_int,ctypes.c_uint16,ctypes.c_void_p);trials=[];chunks='NNANNNNA'
 @callback
 def oracle(mask,arg):
  candidate='['+''.join(c for i,c in enumerate(chunks) if mask&(1<<i))+']';state,replays=classify(binary,candidate);trials.append({'mask':mask,'input':candidate,'state':state,'replays':replays});return {'PASS':0,'FAIL':1,'UNRESOLVED':2}[state]
 lib=ctypes.CDLL(str(library));lib.dx_reduce.argtypes=[ctypes.c_uint,ctypes.c_uint,callback,ctypes.c_void_p,ctypes.POINTER(Reduction)];r=Reduction();assert lib.dx_reduce(8,128,oracle,None,ctypes.byref(r))==0
 selected='['+''.join(c for i,c in enumerate(chunks) if r.mask&(1<<i))+']';assert selected=='[A]' and not r.exhausted and r.trials==len(trials)
 print(json.dumps({'passed':True,'manifest':m,'expected_new_build':expected,'rejected_stale_build':stale,'identity_rejections':checks,'classes':result,'native_C_reducer':{'selected':selected,'trials':trials,'cache_hits':r.hits},'tree_candidates':candidates,'flat_unbalanced_example':'[N][A]]','tree_reducer':{'selected':render(tree),'trials':tree_trials,'one_deletion_neighbors':[render(t) for t in neighbors],'grammar':'nested A/N lists;whole-child deletion;validated leaf frontend to original C parser'},'tree_minimum_claim':'one-deletion-minimal under declared tree rewrite;not global'}))
if __name__=='__main__':main(*sys.argv[1:])
