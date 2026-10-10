import json
from pathlib import Path
import sys
ROOT = Path(__file__).resolve().parents[1]
sys.stdout.reconfigure(encoding='utf-8')
data = json.loads((ROOT / 'curriculum/taxonomy.json').read_text(encoding='utf-8'))
for domain in data['domains']:
    if len(sys.argv) > 1 and domain['id'] not in sys.argv[1:]:
        continue
    print(domain['id'], domain['title'], 'prerequisites:', ', '.join(domain.get('prerequisite_domains', [])))
    for topic in domain['subtopics']:
        print(' ', topic['id'], topic['title'], '[elective]' if topic['elective'] else '')
