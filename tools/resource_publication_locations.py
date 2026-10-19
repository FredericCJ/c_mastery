"""Exact bibliography-location projection; never modify inspected resource rows.

The one supported mapping replaces a private capture link in rendered teaching
with an already registered official location for the identical captured bytes.
It makes no new access, passage-inspection, redistribution or execution claim.
"""
import json
import hashlib
from pathlib import Path
from urllib.parse import urlsplit

from common import ROOT
from resource_identity_v5 import digest

POLICY = 'research/v5/resource_publication_locations.json'
RESOURCE = 'R-LCS-ST'
ROW_SHA256 = '562a328b18428e0b22a10f3743fa487d3967daed47ccb3a221d650638c4f26f2'
REGISTER = 'research/v5/expansion/legacy_constrained_supervision/resources.json'
REGISTER_SHA256 = 'a02d4671e3260ed38e36b70d6b925eb155c8ebc5ce5d9de214af0fb14207c0b8'
INSPECTION = 'research/v5/expansion/legacy_constrained_supervision/source_access.json'
INSPECTION_SHA256 = '71418c6a3242bf2b00f55d9dbb3a8fc99f9b840fc450288b6310329d42dcffe1'
CAPTURE = 'research/private/root/rm0090-rev21.pdf'
CAPTURE_SHA256 = 'e34807b876642b67a6208e6e4071694f2892b5df088ef89f49cdad5cb254165f'
CORROBORATION = 'research/v5/expansion/legacy_advanced_dma/resources.json'
CORROBORATION_SHA256 = '2690e7b108e770d599e7aa233b0fd541d12bf4046f348c372d89405879a499c8'
CORROBORATING_ID = 'R-PD-RM21'
CORROBORATING_ROW_SHA256 = '0c561ed1363be0fdea639847b835d5694ba992086f8b9ce669a78d9626e23352'


def require(condition, message):
    if not condition:
        raise ValueError('Publication location: ' + message)


def read(root, name, expected=None):
    """Read only named portable JSON metadata, never a captured document."""
    path = root / name
    require(path.resolve() == root.resolve() / name and path.is_file(),
            'missing or redirected metadata: ' + name)
    raw = path.read_bytes()
    if expected is not None:
        require(hashlib.sha256(raw).hexdigest() == expected, 'changed metadata: ' + name)
    return json.loads(raw.decode('utf-8'))


def exact_keys(value, names, label):
    require(isinstance(value, dict) and set(value) == set(names), 'unexpected fields: ' + label)


def row(data, identifier):
    require(isinstance(data, dict) and isinstance(data.get('resources'), list), 'malformed resource register')
    selected = [r for r in data['resources'] if isinstance(r, dict) and r.get('id') == identifier]
    require(len(selected) == 1, 'missing or duplicate resource: ' + identifier)
    return selected[0]


def locations(*, root=ROOT, policy=None, resource=None):
    """Validate the complete exact mapping and return publication URLs only."""
    root = Path(root)
    policy = read(root, POLICY) if policy is None else policy
    exact_keys(policy, ('schema_version', 'operation', 'mappings'), 'policy')
    require(type(policy['schema_version']) is int and policy['schema_version'] == 1
            and policy['operation'] == 'bibliography-public-location-only', 'unsupported operation')
    entries = policy['mappings']
    require(isinstance(entries, list) and len(entries) == 1, 'missing or duplicate mapping')
    entry = entries[0]
    exact_keys(entry, ('id', 'current_row_sha256', 'registration', 'inspection',
                       'corroboration', 'capture_sha256', 'public_url', 'rationale', 'limits'), 'mapping')
    require(entry['id'] == RESOURCE and entry['current_row_sha256'] == ROW_SHA256,
            'changed target identity')
    for key in ('rationale', 'limits'):
        require(isinstance(entry[key], str) and bool(entry[key].strip()), 'missing ' + key)
    require(entry['capture_sha256'] == CAPTURE_SHA256, 'changed captured identity')
    registration = entry['registration']
    exact_keys(registration, ('path', 'sha256', 'row_sha256'), 'registration')
    require(registration == {'path': REGISTER, 'sha256': REGISTER_SHA256, 'row_sha256': ROW_SHA256},
            'changed registration binding')
    original = row(read(root, REGISTER, REGISTER_SHA256), RESOURCE)
    require(digest(original) == ROW_SHA256 and original['url'] == CAPTURE,
            'changed original registration or private location')
    accepted = read(root, 'curriculum/v5/baseline.json').get('resources')
    require(isinstance(accepted, dict) and RESOURCE not in accepted, 'accepted resource cannot be overridden')
    current = row(read(root, 'references/resources.json'), RESOURCE)
    require(digest(current) == ROW_SHA256, 'changed current resource or inspection scope')
    if resource is not None:
        require(isinstance(resource, dict) and resource.get('id') == RESOURCE
                and digest(resource) == ROW_SHA256, 'changed rendered resource')

    inspection = entry['inspection']
    exact_keys(inspection, ('path', 'sha256', 'capture_path'), 'inspection')
    require(inspection == {'path': INSPECTION, 'sha256': INSPECTION_SHA256, 'capture_path': CAPTURE},
            'changed original inspection binding')
    inspected = read(root, INSPECTION, INSPECTION_SHA256)
    identities = [r for r in inspected.get('private_source_identities', [])
                  if isinstance(r, dict) and r.get('path') == CAPTURE]
    require(identities == [{'path': CAPTURE, 'sha256': CAPTURE_SHA256}], 'unequal original capture identity')

    corroboration = entry['corroboration']
    exact_keys(corroboration, ('path', 'sha256', 'id', 'row_sha256'), 'corroboration')
    require(corroboration == {'path': CORROBORATION, 'sha256': CORROBORATION_SHA256,
                             'id': CORROBORATING_ID, 'row_sha256': CORROBORATING_ROW_SHA256},
            'changed corroborating binding')
    public = row(read(root, CORROBORATION, CORROBORATION_SHA256), CORROBORATING_ID)
    require(digest(public) == CORROBORATING_ROW_SHA256, 'changed corroborating resource')
    require(public.get('inspection', {}).get('capture', {}).get('sha256') == CAPTURE_SHA256,
            'unequal corroborating capture identity')
    url = entry['public_url']
    require(isinstance(url, str), 'non-string public location')
    parsed = urlsplit(url)
    require(parsed.scheme == 'https' and parsed.netloc == 'www.st.com'
            and parsed.path.startswith('/resource/en/reference_manual/')
            and parsed.path.endswith('.pdf') and not parsed.query and not parsed.fragment
            and '\\' not in url and url == public.get('url'), 'nonofficial or uncorroborated public location')
    return {RESOURCE: url}


def publication_url(resource, *, root=ROOT):
    """Return the qualified public URL, or None for an unaffected resource."""
    if resource.get('id') != RESOURCE:
        return None
    return locations(root=root, resource=resource)[RESOURCE]
