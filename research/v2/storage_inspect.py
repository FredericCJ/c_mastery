"""Read-only bounded inspection of the explicitly selected ordinary held EPUB."""
from pathlib import Path
from zipfile import ZipFile
import re, hashlib, sys
P=Path(sys.argv[1])
with ZipFile(P) as z:
    print('sha256', hashlib.sha256(P.read_bytes()).hexdigest())
    for name in z.namelist():
        if not name.endswith(('.html','.xhtml','.htm')): continue
        raw=z.read(name).decode('utf-8',errors='replace')
        text=re.sub('<[^>]+>', ' ', raw)
        text=re.sub(r'\s+', ' ', text)
        if len(sys.argv)==2:
            if any(s in text.lower() for s in ('write-ahead','redo logging','transactions','crash recovery')):
                print(name, text[:160])
        elif name==sys.argv[2]:
            term=sys.argv[3] if len(sys.argv)>3 else ''
            start=text.lower().find(term.lower()) if term else 0
            print(text[max(0,start-100):max(0,start-100)+6500])
