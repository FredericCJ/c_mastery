import json
from pathlib import Path
import sys
from common import ROOT
sys.stdout.reconfigure(encoding='utf-8')
for name in sys.argv[1:]:
    path=ROOT/name
    value=json.loads(path.read_text(encoding='utf-8-sig'))
    if isinstance(value,dict): value=value.get('resources',value.get('records',value))
    if isinstance(value,list):
        for row in value:
            if isinstance(row,dict) and 'locators' in row:
                print(row['id'],row['title'],row.get('edition'),','.join(row['domains']))
                print('  LOCATORS:', ' | '.join(row['locators']))
                print('  SCOPE:', row['inspection']['scope'])
                print('  LIMITS:', row['inspection'].get('limits'))
    else: print(value)
