"""Execute foreign byte requests, inspect imports and build an isolated public export."""
from datetime import datetime,timezone
import hashlib,json,re,shutil,struct,subprocess,sys,tempfile,time
from pathlib import Path
from common import ROOT,dump,sha
from verify_ownership_authority_boundary import expected
def vectors():
    rows=[]
    for tenant,rights,epoch,offset,count in [(1,1,1,0,4),(1,1,1,4,0),(1,1,1,4,1),(0,1,1,0,1),(1,3,1,0,1),(1,0,1,0,0),(1,1,2,0,1),(1,1,0,0,1),(1,1,1,4294967292,8),(1,1,4294967295,0,1),(1,2,1,0,1),(1,1,1,1,2)]:
        wire=bytes([1,tenant,rights,0])+struct.pack('>III',epoch,offset,count)
        for op in range(2):rows.append((wire,op,expected((1,tenant,1,rights,op,epoch,offset,count,2,4))))
    good=bytes([1,1,1,0])+struct.pack('>III',1,0,1);bad=[good[:n] for n in range(16)]+[good+b'X']
    for i in range(4):b=bytearray(good);b[i]=255;bad.append(bytes(b))
    invalid=','.join(map(str,[1,*range(90,106),*([-9]*8)]))
    for wire in bad:
        for op in range(2):rows.append((wire,op,invalid))
    return rows
