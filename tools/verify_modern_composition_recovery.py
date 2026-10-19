"""Independent ownership ledger for five-resource publication failure cuts."""
from pathlib import Path
import argparse
import json
import subprocess


def expected():
    rows=[]
    for allocation in range(4):
        for opening in range(3):
            allocated = allocation == 0
            committed = allocated and opening == 0
            pending = allocated and opening != 1
            rows.append(dict(allocation=allocation, open=opening,
                committed=int(committed), pending=int(pending),
                cells=6 if pending else 3,
                resources=4 if committed else 3 if pending else 2,
                sum=16 if committed else 5,
                generation=2 if allocated else 1,
                attempts=6 if allocated else 3+allocation,
                releases=3 if allocated and opening==1 else allocation-1 if allocation else 0,
                opens=2 if not allocated else 3 if opening==1 else 4))
    return rows


def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver');args=parser.parse_args()
    fixture=Path(__file__).resolve().parents[1]/'fixtures/modern_composition/recovery.json'
    want=expected()
    if json.loads(fixture.read_text(encoding='utf-8'))['expected'] != want:
        raise ValueError('Fixture differs from independent prefix/release accounting')
    if args.driver:
        result=subprocess.run([args.driver,'oracle'],check=True,capture_output=True,text=True,timeout=20)
        got=[json.loads(line) for line in result.stdout.splitlines()]
        if got != want:raise ValueError({'observed':got,'expected':want})
    print(json.dumps({'passed':True,'ledger_cases':len(want),'c_executed':bool(args.driver),
        'limits':'Finite allocation/open/retained-close combinations. No arbitrary scheduling, durability or learner mastery claim.'}))
    return 0


if __name__=='__main__':raise SystemExit(main())
