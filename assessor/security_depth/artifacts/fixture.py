"""Declared fixture identity/compatibility; not an origin or signature oracle."""
import hashlib,json,pathlib
ROOT=pathlib.Path(__file__).resolve().parents[3]
f=json.loads((ROOT/'fixtures/embedded/PR-T35-06.json').read_text());v=f['input'];r=dict(sha256=hashlib.sha256(v['payload'].encode()).hexdigest(),compatible=[n>=v['minimum_version'] for n in v['versions']],authenticity_supported=False);assert r==f['expected'];print(json.dumps(dict(status='pass',fixture=r,origin_inferred=False)))
