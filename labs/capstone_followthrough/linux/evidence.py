"""Property-specific evidence admission; accepts learner manifests, never invents observations."""
import argparse,json,pathlib
REQUIRES={
 'parser-bounds':({'derivation','native-sanitizer'}, {'max_payload','source'}),
 'message-conservation':({'state-derivation','native-trace'}, {'admission','terminal'}),
 'race-freedom':({'synchronization-argument','thread-instrumentation'}, {'shared_fields','lifetime'}),
 'durable-output':({'flush-order','recovery-experiment'}, {'storage_model','power_limits'}),
 'throughput':({'measurement'}, {'workload','toolchain','clock','raw_samples'})}
def validate(rows):
    errors=[]
    for name,(types,premises) in REQUIRES.items():
        row=rows.get(name,{})
        if row.get('state')=='pending':
            if not row.get('missing'):errors.append(name+': pending without exact gap')
            continue
        if row.get('state')!='supported' or not types<=set(row.get('evidence_types',[])) or not premises<=set(row.get('assumptions',{})) or not row.get('artifacts'):
            errors.append(name+': insufficient property-specific support')
    return errors
def changed(rows,kind):
    affected={'runtime':['race-freedom','throughput'],'parser-source':['parser-bounds','message-conservation','throughput'],'storage':['durable-output'],'clock':['throughput']}[kind]
    return [name for name in affected if rows.get(name,{}).get('state')=='supported']
def main():
    p=argparse.ArgumentParser();p.add_argument('manifest',type=pathlib.Path);p.add_argument('--changed',choices=['runtime','parser-source','storage','clock']);a=p.parse_args();rows=json.loads(a.manifest.read_text(encoding='utf8'));errors=validate(rows)
    print(json.dumps(dict(passed=not errors,errors=errors,rerun=changed(rows,a.changed) if a.changed else [],limit='Checks declared evidence categories and premises; actual hashes, source truth, results and causal claims require assessor verification.')))
    return int(bool(errors))
if __name__=='__main__':raise SystemExit(main())
