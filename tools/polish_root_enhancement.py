"""Normalize plain-prose spacing in integrator-owned enhancement sources."""
from common import ROOT, write

REPLACEMENTS = {
    'loopback127.0.0.1':'loopback 127.0.0.1', 'Descriptor15':'Descriptor 15',
    'Releasing12':'Releasing 12', 'allows16':'allows 16', 'to12':'to 12',
    'offset46':'offset 46', 'lengths0':'lengths 0', 'through46':'through 46',
    'HTTP1.1':'HTTP/1.1', 'returns400':'returns 400', 'is400':'is 400',
    'is413':'is 413', 'is501':'is 501', 'returns404':'returns 404',
    'returns405':'returns 405', 'returns501':'returns 501',
    'maximum4294967295':'maximum 4294967295', 'the1024':'the 1024',
    'LENGTH5':'LENGTH 5', 'and100-continue':'and 100-continue',
    'hex6f6b0a':'hex `6f6b0a`', 'Content-Length3':'Content-Length: 3',
    'GET,HEAD,POST':'GET, HEAD, POST', 'Expect100-continue':'Expect: 100-continue',
    'interim100':'interim 100', 'before100':'before 100',
    'epoch1672531200':'epoch 1672531200', 'offset0':'offset 0',
    'results4,EINTR,2,EAGAIN,3':'results 4, EINTR, 2, EAGAIN, 3',
    'are4,4,6,6,9':'are 4, 4, 6, 6, 9', 'requests1,2':'requests 1, 2',
    'parsing3':'parsing 3', 'request2':'request 2', ',3 is':', 3 is',
    'the1000ms':'the 1000 ms', 'at8192':'at 8192', 'octets,32':'octets, 32',
    'target1024':'target 1024', 'body1024':'body 1024', 'extension128':'extension 128',
    'trailer1024':'trailer 1024', 'start0':'start 0',
    'progress4000,8000,12000,14900':'progress at 4000, 8000, 12000, 14900 ms',
    'now15000ms':'now 15000 ms', 'bounded408':'bounded 408', 'recv0':'recv 0',
    'RFC9112':'RFC 9112', 'RFC9110':'RFC 9110', 'RFC9931':'RFC 9931',
    'GET,HEAD,binary':'GET, HEAD, binary', 'expected100':'expected 100',
    'profileA':'profile A', 'profileC':'profile C',
    'request ID0x1234':'request ID 0x1234', 'is15':'is 15', 'schema1':'schema 1',
    'makes22':'makes 22', 'the24-octet':'the 24-octet', 'makes30':'makes 30',
    'makes36':'makes 36', 'the40-octet':'the 40-octet', 'make43':'make 43',
    'source17/destination51':'source 17/destination 51', 'hop4 to3':'hop 4 to 3',
    'numbers1/2':'numbers 1/2', 'through08':'through 08', 'through05':'through 05',
    'layer1':'layer 1', 'immediate100':'immediate 100', 'March2026':'March 2026',
}

def main():
    paths=[ROOT/'curriculum/modules/T42.md',ROOT/'curriculum/guides/communication-stacks.md']
    paths += sorted((ROOT/'curriculum/projects').glob('PR-T42-*.md'))
    # Preserve JSON contracts, IDs, literal wire fixtures and reading identities.
    for path in paths:
        text=path.read_text(encoding='utf-8')
        if path.suffix=='.md' and text.startswith('```json'):
            boundary=text.index('\n```',7)+4
            meta,body=text[:boundary],text[boundary:]
        else:
            meta,body='',text
        for old,new in REPLACEMENTS.items(): body=body.replace(old,new)
        write(path,meta+body)
    print('Polished root-owned teaching prose; contracts and literal fixtures preserved.')

if __name__=='__main__': main()
