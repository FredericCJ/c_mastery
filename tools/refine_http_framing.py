"""Apply the reviewed request-framing distinction to original HTTP teaching data."""
from common import ROOT, read_json, dump, write

FRAMING = ('Choose NONE, exact decimal Content-Length, or a sole chunked Transfer-Encoding. '
    'Reject duplicate Content-Length, TE+CL, malformed/repeated coding tokens, chunked parameters, '
    'integer overflow and any request whose final transfer coding is not chunked with400+close. '
    'A valid bare-token coding list with an unsupported coding before one final chunked '
    '(for example gzip,chunked) receives501+close. Parameterized transfer codings are outside '
    'this declared strict syntax profile and rejected400+close. Consume bounded request content '
    'before reusing the connection; premature EOF never publishes a successful empty body.')

def main():
    path=ROOT/'research/enhancement/networking_contracts.json'
    contracts=read_json(path)
    http=next(f for f in contracts['families'] if f['domain']=='T42')
    http['framing']=FRAMING
    dump(path,contracts)
    path=ROOT/'fixtures/network_http/contract.json'
    fixture=read_json(path); fixture['input']=http; dump(path,fixture)
    path=ROOT/'fixtures/network_http/03_framing.json'
    fixture=read_json(path)
    values={case['headers'][0][1] for case in fixture['input']['cases'] if len(case['headers'])==1 and case['headers'][0][0]=='Transfer-Encoding'}
    for value in ['gzip','chunked, gzip','chunked, chunked','chunked;foo=bar','gzip,,chunked']:
        if value not in values:
            fixture['input']['cases'].append({'headers':[['Transfer-Encoding',value]]})
            fixture['expected']['outcomes'].append(['ERROR',400,True])
    dump(path,fixture)
    old='A transfer-coding other than the supported sole final chunked coding is 501+close.'
    new=('Check syntax and framing before unsupported decoding: a request whose final coding is not chunked is 400+close. '
         'Thus gzip alone and chunked,gzip are 400. Repeated chunked, malformed token lists and chunked parameters are 400. '
         'A valid gzip,chunked list is framed but requires an unsupported decoder, so it is 501+close. '
         'The declared strict profile rejects other parameterized coding lists with 400; this is a syntax-subset policy.')
    for name in ['curriculum/modules/T42.md','curriculum/projects/PR-T42-03.md']:
        path=ROOT/name; body=path.read_text(encoding='utf-8'); write(path,body.replace(old,new))
    print('Updated reviewed HTTP framing distinction; research gate refresh is required.')

if __name__=='__main__': main()
