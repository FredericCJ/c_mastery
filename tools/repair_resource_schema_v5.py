"""Add narrowly evidenced identity/schema fields; preserve all prior bytes.

The resource JSON files below are canonical author inputs. Their close_* tools
consume them; they do not generate these reading records. This one-time producer
records every old/new row and verifies all existing local source-capture pins.
"""
import argparse
import copy
import hashlib
import json
from pathlib import Path
import shutil

ROOT=Path(__file__).resolve().parents[1]
HERE=ROOT/'research/v5/expansion/resource_merge_repair'

def sha(p):return hashlib.sha256(p.read_bytes()).hexdigest()
def read(p):return json.loads(p.read_text(encoding='utf8'))
def put(p,v):p.parent.mkdir(parents=True,exist_ok=True);p.write_text(json.dumps(v,ensure_ascii=False,indent=2)+'\n',encoding='utf8',newline='\n')

def edition(r):
    url=r['url']; ident=r['id']
    if url.endswith('/n1570.pdf'):return 'WG14 N1570 committee draft, 12 April 2011; not published C17'
    if url.endswith('/n3096.pdf'):return 'WG14 N3096 working draft, 1 April 2023; not the published C23 standard'
    if '/gcc-15.2.0/' in url:return 'GCC 15.2.0 manual'
    if '/9799919799/' in url:return 'The Open Group Base Specifications Issue 8 / IEEE Std 1003.1-2024'
    if ident in {'R-P5-EPOLL','R-P5-CLOSE','R-FT-EXECVE-2026-01','R-TD-JOIN-2026-01'}:return 'Linux man-pages 6.19'
    exact={
      'R-P4-RM0090-21':'RM0090 revision 21, June 2024',
      'R-P4-ARMV7M-EE':'Arm DDI 0403E.e, ID021621',
      'R-P4-DS9484-14':'DS9484 revision 14, February 2026',
      'R-P1-ARMRT-2026-01':'Arm RTABI32 release 2025Q1, 7 April 2025',
      'R-FF-MPFR422-2026-01':'GNU MPFR 4.2.2 manual',
      'R-FF-INTEL-FTZ2025-2026-01':'Intel oneAPI DPC++/C++ compiler guide 2025.0',
      'R-FF-CLANG211-2026-01':'Clang 21.1.0 user manual',
      'R-FT-GIT-2026-01':'git-rebase HTML snapshot 2026-10-18; captured page states last updated in 2.54.0',
      'R-FT-PSABI-2026-01':'x86-64 psABI low-level-sys-info.tex master snapshot 2026-10-18, SHA256 2d42f2ab76b99a3b8456e6bc07e18314e85991119f72db90ca1175328e27b705; release tag unrecorded',
      'R-FI-N617-2026-01':'WG14 N617 / X3J11 96-081 proposal, 1996; historical proposal, not a published standard',
    }
    if ident not in exact:raise ValueError('Edition needs explicit inspection: '+ident)
    return exact[ident]

def main():
    ap=argparse.ArgumentParser();ap.add_argument('--apply',action='store_true');args=ap.parse_args()
    omissions=read(HERE/'schema_omissions.json');changes=[];values={};pins={}
    for omission in omissions:
        name=omission['path'];value=values.setdefault(name,read(ROOT/name));r=next(x for x in value['resources'] if x['id']==omission['record']['id'])
        assert r==omission['record'],('Input changed since audit',name,r['id'])
        old=copy.deepcopy(r);fields={}
        if 'edition' in omission['missing']:r['edition']=edition(r);fields['edition']=r['edition']
        inspection=r['inspection']
        if 'inspection.scope' in omission['missing']:
            inspection['scope']=copy.deepcopy(r['locators']);fields['inspection.scope']=inspection['scope']
        if 'inspection.limits' in omission['missing']:
            inspection['limits']='Only the named retained inspection scope and exact recorded source version. No whole-manual, published C17, physical-board, electrical or timing qualification is inferred from this source record.'
            fields['inspection.limits']=inspection['limits']
        if 'inspection.method' in omission['missing']:
            inspection['method']='Retained actual primary-passage inspection documented by the existing access, scope and local capture record; this metadata repair verifies the pinned capture identity without claiming a new or broader reading.'
            fields['inspection.method']=inspection['method']
        assert r.get('assignable')==old.get('assignable') and r['locators']==old['locators']
        for path,digest in inspection.get('source_sha256',{}).items():
            assert sha(ROOT/path)==digest,('Changed inspected capture',path);pins[path]=digest
        changes.append(dict(path=name,id=r['id'],old=old,new=r,added_fields=fields,basis='Existing inspected title/versioned URL/locator registration and verified capture; edition headers independently checked for N1570/N3096/Arm/RM0090, Git, execve, POSIX, RTABI32 and N617. Unversioned psABI is explicitly identified only by snapshot date and hash. Existing browser-only inspection remains browser-only; no new cached bytes inferred.'))
    if not args.apply:
        print(json.dumps(dict(changed_registers=len(values),changed_records=len(changes),verified_captures=len(pins))));return
    history=HERE/'history_schema_01';assert not history.exists(),'Do not overwrite historical bytes'
    manifest={}
    for name in values:
        folder=(ROOT/name).parent; review=ROOT/'reviews/v5/expansion'/folder.name
        for p in [*folder.glob('*'),*review.glob('*.json')]:
            if not p.is_file():continue
            rel=p.relative_to(ROOT).as_posix();target=history/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,target);manifest[rel]=sha(p)
    # Attachment regeneration can touch shared assessment inputs. Preserve every
    # card now, so later scoped producers retain the exact pre-repair identities.
    for p in (ROOT/'curriculum/v5/expansion/legacy_assessments').glob('*.json'):
        rel=p.relative_to(ROOT).as_posix();target=history/rel;target.parent.mkdir(parents=True,exist_ok=True);shutil.copyfile(p,target);manifest[rel]=sha(p)
    put(history/'manifest.json',dict(sha256=manifest,reason='Before additive resource identity/schema normalization; no C changes'))
    for name,value in values.items():put(ROOT/name,value)
    put(HERE/'schema_normalization.json',dict(changes=changes,verified_capture_sha256=pins,history='research/v5/expansion/resource_merge_repair/history_schema_01/manifest.json',limits=['No eligibility or locator changes.','No new full-source/standard-access or execution claim.','Package exports and envelope guards require refresh after source metadata changes.']))
    print(json.dumps(dict(changed_registers=len(values),changed_records=len(changes),verified_captures=len(pins))))

if __name__=='__main__':main()
