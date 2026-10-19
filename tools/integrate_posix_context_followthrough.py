"""Apply only pure additive T23 support; never regenerate other domains."""
import json,pathlib,sys
ROOT=pathlib.Path(__file__).resolve().parents[1];sys.path.insert(0,str(ROOT))
from research.v5.expansion.legacy_posix_context.author_support import attach
def main():
 for number in ['01','05','06']:
  path=ROOT/f'curriculum/v5/expansion/legacy_assessments/PR-T23-{number}.json';d=json.loads(path.read_text(encoding='utf8'));path.write_text(json.dumps(attach(d),indent=2)+'\n',encoding='utf8')
 print('T23 additive support attached; all prior criterion objects and pointers retained')
if __name__=='__main__':main()
