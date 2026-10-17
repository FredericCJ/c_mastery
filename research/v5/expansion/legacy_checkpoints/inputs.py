"""Author source records and hand-derived model expectations for checkpoints."""
from pathlib import Path
import hashlib
import json
import zlib

ROOT = Path(__file__).resolve().parents[4]
HERE = Path(__file__).resolve().parent


def write(path, value):
    path.parent.mkdir(parents=True, exist_ok=True)
    path.write_text(json.dumps(value, ensure_ascii=False, indent=2) + '\n', encoding='utf-8', newline='\n')


def main():
    resources = []
    for identifier, title, url, locators, domains, limits in [
        ('R-LCP-C11', 'WG14 N1570 C11 committee draft',
         'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf',
         ['6.2.4 paragraphs1–5: storage duration, lifetime and invalid pointers',
          '6.2.8 paragraphs1–7: alignment requirements',
          '6.5 paragraphs6–8: effective type, allowed lvalue access and FP contraction'],
         ['T04', 'T14', 'T16', 'T17', 'T18'],
         'C11 committee draft, not direct inspection of the published C17/C23 standards. Only listed paragraphs are claimed; local PDF text was actually read.'),
        ('R-LCP-ZLIB', 'zlib1.3.1 manual: checksum functions', 'https://www.zlib.net/manual.html',
         ['Checksum Functions: crc32 and crc32_z'], ['T15'],
         'The inspected API defines the host verifier and running checksum conventions. It is not a proof of authentication, nor permission to link zlib into the first-principles learner core.')]:
        resources.append(dict(id=identifier, title=title, authors=['Primary document owner'], url=url,
                              locators=locators, domains=domains, kind='specification' if 'C11' in identifier else 'manual',
                              edition='C11 N1570' if 'C11' in identifier else 'Displayed zlib1.3.1 manual',
                              access='Public primary passages inspected', role='oracle', assignable=True,
                              reason='Support exact object/wire/numerical contract boundaries in checkpoint composition.',
                              prerequisites=['C17 object, arithmetic and dependency-profile fundamentals'],
                              inspection=dict(date='2026-10-17', method='Actual bounded primary text read from downloaded PDF text or web page; no title-only inference.',
                                              scope='; '.join(locators), limits=limits, evidence_urls=[url])))
    shared=[]
    for register, identifiers in [
        ('research/v5/expansion/legacy_systems/resources.json', ['R-LSYS-GCC-LINK','R-LSYS-GCC-ASM','R-LSYS-WRITE','R-LSYS-ELF']),
        ('research/v5/expansion/legacy_runtime/resources.json', ['R-LRUN-THREAD','R-LRUN-COND','R-LRUN-C11-ATOMICS','R-LRUN-CM4','R-LRUN-RM21'])]:
        shared += [dict(path=register,id=identifier) for identifier in identifiers]
    write(HERE / 'resources.json', dict(schema_version=1, resources=resources, canonical_external_sources=shared,
        prior_domain_sources=dict(path='research/v5/b06_resources.json',ids=['R-B06-EPOLL','R-B06-GCCMATH'],
                                  owning_gate='research/v5/expansion/b06/research_gate.json',
                                  validator='tools.v5_common.require_domain_research for T62 and T64',
                                  scope='The domain gate pins this existing top-level register. The checkpoint batch gate covers only its local and explicitly shared expansion registers; scoped checkpoint validation separately checks the B06 gate and pins the reused register.'),reference_gaps=[
        'Exact current published C17/C23 clauses beyond the inspected draft comparison.',
        'Selected x86-64 process-entry ABI, complete CPU/OS SIMD dispatch requirements, precise F439 UART TXE/TC/DMA interaction and current board errata require separate primary inspection before those target claims.',
        'Previously inspected ELF/linker/Linux/Arm/ST passages are explicitly reused from their source registers; no whole-document reinspection is claimed.']))

    cases = []
    cases.append(('P1', 'arena', dict(capacity=32, header=3, alignment=8, requests=[5, 9, 1]),
                  dict(starts=[8, 16, None], committed_offsets=[13, 25, 25], final_offset=25)))
    cases.append(('P2', 'active-transfer', dict(queue=[65, 66], outcomes=['retry', 'accept', 'error', 'accept']),
                  dict(trace=[dict(queue=[66], active=65, sink=[]), dict(queue=[66], active=None, sink=[65]),
                              dict(queue=[], active=66, sink=[65]), dict(queue=[], active=None, sink=[65, 66])], sink=[65, 66])))
    # Fixture bytes are authored using zlib, while the separate oracle derives CRC
    # from the bit recurrence. Neither implementation is supplied as the learner C core.
    body = bytes([1, 7, 0, 0, 0])
    nested = bytes([0xa5]) + body + zlib.crc32(body).to_bytes(4, 'little')
    outer_body = bytes([1, 1, 0, 32, 0]) + nested + bytes(22)
    outer = bytes([0xa5]) + outer_body + (zlib.crc32(outer_body) ^ 1).to_bytes(4, 'little')
    valid = bytes.fromhex('a501020103000041ff62f68b76')
    assert len(outer) == 42
    cases.append(('P3', 'wire-recovery', dict(wire_hex=(outer + valid).hex(), valid_hex=valid.hex()),
                  dict(crc_check='cbf43926', recovered=[dict(offset=6, sequence=7, payload_hex=''),
                       dict(offset=42, sequence=258, payload_hex='0041ff')], stale_crc_rejected=True,
                       recomputed_crc_accepts_changed_payload='0141ff', valid_partition_count=14)))
    events = [dict(action='complete', generation=5), dict(action='complete', generation=6),
              dict(action='publish'), dict(action='take'), dict(action='publish'), dict(action='complete', generation=6)]
    states = [([9], False, False, False, True), ([9], True, True, False, True),
              ([9], True, True, False, True), ([], True, True, False, True),
              ([6], True, False, True, False), ([6], True, False, True, False)]
    cases.append(('P4', 'dma-quiescence', dict(generation=6, queue=[9], capacity=1, events=events),
                  dict(trace=[dict(queue=q, quiescent=s, pending=p, published=u, retained=r) for q,s,p,u,r in states], publications=1)))
    events = [dict(action='cancel'), dict(action='complete', generation=0), dict(action='cancel'),
              dict(action='complete', generation=1), dict(action='complete', generation=1)]
    states = [(True, False, 2, 0, 0)] * 3 + [(False, True, 0, 1, 2)] * 2
    cases.append(('P5', 'cancel-ownership', dict(generation=1, queue=[1, 2], events=events),
                  dict(trace=[dict(pending=p, complete=c, queued=q, releases=r, discarded=d) for p,c,q,r,d in states], releases=1, discarded=2)))
    cases.append(('P6', 'fir-chunks', dict(samples=[0,4,8,4,0], coefficients=[1,2,1], denominator=4, chunks=[1,2,2], width=4),
                  dict(outputs=['0','1','4','6','4'], history_after_chunks=[[0,0],[8,4],[0,4]], final_history=[0,4],
                       exact_cancellation='1', vector_partition=[[0,0],[0,1],[0,2],[0,3],[1,0],[1,1],[1,2],[1,3],[2,0],[2,1]])))
    for name, oracle, data, expected in cases:
        write(ROOT / 'fixtures/legacy_checkpoints' / (name + '.json'), dict(schema_version=1, id='FIX-LCP-'+name,
              oracle=oracle, input=data, expected=expected, scope='Bounded independent host model; not learner or target execution.',
              provenance='Concrete inputs and hand-derived state/layout/rational expectations; P3 CRC bytes authored with zlib and checked by a distinct bitwise oracle.'))


if __name__ == '__main__':
    main()