def run():
    public='labs/ownership/authority_boundary';private='assessor/ownership/authority_boundary'
    paths=[public+'/'+p for p in ['include/authority_boundary.h','src/authority_boundary.c','tests/tests.c','tests/foreign.c','AUTHORITY.md','CMakeLists.txt','expect_failure.py']]+[private+'/authority_boundary.c','labs/ownership/include/ownership.h','labs/ownership/include/imports/v4_algo.h','labs/ownership/src/ownership.c','assessor/ownership/ownership.c','tools/common.py','tools/run_ownership_authority_boundary_investigation.py','tools/verify_ownership_authority_boundary.py'];inputs={p:sha(ROOT/p) for p in paths};external={};records=[];obs=[];artifacts=[]
    folder=ROOT/'reviews/v5/expansion/modern_completion/runs'/('ownership-authority-boundary-investigation-'+datetime.now(timezone.utc).strftime('%Y%m%dT%H%M%S%fZ'));folder.mkdir(parents=True)
    def execute(cmd,label,marker=None,data=None):
        cmd=list(map(str,cmd));start=time.monotonic();p=subprocess.run(cmd,cwd=ROOT,input=data,capture_output=True,timeout=180);body=p.stdout.decode('utf8')+'\n'+p.stderr.decode('utf8');log=folder/(str(len(records))+'-'+label+'.log');log.write_text(body,encoding='utf8');records.append(dict(command=cmd,exit_code=p.returncode,seconds=time.monotonic()-start,log=log.relative_to(ROOT).as_posix(),log_sha256=sha(log),required_marker=marker,stdin_hex=data.hex() if data is not None else None));assert p.returncode==0 and (marker is None or marker in body),(label,body);return body
    for tool in ['gcc','clang','cmake','ninja','nm','objdump']:
        p=Path(shutil.which(tool)).resolve();external[str(p)]=sha(p);execute([tool,'--version'],tool+'-version')
    p=Path(sys.executable).resolve();external[str(p)]=sha(p)
    with tempfile.TemporaryDirectory(prefix='cmastery-authority-boundary-') as name:
        tmp=Path(name);inc=['-I',ROOT/public/'include','-I',ROOT/'labs/ownership/include'];flags=['-std=c17','-Wall','-Wextra','-Wpedantic','-Werror'];sources=[ROOT/public/'tests/foreign.c',ROOT/private/'authority_boundary.c',ROOT/'assessor/ownership/ownership.c']
        for cc in ['gcc','clang']:
            deps=execute([cc,*flags,*inc,'-M',*sources],cc+'-inputs')
            for token in deps.replace('\\\n',' ').split():
                p=Path(token)
                if p.is_file():
                    try:rel=p.resolve().relative_to(ROOT.resolve()).as_posix();inputs.setdefault(rel,sha(p))
                    except ValueError:external[str(p.resolve())]=sha(p.resolve())
            obj=tmp/(cc+'.o');execute([cc,*flags,'-O3',*inc,'-c',ROOT/private/'authority_boundary.c','-o',obj],cc+'-object');body=execute(['nm','-u',obj],cc+'-imports');symbols={l.split()[-1] for l in body.splitlines() if l.strip()};assert not symbols&{'malloc','calloc','realloc','free','ow_flawed_write'};execute(['objdump','-dr',obj],cc+'-disassembly');saved=folder/obj.name;shutil.copyfile(obj,saved);artifacts.append(dict(path=saved.relative_to(ROOT).as_posix(),sha256=sha(saved)));obs.append(dict(kind='object-imports',compiler=cc,symbols=sorted(symbols),direct_heap_imports=False,flawed_import=False,whole_process_claim=False))
            for mode in ['debug','release','asan']:
                opts={'debug':['-O0','-g'],'release':['-O3'],'asan':['-O1','-g','-fsanitize=address,undefined','-fno-omit-frame-pointer','-fno-pie','-no-pie']}[mode];exe=tmp/(cc+'-'+mode);execute([cc,*flags,*opts,*inc,*sources,'-o',exe],cc+'-'+mode+'-build');trace=[]
                for i,(wire,op,want) in enumerate(vectors()):
                    text=execute([exe,'write' if op else 'read'],cc+'-'+mode+'-foreign-'+str(i),data=wire);assert text.strip()==want,(i,wire,op,text,want);trace.append(wire.hex()+':'+str(op)+':'+text.strip())
                obs.append(dict(kind='foreign-byte-boundary',profile=cc+'-'+mode,language='c17',requests=len(trace),trace_sha256=hashlib.sha256(('\n'.join(trace)+'\n').encode()).hexdigest(),trusted_parent='tenant1 READ interval2..6 selected in C server',python_serialized=True,executed=True));body=execute(['ldd',exe],cc+'-'+mode+'-runtime')
                for token in re.findall(r'(?:=>\s+|^\s*)(/[^\s]+)',body,re.MULTILINE):
                    p=Path(token)
                    if p.is_file():external[str(p.resolve())]=sha(p.resolve())
        exported=tmp/'export';shutil.copytree(ROOT/public,exported/'authority_boundary');shutil.copytree(ROOT/'labs/ownership/include',exported/'include');(exported/'src').mkdir();shutil.copyfile(ROOT/'labs/ownership/src/ownership.c',exported/'src/ownership.c');build=tmp/'build';execute(['cmake','-S',exported/'authority_boundary','-B',build,'-G','Ninja','-DCMASTERY_ASSESSOR_ROOT='+str(ROOT/'assessor/ownership')],'export-configure');execute(['cmake','--build',build,'--target','ownership-authority_boundary','--parallel','4'],'export-build');clean=records[-1]['seconds'];execute([build/'ownership-authority_boundary','authority_rights'],'export-rights','OAF_RIGHTS');execute([build/'ownership-authority_boundary','authority_wire'],'export-wire','OAF_WIRE');obs.append(dict(kind='exported-composition',private_provider_copied=False,actual_source='assessor/ownership/ownership.c',executed=True));execute(['cmake','--build',build,'--target','ownership-authority_boundary','--parallel','4'],'unchanged-build','ninja: no work to do.');obs.append(dict(kind='incremental',clean_seconds=clean,unchanged_seconds=records[-1]['seconds'],unchanged_no_work=True))
    stable=all(sha(ROOT/p)==h for p,h in inputs.items());ext=all(sha(Path(p))==h for p,h in external.items());r=dict(schema_version=1,passed=stable and ext,status='passed' if stable and ext else 'failed',source_stable=stable,external_inputs_stable=ext,input_sha256=inputs,external_input_sha256=external,executions=records,observations=obs,generated_artifacts=artifacts,physical_validation='not-run',independent_review='not-run',learner_mastery='not-assessed',limits=['Local byte stream, not authenticated IPC or arbitrary same-process capability security.','Six foreign executables import the actual original ownership producer.','No target execution, physical evidence or independent learner assessment.']);dump(ROOT/'research/v5/modern/ownership_authority_boundary_investigation_validation.json',r);print(json.dumps(dict(passed=r['passed'],commands=len(records),observations=len(obs))));return r['passed']
if __name__=='__main__':raise SystemExit(0 if run() else 1)
