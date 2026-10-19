"""Inspect retained POSIX nm output against a stated experiment, not compiler prose."""
import argparse,json,pathlib
def symbols(path):
    rows={}
    for line in pathlib.Path(path).read_text().splitlines():
        parts=line.split()
        if len(parts)>=2: rows.setdefault(parts[0],[]).append(parts[1])
    return rows
def main():
    p=argparse.ArgumentParser();p.add_argument('kind',choices=['static','U','T']);p.add_argument('a');p.add_argument('b');p.add_argument('external',nargs='?');a=p.parse_args()
    x,y=symbols(a.a),symbols(a.b)
    if a.kind=='static':
        assert x.get('hh_mix')==['t'] and y.get('hh_mix')==['t'],(x,y)
        z=symbols(a.external);assert z.get('hh_read')==['T'] and 'hh_read' not in x and 'hh_read' not in y,(x,y,z)
        expected='two internal definitions; one external definition'
    else:
        assert x.get('hh_experiment')==[a.kind] and y.get('hh_experiment')==[a.kind],(x,y)
        expected=a.kind
    print(json.dumps(dict(status='pass',expected=expected,a=x,b=y)))
if __name__=='__main__':main()
