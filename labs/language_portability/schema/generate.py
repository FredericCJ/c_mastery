"""Compiling learner scaffold. Replace admission, identity and rendering."""
import argparse
from pathlib import Path
p=argparse.ArgumentParser();p.add_argument('source');p.add_argument('output',type=Path);p.add_argument('--check',action='store_true');a=p.parse_args()
files={'config.h':'#define LS_WIRE_ID "unfinished"\n#define LS_ABI_ID "unfinished"\n#define LS_BUILD_ID "unfinished"\ntypedef struct {unsigned value;} LsApiRecord;\n','fields.h':'#define LS_TAG_MASK 0u\n#define LS_LENGTH_MASK 0u\n#define LS_FLAGS_MASK 0u\n','codec.h':'/* Implement generated pack/unpack after admission. */\n','manifest.json':'{}\n'}
files['rules.h']='/* Implement constraint graph emission. */\n'
if a.check:raise SystemExit(1)
a.output.mkdir(parents=True,exist_ok=True)
for n,s in files.items():(a.output/n).write_text(s,encoding='ascii')
