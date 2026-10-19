"""Instrument exactly the three retained probe loops; no production edit."""
import sys
from pathlib import Path
source=Path(sys.argv[1]).read_text(encoding='utf8')
needle='for(j=0;j<7;j++,i=(i+1)%7){'
assert source.count(needle)==3,'Retained B02 probe loops changed: inspect before instrumenting'
Path(sys.argv[2]).write_text('extern void bf_probe(void);\n'+source.replace(needle,needle+'bf_probe();'),encoding='utf8')
