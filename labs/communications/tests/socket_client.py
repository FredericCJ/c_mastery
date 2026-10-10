"""Independent raw HTTP client exercises a real Linux loopback listener."""
import socket
import subprocess
import sys


def parse(data, methods):
    results=[]
    for method in methods:
        header,sep,data=data.partition(b'\r\n\r\n')
        assert sep, (header,data)
        lines=header.split(b'\r\n');status=int(lines[0].split()[1]);fields={}
        for line in lines[1:]:
            name,value=line.split(b':',1);fields[name.lower()]=value.strip()
        size=0 if method=='HEAD' or status==100 else int(fields[b'content-length'])
        assert len(data)>=size
        results.append((status,data[:size],fields));data=data[size:]
    assert not data,data
    return results


proc=subprocess.Popen([sys.argv[1],'0'],stdout=subprocess.PIPE,stderr=subprocess.PIPE,text=True)
try:
    line=proc.stdout.readline().strip()
    assert line.startswith('READY '),line
    authority=line.split()[1];port=int(authority.rsplit(':',1)[1])
    def exchange(data, half=False, fragments=False):
        with socket.create_connection(('127.0.0.1',port),timeout=3) as s:
            s.settimeout(3)
            if fragments:
                for byte in data:s.sendall(bytes([byte]))
            else:s.sendall(data)
            if half:s.shutdown(socket.SHUT_WR)
            chunks=[]
            while True:
                block=s.recv(17)
                if not block:break
                chunks.append(block)
            return b''.join(chunks)
    host=f'Host: {authority}\r\n'.encode()
    first=b'GET /health HTTP/1.1\r\n'+host+b'\r\n'
    last=b'HEAD /health HTTP/1.1\r\n'+host+b'Connection: close\r\n\r\n'
    answers=parse(exchange(first+first+last),['GET','GET','HEAD'])
    assert [a[0] for a in answers]==[200]*3 and answers[0][1]==b'ok\n' and answers[2][1]==b''
    chunk=b'POST /echo HTTP/1.1\r\n'+host+b'Transfer-Encoding: chunked\r\nConnection: close\r\n\r\n3;x="a;b"\r\na\x00b\r\n0\r\nX-End: ok\r\n\r\n'
    assert parse(exchange(chunk,fragments=True),['POST'])[0][1]==b'a\x00b'
    assert parse(exchange(first,half=True),['GET'])[0][1]==b'ok\n'
    incomplete=b'POST /echo HTTP/1.1\r\n'+host+b'Content-Length: 4\r\n\r\nx'
    assert parse(exchange(incomplete,half=True),['POST'])[0][0]==400
    bad=b'GET /health HTTP/1.1\r\n'+host+b'Content-Length: 0\r\nContent-Length: 0\r\n\r\n'
    assert parse(exchange(bad+first),['GET'])[0][0]==400
    # Four-request lifetime: the fifth optimistic request is never dispatched.
    assert len(parse(exchange(first*5),['GET']*4))==4
    with socket.create_connection(('127.0.0.1',port),timeout=3) as s:
        s.settimeout(3)
        s.sendall(b'POST /echo HTTP/1.1\r\n'+host+b'Content-Length: 3\r\nExpect: 100-continue\r\nConnection: close\r\n\r\n')
        interim=b''
        while not interim.endswith(b'\r\n\r\n'):interim+=s.recv(1)
        assert parse(interim,['POST'])[0][0]==100
        s.sendall(b'xyz');response=b''
        while True:
            part=s.recv(100)
            if not part:break
            response+=part
        assert parse(response,['POST'])[0][1]==b'xyz'
    curl=subprocess.run(['curl','--noproxy','*','--fail','--silent','--max-time','3',f'http://{authority}/health'],capture_output=True,check=True)
    assert curl.stdout==b'ok\n'
    print('PASS real loopback socket, independent raw parser, partial reads, pipeline, half-close, 100-continue and curl')
finally:
    proc.terminate()
    try:proc.wait(timeout=3)
    except subprocess.TimeoutExpired:proc.kill();proc.wait()
    errors=proc.stderr.read()
    if errors:print(errors,file=sys.stderr)
