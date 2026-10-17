"""Independent Python Unicode/coordinate/wire oracle for original ADT targets."""
import argparse
import json
from pathlib import Path
import subprocess
ROOT=Path(__file__).resolve().parents[1]

def packet(raw):
    if len(raw)<3:return [1]
    if raw[0] not in (1,2):return [2]
    size=int.from_bytes(raw[1:3],'big')
    if len(raw)!=size+3:return [3]
    if raw[0]==2 and size:return [4]
    return [0,raw[0],size,3]

def scalar_count(raw):
    try:return len(raw.decode('utf-8','strict'))
    except UnicodeError:return None

def verify_fixtures():
    cases=json.loads((ROOT/'fixtures/adt/cases.json').read_text(encoding='utf-8'))['cases']
    for row in cases:
        kind=row['kind'];value=row['input'];expected=row['expected']
        if kind=='sums':actual=value['x']+value['y']
        elif kind=='quantities':actual=[s*1000 if s*1000<=2**32-1 else None for s in value['seconds']]
        elif kind=='flags':actual=[0<=n<8 for n in value['raw']]
        elif kind=='text':actual=[scalar_count(bytes.fromhex(s)) for s in value['hex']]
        elif kind=='brands':actual={'generation':value['initial_generation']+1,'old_valid':False}
        elif kind=='views':
            offsets=[r*value['row_stride']+c*value['column_stride'] for r in range(value['rows']) for c in range(value['columns'])]
            actual={'offsets':offsets,'required':max(offsets,default=-1)+1}
        elif kind=='arena':actual=sum(value['literals'][i] for i in value['children'])
        elif kind=='optics':actual=[value['replace_x'],value['pair'][1]]
        elif kind=='evolution':
            raw=bytes.fromhex(value['hex']);assert len(raw)==2+raw[1]
            actual={'tag':raw[0],'payload':raw[2:].hex(),'roundtrip':raw.hex()}
        elif kind=='packet':actual=[packet(bytes.fromhex(s))[0] for s in value['hex']]
        else:raise ValueError(kind)
        assert actual==expected,(kind,actual,expected)
    return len(cases)

def differential(driver):
    requests=[];expected=[]
    sequences=[b'']+[bytes([i]) for i in range(256)]+[i.to_bytes(2,'big') for i in range(65536)]
    sequences += [bytes.fromhex(s) for s in ['e09fbf','e0a080','ed9fbf','eda080','efbfbf','f08fbfbf','f0908080','f48fbfbf','f4908080','e282','f09080']]
    for raw in sequences:
        value=scalar_count(raw);requests.append('T '+raw.hex());expected.append('0' if value is None else '1 '+str(value))
    for rows in range(4):
        for columns in range(4):
            for rs in range(5):
                for cs in range(5):
                    offsets=[r*rs+c*cs for r in range(rows) for c in range(columns)]
                    for capacity in [0,1,4,8,16]:
                        admitted=not offsets or (rs>0 and cs>0 and max(offsets)<capacity)
                        requests.append(f'V {rows} {columns} {rs} {cs} {capacity}')
                        expected.append('1'+(' '+' '.join(map(str,offsets)) if offsets else '') if admitted else '0')
    for tag in [0,1,2,3,255]:
        for declared in [0,1,2,255,256,65535]:
            for actual in range(5):
                raw=bytes([tag])+declared.to_bytes(2,'big')+bytes(range(actual))
                requests.append('P '+raw.hex());expected.append(' '.join(map(str,packet(raw))))
    result=subprocess.run([str(driver)],input='\n'.join(requests)+'\n',text=True,capture_output=True,check=True)
    observed=result.stdout.splitlines();assert len(observed)==len(expected),(len(observed),len(expected),result.stderr)
    for i,(actual,want) in enumerate(zip(observed,expected)):
        assert actual==want,(i,requests[i],actual,want)
    return len(requests)

def main():
    parser=argparse.ArgumentParser();parser.add_argument('--driver',type=Path);args=parser.parse_args()
    count=verify_fixtures();runs=differential(args.driver) if args.driver else 0
    print(json.dumps({'passed':True,'fixture_cases':count,'independent_driver_cases':runs,'scope':'Python strict UTF-8 decoder, integer coordinate enumeration and byte-wire interpretation; no lifetime, normalization, learner mastery or universal proof claim.'}))

if __name__=='__main__':main()
