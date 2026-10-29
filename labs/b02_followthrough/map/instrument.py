"""Instrument exactly the three retained probe loops; no production edit."""
import re,sys
from pathlib import Path
source=Path(sys.argv[1]).read_text(encoding='utf8')
needle='for(j=0;j<7;j++,i=(i+1)%7){'
pattern=r'\s*'.join(re.escape(t) for t in re.findall(r'[A-Za-z_]\w*|\d+|[^\s]',needle))
matches=list(re.finditer(pattern,source))
assert len(matches)==3,'Retained B02 probe loops changed: inspect before instrumenting'
for match in reversed(matches):source=source[:match.end()]+'bf_probe();'+source[match.end():]
Path(sys.argv[2]).write_text('extern void bf_probe(void);\n'+source,encoding='utf8')
