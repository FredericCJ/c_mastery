"""Independent whole-buffer zlib oracle for the composed original telemetry target."""
import itertools,json,pathlib,random,struct,subprocess,sys,zlib
def frame(seq,payload):
    body=struct.pack('<BHH',1,seq,len(payload))+payload
    return b'\xa5'+body+struct.pack('<I',zlib.crc32(body))
def oracle(data,authorized):
    out=[];i=0
    while i<len(data):
        if data[i]!=0xa5:i+=1;continue
        if len(data)-i<2:break
        if data[i+1]!=1:i+=1;continue
        if len(data)-i<6:break
        size=int.from_bytes(data[i+4:i+6],'little')
        if size>32:i+=1;continue
        if len(data)-i<10+size:break
        end=i+10+size
        if zlib.crc32(data[i+1:end-4])!=int.from_bytes(data[end-4:end],'little'):i+=1;continue
        if authorized:out.append(str(int.from_bytes(data[i+2:i+4],'little'))+':'+data[i+6:end-4].hex())
        i=end
    return '\n'.join(out)+ ('\n' if out else '')
def main():
    executable=pathlib.Path(sys.argv[1]).resolve();r=random.Random(737);cases=[b'',frame(258,b'\x00A\xff'),frame(0,b''),frame(65535,bytes(range(32)))]
    nested=bytearray(frame(1,frame(7,b'\xa5')+b'Z'*21));nested[-1]^=1;cases += [bytes(nested),bytes(nested)+frame(9,b'last')]
    for i in range(80):
        raw=bytearray(r.randbytes(r.randrange(0,80)));raw += frame(i,r.randbytes(r.randrange(33)))
        if i%3==0:raw[-1]^=1
        if i%4==0:raw+=frame(300+i,b'\xa5\x00')
        cases.append(bytes(raw))
    observations=[]
    for index,data in enumerate(cases):
        for chunk,authorized in itertools.product([1,7,42],[0,1]):
            p=subprocess.run([str(executable),str(chunk),str(authorized)],input=data,stdout=subprocess.PIPE,stderr=subprocess.PIPE,timeout=5)
            expected=oracle(data,authorized);assert p.returncode==0 and p.stdout.decode()==expected,(index,chunk,authorized,p.returncode,p.stdout,expected,p.stderr)
            observations.append(dict(case=index,chunk=chunk,authorized=authorized,input=data.hex(),output=p.stdout.decode()))
    pathlib.Path(sys.argv[2]).write_text(json.dumps(dict(passed=True,cases=len(cases),observations=observations),indent=2)+'\n')
    print('CAP_FUZZ cases='+str(len(cases))+' observations='+str(len(observations))+' oracle=zlib-whole-buffer')
if __name__=='__main__':main()
