"""Explicit identity corrections for exact unaccepted prior/current records."""
import hashlib,json
from resource_inputs_v5 import local,read,sha,GENERATED

def digest(row):
    return hashlib.sha256(json.dumps({k:v for k,v in row.items() if k not in GENERATED},sort_keys=True,ensure_ascii=False,separators=(',',':')).encode()).hexdigest()

def corrections(root):
    p=root/'research/v5/resource_identity_corrections.json'
    if not p.exists():return {}
    data=read(p)
    if data.get('schema_version')!=1:raise ValueError('Unsupported resource identity correction policy')
    previous=data.get('previous_registry',{});archive=(root/previous.get('path','missing')).resolve()
    if not archive.is_relative_to(root.resolve()) or not archive.is_file() or sha(archive)!=previous.get('sha256'):raise ValueError('Changed historical resource registry')
    prior_rows=read(archive).get('resources',[])
    prior={r['id']:r for r in prior_rows}
    if len(prior)!=len(prior_rows):raise ValueError('Ambiguous historical resource registry')
    out={}
    for entry in data['corrections']:
        key=entry['id']
        if key in out:raise ValueError('Duplicate resource identity correction: '+key)
        if digest(entry['previous'])!=entry['previous_sha256'] or digest(entry['incoming'])!=entry['incoming_sha256']:
            raise ValueError('Corrupt recorded identity correction: '+key)
        if key not in prior or prior[key]!=entry['previous']:
            raise ValueError('Identity correction does not match historical registry: '+key)
        evidence=entry['evidence'];path=local(root,evidence['path'])
        if sha(path)!=evidence['sha256'] or evidence['id']!=key or key not in read(path)['records']:
            raise ValueError('Missing identity correction evidence: '+key)
        record=read(path)['records'][key]
        if 'capture' in record and sha(local(root,record['capture']))!=record.get('sha256'):
            raise ValueError('Changed identity evidence capture: '+key)
        out[key]=entry
    return out

def admitted(prior,incoming,entry):
    fields=sorted(k for k in ('title','authors','edition','year','url') if prior.get(k)!=incoming.get(k))
    return (isinstance(entry,dict) and entry.get('id')==incoming.get('id')==prior.get('id')
            and fields and fields==entry.get('identity_fields') and set(fields)<= {'authors','edition','year'}
            and prior.get('title')==incoming.get('title') and prior.get('url')==incoming.get('url')
            and digest(prior)==entry.get('previous_sha256') and digest(incoming)==entry.get('incoming_sha256'))


def add_missing_authors(root, resources, provenance):
    """Apply exact, source-credited metadata additions without editing inspections.

    Unlike an identity replacement, an addition may only supply a genuinely
    absent authors field on one unaccepted registration. Original register bytes,
    technical inspection, edition and assignability remain authoritative.
    Credit evidence is a portable author-inspection record, not a claim that its
    privately cached source payload ships in a curriculum archive.
    """
    from copy import deepcopy
    policy_path = root / 'research/v5/resource_authorship_additions.json'
    if not policy_path.is_file():
        return resources, []
    policy = read(policy_path)
    if policy.get('schema_version') != 1 or policy.get('operation') != 'add-absent-authors-only':
        raise ValueError('Unsupported resource authorship addition policy')
    entries = policy.get('additions', [])
    required = policy.get('required_ids')
    if (not isinstance(entries, list) or not isinstance(required, list) or not required
            or any(not isinstance(e, dict) for e in entries)
            or len(required) != len(set(required))
            or sorted(e.get('id', '') for e in entries) != sorted(required)):
        raise ValueError('Missing or duplicate resource authorship addition')
    accepted = read(local(root, 'curriculum/v5/baseline.json'))['resources']
    current = {r['id']: r for r in resources}
    if len(current) != len(resources):
        raise ValueError('Ambiguous authorship addition input')
    result = deepcopy(resources)
    corrected = {r['id']: r for r in result}
    applied = []
    for entry in entries:
        key = entry['id']
        if set(entry) != {'id', 'register', 'register_sha256', 'original_sha256',
                          'authors', 'incoming_sha256', 'evidence'}:
            raise ValueError('Unexpected authorship addition field: ' + key)
        if key in accepted or key not in current or 'authors' in current[key]:
            raise ValueError('Authorship addition changes accepted or existing identity: ' + key)
        register = local(root, entry['register'])
        if sha(register) != entry['register_sha256']:
            raise ValueError('Changed authorship source register: ' + key)
        records = read(register)
        records = records if isinstance(records, list) else records.get('resources', [])
        original = [r for r in records if r.get('id') == key]
        providers = provenance.get(key, [])
        if (len(original) != 1 or 'authors' in original[0]
                or digest(original[0]) != entry['original_sha256']
                or digest(current[key]) != entry['original_sha256']
                or providers != [{'path': entry['register'], 'sha256': entry['register_sha256'],
                                  'role': 'registration'}]):
            raise ValueError('Wrong original authorship row or producer: ' + key)
        authors = entry['authors']
        if (not isinstance(authors, list) or not authors
                or any(not isinstance(x, str) or not x.strip() for x in authors)
                or len(authors) != len(set(authors))):
            raise ValueError('Missing or duplicate credited authors: ' + key)
        evidence = entry['evidence']
        if not isinstance(evidence, dict) or set(evidence) != {'path', 'sha256', 'record'}:
            raise ValueError('Malformed authorship evidence: ' + key)
        evidence_path = local(root, evidence['path'])
        if sha(evidence_path) != evidence['sha256']:
            raise ValueError('Changed authorship inspection: ' + key)
        credit = read(evidence_path).get('records', {}).get(evidence['record'], {})
        if (credit.get('authors') != authors or key not in credit.get('resource_ids', [])
                or credit.get('status') != 'credit-inspected'
                or any(not credit.get(f) for f in ('source', 'locator', 'observation', 'method', 'date', 'limits'))):
            raise ValueError('Uninspected or mismatched authorship credit: ' + key)
        corrected[key]['authors'] = deepcopy(authors)
        if digest(corrected[key]) != entry['incoming_sha256']:
            raise ValueError('Non-authorship change in corrected resource: ' + key)
        applied.append(deepcopy(entry))
    return result, applied
