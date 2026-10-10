"""Read the public, bounded source scopes of one enhancement group."""
import argparse
import json
from common import ROOT, read_json

parser=argparse.ArgumentParser()
parser.add_argument('group', choices=['algorithms','communications','networking'])
args=parser.parse_args()
data=read_json(ROOT/f'research/enhancement/{args.group}_resources.json')
records=data if isinstance(data,list) else data.get('resources',data.get('records',[]))
print(json.dumps([{'id':r['id'],'title':r['title'],'edition':r['edition'],'domains':r['domains'],
                  'assignable':r.get('assignable',True),'locators':r['locators'],
                  'scope':r['inspection']['scope'],'limits':r['inspection'].get('limits')}
                 for r in records],ensure_ascii=False,indent=2))
