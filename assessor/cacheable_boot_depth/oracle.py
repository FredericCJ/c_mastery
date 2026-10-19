"""Independent Cartesian set/way encoding; actual driver output, no C implementation import."""
import json
import subprocess
import sys
exe=sys.argv[1]
total=0
for sets in (32,64,128,256,512):
 r=subprocess.run([exe,str(sets)],capture_output=True,text=True,check=True)
 actual=[int(x) for x in r.stdout.splitlines()]
 expected=[way*2**30+index*32 for index in range(sets) for way in range(4)]
 assert actual==expected
 total+=len(actual)
print(json.dumps(dict(passed=True,independent_set_way_words=total,geometries=5)))
