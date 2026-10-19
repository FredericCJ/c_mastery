import importlib.util,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[3]
spec=importlib.util.spec_from_file_location('evidence',ROOT/'labs/capstone_followthrough/linux/evidence.py');e=importlib.util.module_from_spec(spec);spec.loader.exec_module(e)
def main():
    rows={n:dict(state='supported',evidence_types=sorted(t),assumptions={p:'explicit fixture assumption' for p in req},artifacts=['exact fixture only']) for n,(t,req) in e.REQUIRES.items()}
    assert not e.validate(rows)
    for n in rows:
        bad=json.loads(json.dumps(rows));bad[n]['evidence_types']=['checksum-page','sanitizer-pass'];assert e.validate(bad)
    pending=json.loads(json.dumps(rows));pending['durable-output']=dict(state='pending',missing=['No actual durable-output/recovery implementation in this service.']);assert not e.validate(pending)
    assert e.changed(rows,'runtime')==['race-freedom','throughput']
    assert e.changed(rows,'clock')==['throughput']
    print('CAP_EVIDENCE five-substitution-controls runtime-rerun=2 clock-rerun=1 durability-pending')
if __name__=='__main__':main()
