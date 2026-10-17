"""Independent prefix-sum fixture check, not a POSIX execution oracle."""
from pathlib import Path
import json
ROOT=Path(__file__).resolve().parents[1]
def main():
    rows=json.loads((ROOT/'fixtures/posix_lifecycle_depth/cases.json').read_text(encoding='utf-8'))['pool']
    for row in rows:
        n=row['fail_ordinal']-1 if row['fail_ordinal'] else row['requested']
        assert row['created']==n and row['sum']==n*(n+1)*(2*n+1)//6
    print(json.dumps({'passed':True,'checks':len(rows),'scope':'prefix arithmetic only'}))
if __name__=='__main__':main()
