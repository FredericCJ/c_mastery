"""Record only passages actually inspected during checkpoint implementation."""
import hashlib,json
from pathlib import Path
ROOT=Path(__file__).resolve().parents[4]
BASE=Path(__file__).resolve().parent
ROWS=[
('R-LCOMP-EPOLL','Linux epoll(7), man-pages6.19','https://man7.org/linux/man-pages/man7/epoll.7.html',
 ['Level-triggered and edge-triggered, lines56-111','Questions and answers: fd/open-file-description identity and closure, lines217-278','Possible pitfalls: ready lists and cached closed events, lines315-346'],['T24'],
 'Read opened primary manual text.','Linux readiness, partial draining and stale cached events.','Local pipe evidence only; no network service, IOCP or production throughput claim.'),
('R-LCOMP-DUP','Linux dup(2), man-pages6.19','https://man7.org/linux/man-pages/man2/dup.2.html',
 ['DESCRIPTION dup2 atomic replacement, lines51-75','NOTES close-error inspection without prior close, lines127-155'],['T23','T24'],
 'Read opened primary manual text.','Atomic fd replacement and separately owned close-error evidence.','Does not provide application-level fd lifetime ownership automatically.'),
('R-LCOMP-GLIBC-COND','glibc pthread_cond_wait.c implementation commentary','https://raw.githubusercontent.com/bminor/glibc/master/nptl/pthread_cond_wait.c',
 ['Total ordering and spurious wakeups, lines158-173','Destruction waiter references, lines224-228','Limitations and mutex errors, lines277-299'],['T25'],
 'Read actual primary implementation source commentary.','Predicate-loop rationale and implementation limitations.','Live main source inspected2026-10-17; not a normative POSIX specification or proof of every installed glibc version.'),
('R-LCOMP-ST-TX','STM32 RM0090 Rev21 UART/DMA and SRAM passages','https://www.stmcu.jp/wp/wp-content/uploads/2017/08/RM0090_Rev21.pdf',
 ['2.3.1 SRAM and CCM access, p68','10.3.3 Table43 DMA1 channel4/stream6 USART2_TX, p310','10.3.15 DMA flow and NDTR, p324','10.5.5 EN read-low/configuration rules, p334','30.3.2 transmitter and TXE/TC sequence, pp973-975','30.6.1 TXE and TC, p1011'],['T27','T28','T29'],
 'Read actual local PDF text in build/legacy_runtime_sources/rm0090.txt; PDF SHA256 e34807b876642b67a6208e6e4071694f2892b5df088ef89f49cdad5cb254165f. Remote web open failed and was not counted as inspection.',
 'Specific F439 byte-transfer, ownership and memory-placement contract.','Selected Rev21 passages only; updated errata, board wiring, clocks and physical behavior require separate evidence.'),
('R-LCOMP-ST-PINS','STM32F437/F439 datasheet DS9484 Rev14','https://www.st.com/resource/en/datasheet/stm32f439ni.pdf',
 ['Table12 alternate functions, printed75/PDFpage74: PA2 AF7 USART2_TX and PA3 AF7 USART2_RX'],['T28'],
 'Opened primary PDF and read actual Table12 text; requested PDF screenshot for the table.','Selected AF mapping for the original transmit adapter.','Not complete package/electrical/board-routing inspection. No physical pin validation.'),
('R-LCOMP-ARM-RT','Arm Run-time ABI for the Arm Architecture','https://raw.githubusercontent.com/ARM-software/abi-aa/main/rtabi32/rtabi32.rst',
 ['Memory copying, clearing, and setting: __aeabi_memclr4 signature, sizes and clobbers, lines1313-1375'],['T08','T10','T27'],
 'Read actual primary ABI source passage.','Explicit compiler-emitted byte-clear helper ABI and permitted implementation.','Only memory helpers inspected; no implicit permission for other runtime helpers.'),
('R-LCOMP-C-ATOMICS','WG14 N1570 C11 committee draft','https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',
 ['7.17.1 non-explicit operations use seq_cst, p273/PDF291','7.17.2.2 atomic_init and concurrent initialization, p274/PDF291','7.17.3 order and consistency paragraphs1-8, pp275-276/PDF292-293'],['T25','T29'],
 'Read actual opened primary draft passages.','Acquire/release publication and conservative seq_cst multiword snapshot.','C11 draft evidence for mechanisms used under the active C17 baseline; Cortex-M IRQ entry and lock-free code generation remain implementation/ABI obligations.')
]
ROWS += [
('R-LCOMP-OPENSSL-MAC','OpenSSL3 EVP_MAC and EVP_Q_mac documentation','https://docs.openssl.org/3.0/man3/EVP_MAC/',
 ['SYNOPSIS EVP_Q_mac signature, lines5440-5444','Computing functions EVP_Q_mac output extent and allocation, lines5491-5492','RETURN VALUES EVP_Q_mac null on error, line5606'],['T15','T35'],
 'Read actual opened official documentation sections, not navigation entries.','Hosted HMAC-SHA256 primitive adapter; caller-supplied32-byte output and checked return/length.','Only primitive API contract; not an audit of OpenSSL internals, RNG, key management or whole-program constant time. Actual local library3.5.5 recorded by runner.'),
('R-LCOMP-OPENSSL-COMPARE','OpenSSL3 CRYPTO_memcmp documentation','https://docs.openssl.org/3.0/man3/CRYPTO_memcmp/',
 ['DESCRIPTION, RETURN VALUES and NOTES, lines5403-5412'],['T15','T35'],
 'Read actual opened official function contract.','Fixed-length tag equality through the library contract.','Content-independent comparison time is the library contract; protocol branches and lookup are not claimed constant-time.'),
('R-LCOMP-HMAC-VECTOR','RFC4231 HMAC-SHA test vectors','https://www.rfc-editor.org/rfc/rfc4231.txt',
 ['4.1 Introduction and4.2 Test Case1, lines118-145'],['T15','T35'],
 'Read actual RFC text including20-byte0b key, Hi There input and HMAC-SHA256 expected digest.','Independent known-answer check for the supplied hosted primitive.','Known-answer agreement is bounded evidence, not a proof of primitive security or certificate of implementation correctness.')
]

def main():
    path=BASE/'resources.json';data=json.loads(path.read_text(encoding='utf-8'))
    ids={row[0] for row in ROWS};data['resources']=[r for r in data['resources'] if r['id'] not in ids]
    for ident,title,url,locators,domains,method,scope,limits in ROWS:
        data['resources'].append({'id':ident,'title':title,'url':url,'authors':['Primary document owner'],
            'locators':locators,'domains':domains,'kind':'manual','edition':'Version explicit in title; otherwise live source inspected2026-10-17',
            'access':'Public bounded passages inspected','role':'oracle','assignable':True,'reason':scope,
            'prerequisites':['C17 object, ownership and environment distinctions'],
            'inspection':{'date':'2026-10-17','method':method,'scope':scope,'limits':limits,'evidence_urls':[url]}})
    path.write_text(json.dumps(data,indent=2)+'\n',encoding='utf-8',newline='\n')
    gate={'status':'passage-inspected','domains':sorted({d for r in data['resources'] for d in r['domains']}),
          'inspection_kind':'Author inspection, not independent review',
          'research_input_sha256':{path.relative_to(ROOT).as_posix():hashlib.sha256(path.read_bytes()).hexdigest()}}
    (BASE/'research_gate.json').write_text(json.dumps(gate,indent=2)+'\n',encoding='utf-8',newline='\n')
    print(json.dumps({'resources':len(data['resources']),'domains':gate['domains']}))
if __name__=='__main__':main()
