import importlib.util,json,os,sys
from pathlib import Path
root=Path(__file__).resolve().parents[3];spec=importlib.util.spec_from_file_location('retained_replay',root/'labs/debugging_followthrough/replay.py');m=importlib.util.module_from_spec(spec);spec.loader.exec_module(m)
binary=Path(sys.argv[1]);state=Path(sys.argv[2]);assert not state.exists();os.environ['SX_REPLAY_STATE']=str(state)
rows=[]
for i in range(12):
 cls,replays=m.classify(binary,'[A]',timeout=2)
 assert cls=='UNRESOLVED' and [r[0] for r in replays]==['FAIL','PASS']
 rows.append({'class':cls,'replays':replays})
assert state.read_text().strip()=='24'
print(json.dumps({'passed':True,'controlled_mixed_replay_pairs':12,'results':rows,'first_only_control':'FAIL would be wrong','scope':'Owned persistent counter scripts two outcomes, models scheduler/environment instability; not a claim of intrinsic nondeterminism or exhaustive replay.'}))
