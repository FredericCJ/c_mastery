"""Independent, bounded host models for checkpoint composition fixtures.

These are arithmetic/state/wire oracles, not implementations of the checkpoint
projects. They do not execute C, threads, syscalls, interrupts, DMA or SIMD.
"""
from pathlib import Path
from fractions import Fraction
import argparse
import json


def crc32(data):
    value = 0xffffffff
    for byte in data:
        value ^= byte
        for _ in range(8):
            value = (value >> 1) ^ (0xedb88320 if value & 1 else 0)
    return value ^ 0xffffffff


def frames(data):
    """Whole-buffer contract oracle; recovery resumes after rejected sync."""
    result = []
    index = 0
    while index < len(data):
        if data[index] != 0xa5:
            index += 1
            continue
        if len(data) - index < 6:
            break
        version = data[index + 1]
        length = int.from_bytes(data[index + 4:index + 6], 'little')
        if version != 1 or length > 32:
            index += 1
            continue
        size = 10 + length
        if len(data) - index < size:
            break
        end = index + size
        expected = int.from_bytes(data[end - 4:end], 'little')
        if crc32(data[index + 1:end - 4]) != expected:
            index += 1
            continue
        result.append({'offset': index,
                       'sequence': int.from_bytes(data[index + 2:index + 4], 'little'),
                       'payload_hex': data[index + 6:end - 4].hex()})
        index = end
    return result


def compute(kind, data):
    if kind == 'arena':
        offset, starts, ends = 0, [], []
        for size in data['requests']:
            alignment = data['alignment']
            start = ((offset + data['header'] + alignment - 1) // alignment) * alignment
            end = start + size
            if end > data['capacity']:
                starts.append(None)
                ends.append(offset)
            else:
                starts.append(start)
                offset = end
                ends.append(offset)
        return {'starts': starts, 'committed_offsets': ends, 'final_offset': offset}
    if kind == 'active-transfer':
        queue = list(data['queue'])
        active, sink, trace = None, [], []
        for outcome in data['outcomes']:
            if active is None and queue:
                active = queue.pop(0)
            if active is not None and outcome == 'accept':
                sink.append(active)
                active = None
            elif outcome not in ('retry', 'error'):
                raise ValueError('unknown transfer outcome')
            trace.append({'queue': list(queue), 'active': active, 'sink': list(sink)})
        return {'trace': trace, 'sink': sink}
    if kind == 'wire-recovery':
        wire = bytes.fromhex(data['wire_hex'])
        valid = bytes.fromhex(data['valid_hex'])
        damaged = bytearray(valid)
        damaged[6] ^= 1
        repaired = bytearray(damaged)
        repaired[-4:] = crc32(repaired[1:-4]).to_bytes(4, 'little')
        return {'crc_check': f'{crc32(b"123456789"):08x}',
                'recovered': frames(wire),
                'stale_crc_rejected': frames(damaged) == [],
                'recomputed_crc_accepts_changed_payload': frames(repaired)[0]['payload_hex'],
                'valid_partition_count': len(valid) + 1}
    if kind == 'dma-quiescence':
        queue = list(data['queue'])
        retained, pending, published, quiescent = True, False, False, False
        trace = []
        for event in data['events']:
            action = event['action']
            if action == 'complete' and event['generation'] == data['generation']:
                quiescent = True
                pending = not published
            elif action == 'take':
                queue.pop(0)
            elif action == 'publish' and pending and len(queue) < data['capacity']:
                queue.append(data['generation'])
                pending, published, retained = False, True, False
            trace.append({'queue': list(queue), 'quiescent': quiescent,
                          'pending': pending, 'published': published, 'retained': retained})
        return {'trace': trace, 'publications': int(published)}
    if kind == 'cancel-ownership':
        queue = list(data['queue'])
        pending, complete, releases, discarded = False, False, 0, 0
        trace = []
        for event in data['events']:
            if event['action'] == 'cancel':
                pending = not complete
            elif event['action'] == 'complete' and event['generation'] == data['generation'] and not complete:
                complete, pending = True, False
                releases += 1
                discarded += len(queue)
                queue.clear()
            trace.append({'pending': pending, 'complete': complete,
                          'queued': len(queue), 'releases': releases, 'discarded': discarded})
        return {'trace': trace, 'releases': releases, 'discarded': discarded}
    if kind == 'fir-chunks':
        coefficients = [Fraction(x, data['denominator']) for x in data['coefficients']]
        history = [0] * (len(coefficients) - 1)
        output, offset, checkpoints = [], 0, []
        for count in data['chunks']:
            for value in data['samples'][offset:offset + count]:
                window = [value] + history
                output.append(sum(c * v for c, v in zip(coefficients, window)))
                history = window[:-1]
            offset += count
            checkpoints.append(list(history))
        assert offset == len(data['samples'])
        return {'outputs': [str(x) for x in output], 'history_after_chunks': checkpoints,
                'final_history': history,
                'exact_cancellation': str(Fraction(2**53) + 1 - Fraction(2**53)),
                'vector_partition': [[n // data['width'], n % data['width']]
                                     for n in range(2 * data['width'] + 2)]}
    raise ValueError('unknown model ' + kind)


def observations(directory):
    results = []
    for path in sorted(directory.glob('P*.json')):
        value = json.loads(path.read_text(encoding='utf-8'))
        actual = compute(value['oracle'], value['input'])
        results.append({'fixture': path.name, 'model': value['oracle'],
                        'passed': actual == value['expected'], 'observed': actual})
    assert len(results) == 6, 'six checkpoint model fixtures are required'
    return results


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--fixtures', type=Path,
                        default=Path(__file__).resolve().parents[1] / 'fixtures/legacy_checkpoints')
    args = parser.parse_args()
    checks = observations(args.fixtures)
    print(json.dumps({'status': 'passed' if all(x['passed'] for x in checks) else 'failed',
                      'checks': checks,
                      'limits': 'Bounded host oracles only; no learner, target or timing execution.'}))
    return not all(x['passed'] for x in checks)


if __name__ == '__main__':
    raise SystemExit(main())
