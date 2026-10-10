"""Fetch an explicitly selected public research document into private local derivatives."""
import argparse
import hashlib
from pathlib import Path
import requests
from common import ROOT

parser=argparse.ArgumentParser(description=__doc__)
parser.add_argument('url')
parser.add_argument('--output',required=True)
args=parser.parse_args()
target=(ROOT/args.output).resolve()
if not target.is_relative_to((ROOT/'research/private').resolve()):
    raise ValueError('Research derivatives must stay private')
target.parent.mkdir(parents=True,exist_ok=True)
with requests.get(args.url,timeout=(15,60),stream=True) as response:
    response.raise_for_status()
    data=b''.join(response.iter_content(1024*1024))
if args.url.lower().split('?')[0].endswith('.pdf') and not data.startswith(b'%PDF'):
    raise ValueError('Response is not a PDF')
target.write_bytes(data)
print({'bytes':len(data),'sha256':hashlib.sha256(data).hexdigest(),'url':args.url})
