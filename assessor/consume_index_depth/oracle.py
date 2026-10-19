"""Independent path enumeration for the explicitly supplied four-vertex DAG."""
import itertools
import json
import pathlib
import subprocess
import sys

def expected():
    pairs = list(itertools.combinations(range(4), 2))
    answers = {}
    for kinds in itertools.product(range(3), repeat=6):
        adjacency = {v: [] for v in range(4)}
        for (a,b), kind in zip(pairs,kinds):
            if kind == 1:
                adjacency[a].append(b)
        def paths(v):
            return {v}.union(*(paths(w) for w in adjacency[v]))
        code = sum(k*3**i for i,k in enumerate(kinds))
        answers[code] = sum(2**v for v in paths(0))
    return answers

def verify(rows):
    actual = {}
    for row in rows.splitlines():
        k,v = map(int,row.split())
        if k in actual: raise ValueError('duplicate graph')
        actual[k] = v
    if actual != expected(): raise ValueError('CI_ORACLE: value/control graph disagreement')
    return 729

if __name__ == '__main__':
    try:
        count = verify(subprocess.check_output([str(pathlib.Path(sys.argv[1]).resolve()),'graphs'],text=True))
        print(json.dumps({'status':'pass','graphs':count,'model':'value-edge DAG, not the C memory model'}))
    except ValueError as error:
        print(str(error),file=sys.stderr)
        raise SystemExit(41)
