"""Read-only exact active-row/provider matching and direct source-pin audit.

This does not reinterpret a predecessor as current, recurse into arbitrary
historical receipts, or infer execution from a recomputed digest. Scoped guards
and the separate transitive parent audits remain required.
"""
import argparse,copy,hashlib,json,os
from collections import defaultdict
from pathlib import Path
from common import ROOT,dump
from audit_current_envelopes import historical
from check_residual_followthrough import pins,local

def digest(path):
    h=hashlib.sha256()
    with path.open('rb') as f:
        for part in iter(lambda:f.read(1024*1024),b''):h.update(part)
    return h.hexdigest()

def record_hash(row):
    return hashlib.sha256(json.dumps(row,sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()

SUPERSESSIONS = {
 'research/v5/expansion/legacy_posix_context/launcher_binding_successor.json':
   ('rebind_posix_launcher_metadata','validate',{'research/v5/expansion/legacy_posix_followthrough/support.json'}),
 'research/v5/expansion/legacy_posix_binding_current/completion_dispositions.json':
   ('rebind_posix_data_metadata','check',{'research/v5/expansion/legacy_posix_'+n+'/support.json' for n in ['io','namespace','retry']}),
}
PROOF='reviews/v5/expansion/followthrough/posix_current_successor_verification.json'


def supersessions(proof=None):
    """Verify explicit byte-preserving successors with their existing guards.

    Same-row equality alone cannot establish supersession. The exact producer
    verifies the earlier bytes, current original task projection, actual source
    and execution closure, and current selected exports before any old pin is
    classified as historical.
    """
    import importlib
    if proof is None:proof=json.loads((ROOT/PROOF).read_text(encoding='utf8'))
    assert proof['passed'] is True and len(proof['checks'])==len(SUPERSESSIONS)
    assert {r['successor'] for r in proof['checks']}==set(SUPERSESSIONS)
    result={}
    for item in proof['checks']:
        current=item['successor'];module,function,old_paths=SUPERSESSIONS[current]
        assert item['guard']==module+'.'+function and not item['errors']
        assert digest(ROOT/current)==item['sha256']
        assert digest(ROOT/'tools'/ (module+'.py'))==item['guard_source_sha256']
        assert {r['path'] for r in item['originals']}==old_paths
        d=json.loads((ROOT/current).read_text(encoding='utf8'))
        assert len(d['updates'])==item['row_count']
        assert not getattr(importlib.import_module(module),function)(d)
        new_rows={r['id']:record_hash(r) for r in d['updates']}
        for old in item['originals']:
            path=old['path'];assert digest(ROOT/path)==old['sha256']
            prior=json.loads((ROOT/path).read_text(encoding='utf8'))
            assert all(new_rows.get(r['id'])==record_hash(r) for r in prior['updates'])
            result[path]=dict(successor=current,successor_sha256=item['sha256'],original_sha256=old['sha256'],
                              guard=item['guard'],guard_source_sha256=item['guard_source_sha256'],
                              verification=PROOF,verification_sha256=digest(ROOT/PROOF),
                              reason='Earlier bytes preserved; exact unchanged rows and current task/source/log/export closure verified by the explicit successor producer.')
            if module=='rebind_posix_data_metadata':
                family=Path(path).parent.name.removeprefix('legacy_posix_')
                history='research/v5/expansion/legacy_posix_binding_current/'+family+'_original_support.json'
                assert (ROOT/history).read_bytes()==(ROOT/path).read_bytes()
                result[history]=dict(result[path],reason='Exact archived predecessor bytes verified by current data successor guard; not a current execution envelope.')
    return result


def selftest():
    proof=json.loads((ROOT/PROOF).read_text(encoding='utf8'))
    edits=[lambda d:d.update(passed=False),lambda d:d['checks'].pop(),
           lambda d:d['checks'][0].update(guard='unrelated.validate'),
           lambda d:d['checks'][0].update(sha256='0'*64),
           lambda d:d['checks'][0].update(guard_source_sha256='0'*64),
           lambda d:d['checks'][0]['originals'].clear(),
           lambda d:d['checks'][0]['originals'][0].update(sha256='0'*64),
           lambda d:d['checks'][0].update(row_count=1)]
    for edit in edits:
        d=copy.deepcopy(proof);edit(d)
        try:supersessions(d)
        except (AssertionError,KeyError,ValueError):continue
        raise AssertionError('Historical support accepted without exact verified successor')
    return len(edits)


def audit():
    canonical='curriculum/v5/expansion/implementation_updates.json'
    raw=json.loads((ROOT/canonical).read_text(encoding='utf8'))
    current={r['id']:record_hash(r) for r in raw['updates']}
    matches=defaultdict(list);providers={};errors=[];cache={};stale=[];external=[];missing=[]
    if len(current) != len(raw['updates']):
        errors.append(dict(path=canonical,error='Duplicate active disposition identities'))
    superseded=supersessions();historical_bindings={};historical_pins=[];publication=[]
    for area in ['research/v5/expansion','research/v5/modern']:
        for directory,dirs,names in os.walk(ROOT/area):
            dirs[:]=[n for n in dirs if not historical(n)]
            for name in names:
                if historical(name) or not name.endswith('.json'):continue
                path=Path(directory)/name;relative=path.relative_to(ROOT).as_posix()
                try:d=json.loads(path.read_text(encoding='utf8'))
                except (ValueError,UnicodeError) as e:errors.append(dict(path=relative,error=str(e)));continue
                if not isinstance(d,dict) or not isinstance(d.get('updates'),list):continue
                selected=[r['id'] for r in d['updates'] if isinstance(r,dict) and r.get('id') in current and record_hash(r)==current[r['id']]]
                if not selected:continue
                is_history=relative in superseded
                if is_history:historical_bindings[relative]=dict(active_ids=selected,**superseded[relative])
                else:
                    providers[relative]=dict(active_ids=selected,sha256=digest(path))
                    for ident in selected:matches[ident].append(relative)
                for pointer,name,expected in pins(d):
                    normalized=local(name)
                    if normalized is None:external.append(dict(provider=relative,field=pointer,path=name,sha256=expected));continue
                    if normalized not in cache:
                        p=ROOT/normalized
                        try:cache[normalized]=digest(p) if p.is_file() else None
                        except OSError:cache[normalized]=None
                    actual=cache[normalized]
                    if actual is None:missing.append(dict(provider=relative,field=pointer,path=normalized))
                    elif actual!=expected:
                        row=dict(provider=relative,field=pointer,path=normalized,expected=expected,actual=actual)
                        if is_history: historical_pins.append(row)
                        else:
                            from check_expansion_campaigns import publication_pin
                            binding=publication_pin(normalized,expected,actual)
                            if binding is None:stale.append(row)
                            else:publication.append(dict(**row,publication_binding=binding))
    from check_active_source_projections import build as source_projections
    projection=source_projections();label='tools/check_active_source_projections.py'
    selected=[r['id'] for r in projection['updates'] if current.get(r['id'])==record_hash(r)]
    assert len(selected)==128,'Source-only active rows differ from canonical projection'
    providers[label]=dict(active_ids=selected,sha256=digest(ROOT/label),mode='read-only canonical source projection; current authority validation, no new source reading')
    for ident in selected:matches[ident].append(label)
    for name,expected in projection['input_sha256'].items():
        actual=digest(ROOT/name);cache[name]=actual
        if actual!=expected:stale.append(dict(provider=label,field='/input_sha256',path=name,expected=expected,actual=actual))
    projection.pop('updates')
    return dict(schema_version=1,scope='Exact active disposition records matched to current author envelopes, then all directly embedded repository byte pins checked. External identities and historical/transitive observations require their owning guards.',
      input=dict(path=canonical,sha256=digest(ROOT/canonical)),providers=providers,
      verified_superseded_providers=historical_bindings,historical_pin_differences=historical_pins,source_only_projection=projection,
      historical_publication_differences=publication,
      original_execution_reports_rewritten=False,
      all_repository_pins_exactly_current=not(stale or missing or publication or historical_pins),
      source_binding_scope='Exact current provider pins, explicitly preserved POSIX predecessor metadata, and closed nineteen-document publication successors are reported separately; no execution identity is rewritten.',
      unmatched_active_ids=sorted(set(current)-set(matches)),stale=stale,missing=missing,external=external,errors=errors,
      summary=dict(active_rows=len(current),matched_rows=len(matches),providers=len(providers),files=len(cache),stale=len(stale),missing=len(missing),verified_superseded=len(historical_bindings),historical_pin_differences=len(historical_pins),qualified_publication_differences=len(publication)),
      passed=not(errors or stale or missing or set(current)-set(matches)),accepted=False)

if __name__=='__main__':
    p=argparse.ArgumentParser();p.add_argument('--output',required=True);p.add_argument('--self-test',action='store_true');a=p.parse_args();out=(ROOT/a.output).resolve()
    assert out.is_relative_to((ROOT/'reviews/v5/expansion').resolve()) and not out.exists()
    d=audit();d['supersession_negative_controls']=selftest() if a.self_test else 0;dump(out,d);print(json.dumps(d['summary']))
    raise SystemExit(int(not d['passed']))
