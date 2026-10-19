"""Selected ELF64 little-endian x86 image inspection; does not load or execute code."""
import json,pathlib,re,struct,subprocess,sys
def inspect(path,relocatable=False):
    data=path.read_bytes();h=struct.unpack_from('<16sHHIQQQIHHHHHH',data)
    if h[0][:6]!=b'\x7fELF\x02\x01' or h[2]!=62:raise ValueError('selected ELF64 x86-64 format')
    sections=[struct.unpack_from('<IIQQQQIIQQ',data,h[6]+i*h[11]) for i in range(h[12])];names=sections[h[13]];strings=data[names[4]:names[4]+names[5]];named=[]
    for s in sections:
        n=strings[s[0]:].split(b'\0',1)[0].decode();named.append(dict(name=n,type=s[1],flags=s[2],address=s[3],offset=s[4],size=s[5]))
    undef=subprocess.check_output(['nm','-u',str(path)],text=True);defined=subprocess.check_output(['nm','--defined-only',str(path)],text=True)
    result=dict(kind=h[1],entry=h[4],file_bytes=len(data),sections=named,undefined=undef,defined=defined)
    if relocatable:
        if h[1]!=1 or h[10]:raise ValueError('expected relocatable object without load segments')
        return result
    if h[1] not in (2,3):raise ValueError('final executable type')
    segments=[struct.unpack_from('<IIQQQQQQ',data,h[5]+i*h[9]) for i in range(h[10])];loads=[s for s in segments if s[0]==1]
    if any(s[0]==3 for s in segments):raise ValueError('undeclared PT_INTERP')
    if not any(s[1]&1 and s[3]<=h[4]<s[3]+s[6] for s in loads):raise ValueError('entry outside executable PT_LOAD')
    if any(s[6]<s[5] or s[2]+s[5]>len(data) or s[1]&3==3 for s in loads):raise ValueError('invalid load extent or writable executable')
    if not any(s[6]>s[5] for s in loads):raise ValueError('missing zero-filled tail')
    if any(s[1] in (4,9) and s[5] for s in sections):raise ValueError('unhandled relocation')
    for s in sections:
        if s[1]==6:
            for at in range(s[4],s[4]+s[5],16):
                tag,value=struct.unpack_from('<qQ',data,at)
                if tag==1 or tag in (8,18,35) and value:raise ValueError('dynamic dependency or relocation')
    if undef.strip():raise ValueError('undefined provider')
    if re.search(r'\b(?:malloc|calloc|realloc|free|printf|memmove|__.*di3|__stack_chk_fail)$',defined,re.M):raise ValueError('forbidden resolved provider')
    result['loads']=[dict(flags=s[1],offset=s[2],address=s[3],file_bytes=s[5],memory_bytes=s[6]) for s in loads];return result
if __name__=='__main__':
    try:r=inspect(pathlib.Path(sys.argv[1]),len(sys.argv)>3 and sys.argv[3]=='object')
    except (ValueError,struct.error,IndexError) as e:print('CE_AUDIT:'+str(e));raise SystemExit(41)
    pathlib.Path(sys.argv[2]).write_text(json.dumps(r,indent=2)+'\n');print('CE_ELF_PASS')
