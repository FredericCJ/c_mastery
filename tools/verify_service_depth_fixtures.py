"""Independent unbounded-integer/list/dictionary models compare actual C calls."""
import argparse
import ctypes as C
import json
import random


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--library', required=True)
    lib = C.CDLL(parser.parse_args().library)
    u, u32, i32, size = C.c_uint, C.c_uint32, C.c_int32, C.c_size_t
    byte = C.c_ubyte
    ptr = C.POINTER

    class Memory(C.Structure):
        _fields_ = [('refs', u * 16), ('bytes', (byte * 64) * 16)]

    class Image(C.Structure):
        _fields_ = [('active', u), ('count', u), ('length', u), ('frames', u * 4), ('cow', u * 4), ('kind', u * 4)]

    class Cap(C.Structure):
        _fields_ = [('epoch', u32), ('rights', u)]

    class Message(C.Structure):
        _fields_ = [('length', u), ('bytes', byte * 32)]

    class Channel(C.Structure):
        _fields_ = [('epoch', u32), ('closed', u), ('head', u), ('count', u), ('retired', u), ('slots', Message * 2)]

    class Entry(C.Structure):
        _fields_ = [('name', C.c_char_p), ('data', C.c_void_p), ('length', u)]

    class Handle(C.Structure):
        _fields_ = [('index', u32), ('generation', u32)]

    class Open(C.Structure):
        _fields_ = [('active', u), ('entry', u), ('offset', u), ('refs', u)]

    class FileHandle(C.Structure):
        _fields_ = [('active', u), ('open', u), ('generation', u32)]

    class Files(C.Structure):
        _fields_ = [('entries', Entry * 4), ('count', u), ('descriptions', Open * 4), ('handles', FileHandle * 8)]

    class Journal(C.Structure):
        _fields_ = [('durable', (byte * 32) * 2), ('pending', (byte * 32) * 2)]

    class Receipt(C.Structure):
        _fields_ = [('last', u32), ('delta', i32), ('value', i32)]

    validator = C.CFUNCTYPE(C.c_int, C.c_void_p, C.c_void_p, size)
    decision = {'fail': 0, 'lengths': []}

    @validator
    def validate(ctx, out, n):
        decision['lengths'].append(n)
        return decision['fail']

    definitions = {
        'sd_load': [ptr(Memory), C.c_void_p, size, C.c_int, ptr(Image)],
        'sd_clone': [ptr(Memory), ptr(Image), u, ptr(Image)],
        'sd_write': [ptr(Memory), ptr(Image), u, byte, C.c_int],
        'sd_release': [ptr(Memory), ptr(Image)],
        'sd_channel_init': [ptr(Channel)], 'sd_send': [ptr(Channel), Cap, C.c_void_p, size],
        'sd_receive': [ptr(Channel), Cap, C.c_void_p, size, ptr(size), validator, C.c_void_p],
        'sd_channel_close': [ptr(Channel), Cap], 'sd_channel_reopen': [ptr(Channel)],
        'sd_files_init': [ptr(Files), ptr(Entry), u], 'sd_open': [ptr(Files), C.c_char_p, ptr(Handle)],
        'sd_dup': [ptr(Files), Handle, ptr(Handle)], 'sd_close': [ptr(Files), Handle],
        'sd_seek': [ptr(Files), Handle, u, i32],
        'sd_read': [ptr(Files), Handle, C.c_void_p, size, size, ptr(size), validator, C.c_void_p],
        'sd_apply': [ptr(Journal), u32, i32, u, ptr(i32)], 'sd_recover': [ptr(Journal), ptr(Receipt)]}
    for name, arguments in definitions.items():
        getattr(lib, name).argtypes = arguments
        getattr(lib, name).restype = C.c_int
    rng = random.Random(56002)
    cases = dict(memory=0, channel=0, files=0, journal=0)

    # Expected image is a Python byte sequence; ownership counts are derived
    # from the set of live image references, not the allocator's scan order.
    for length in [1, 63, 64, 65, 127, 128, 129, 255, 256]:
        payload = bytes(rng.randrange(256) for _ in range(length))
        storage = C.create_string_buffer(payload)
        for failure in range((length + 63) // 64):
            m, keeper, out = Memory(), Image(), Image()
            assert lib.sd_load(C.byref(m), storage, 1, -1, C.byref(keeper)) == 0
            held = bytes(m.bytes[keeper.frames[0]])
            out.length = 777
            assert lib.sd_load(C.byref(m), storage, length, failure, C.byref(out)) == 2
            assert sum(m.refs) == 1 and out.length == 777
            assert bytes(m.bytes[keeper.frames[0]]) == held
            cases['memory'] += 1
        m, a, b = Memory(), Image(), Image()
        assert lib.sd_load(C.byref(m), storage, length, -1, C.byref(a)) == 0
        assert lib.sd_clone(C.byref(m), C.byref(a), 1, C.byref(b)) == 0
        expected = [bytearray(payload), bytearray(payload)]
        for step in range(80):
            which, off, value, allow = rng.randrange(2), rng.randrange(length), rng.randrange(256), rng.randrange(2)
            image = [a, b][which]
            shared = m.refs[image.frames[off // 64]] > 1
            before = bytes(m), bytes(a), bytes(b)
            result = lib.sd_write(C.byref(m), C.byref(image), off, value, allow)
            assert result == (2 if shared and not allow else 0)
            if result:
                assert before == (bytes(m), bytes(a), bytes(b))
            else:
                expected[which][off] = value
            for index, live in enumerate([a, b]):
                actual = b''.join(bytes(m.bytes[live.frames[p]]) for p in range(live.count))
                assert actual[:length] == expected[index]
                assert actual[length:] == bytes(len(actual) - length)
            for frame in range(16):
                assert m.refs[frame] == sum(frame in list(x.frames)[:x.count] for x in [a, b])
            cases['memory'] += 1
        assert lib.sd_release(C.byref(m), C.byref(a)) == 0
        assert lib.sd_release(C.byref(m), C.byref(b)) == 0
        assert sum(m.refs) == 0 and bytes(m.bytes) == bytes(16 * 64)

    q = Channel()
    assert lib.sd_channel_init(C.byref(q)) == 0
    pending, closed, epoch = [], False, 1
    for step in range(2500):
        action = rng.randrange(4)
        if action == 0:
            data = bytes(rng.randrange(256) for _ in range(rng.randrange(34)))
            raw = C.create_string_buffer(data)
            rights = rng.choice([1, 2, 7])
            expected = 4 if not rights & 1 else 1 if len(data) > 32 else 5 if closed else 2 if len(pending) == 2 else 0
            before = bytes(q)
            assert lib.sd_send(C.byref(q), Cap(epoch, rights), raw, len(data)) == expected
            if not expected:
                pending.append(data)
            else:
                assert bytes(q) == before
        elif action == 1:
            rights = rng.choice([1, 2, 7]); capacity = rng.randrange(34)
            decision['fail'] = rng.randrange(2); decision['lengths'] = []
            out = (byte * 34)(*([99] * 34)); n = size(777)
            expected = 4 if not rights & 2 else (5 if closed else 6) if not pending else 2 if capacity < len(pending[0]) else 7 if decision['fail'] else 0
            before = bytes(q)
            assert lib.sd_receive(C.byref(q), Cap(epoch, rights), out, capacity, C.byref(n), validate, None) == expected
            assert decision['lengths'] == ([len(pending[0])] if expected in [0, 7] else [])
            if not expected:
                data = pending.pop(0)
                assert n.value == len(data) and bytes(out) == data + bytes([99]) * (34 - len(data))
            else:
                assert bytes(q) == before and n.value == 777 and bytes(out) == bytes([99]) * 34
        elif action == 2:
            assert lib.sd_channel_close(C.byref(q), Cap(epoch, 4)) == 0
            closed = True
        else:
            result = lib.sd_channel_reopen(C.byref(q))
            assert result == (0 if closed and not pending else 1)
            if not result:
                epoch += 1; closed = False
                assert lib.sd_send(C.byref(q), Cap(epoch - 1, 7), None, 0) == 3
        assert q.count == len(pending) and bool(q.closed) == closed and q.epoch == epoch
        cases['channel'] += 1

    data = bytes(range(100)); raw = C.create_string_buffer(data)
    entries = (Entry * 2)(Entry(b'alpha', C.addressof(raw), len(data)), Entry(b'empty', None, 0))
    fs = Files(); assert lib.sd_files_init(C.byref(fs), entries, 2) == 0
    active, history, descriptions = {}, [], {}
    generation = [0] * 8
    decision['fail'] = 0
    for step in range(3000):
        action = rng.randrange(6)
        if action == 0:
            which = rng.randrange(2); out = Handle(99, 99)
            free = next((i for i in range(8) if i not in active), None)
            expected = 2 if free is None or len(descriptions) == 4 else 0
            assert lib.sd_open(C.byref(fs), [b'alpha', b'empty'][which], C.byref(out)) == expected
            if not expected:
                generation[free] += 1; assert (out.index, out.generation) == (free, generation[free])
                token = ('description', step); descriptions[token] = dict(offset=0, data=[data, b''][which])
                active[free] = token; history.append((out.index, out.generation))
            else:
                assert (out.index, out.generation) == (99, 99)
        elif history:
            index, gen = rng.choice(history); h = Handle(index, gen)
            live = index in active and generation[index] == gen
            if action == 1:
                assert lib.sd_close(C.byref(fs), h) == (0 if live else 3)
                if live:
                    token = active.pop(index)
                    if token not in active.values(): del descriptions[token]
            elif action == 2:
                free = next((i for i in range(8) if i not in active), None); out = Handle(99, 99)
                expected = 3 if not live else 2 if free is None else 0
                assert lib.sd_dup(C.byref(fs), h, C.byref(out)) == expected
                if not expected:
                    generation[free] += 1; assert (out.index, out.generation) == (free, generation[free])
                    active[free] = active[index]; history.append((out.index, out.generation))
                else: assert (out.index, out.generation) == (99, 99)
            elif action == 3:
                origin, delta = rng.randrange(3), rng.randrange(-110, 111)
                if live:
                    desc = descriptions[active[index]]; pos = [0, desc['offset'], len(desc['data'])][origin] + delta
                    expected = 0 if 0 <= pos <= len(desc['data']) else 1
                else: expected = 3
                assert lib.sd_seek(C.byref(fs), h, origin, delta) == expected
                if not expected: desc['offset'] = pos
            else:
                requested = rng.randrange(110); decision['fail'] = rng.randrange(2); decision['lengths'] = []
                out = (byte * 110)(*([199] * 110)); n = size(777)
                expected = 3 if not live else 7 if decision['fail'] else 0
                assert lib.sd_read(C.byref(fs), h, out, 110, requested, C.byref(n), validate, None) == expected
                if live:
                    desc = descriptions[active[index]]
                    assert decision['lengths'] == [min(requested, len(desc['data']) - desc['offset'])]
                else:
                    assert decision['lengths'] == []
                if not expected:
                    desc = descriptions[active[index]]; part = desc['data'][desc['offset']:desc['offset'] + requested]
                    desc['offset'] += len(part)
                    assert n.value == len(part) and bytes(out) == part + bytes([199]) * (110 - len(part))
                else: assert n.value == 777 and bytes(out) == bytes([199]) * 110
        for index, token in active.items():
            actual = fs.descriptions[fs.handles[index].open]
            assert actual.offset == descriptions[token]['offset'] and actual.refs == list(active.values()).count(token)
        cases['files'] += 1

    # This model tracks committed effects/receipts, never implements the C
    # record encoding, FNV checksum, slot search, or prepare buffers.
    for scenario in range(50):
        j = Journal(); last, old_delta, value = 0, 0, 0
        for step in range(50):
            operation = rng.choice([last + 1, last + 1, max(1, last), max(1, last - 1), last + 2])
            delta = old_delta if operation == last and rng.randrange(2) else rng.randrange(-100, 101)
            fault = rng.randrange(5); out = i32(123456)
            if operation == last: expected = 0 if delta == old_delta else 9
            elif operation < last: expected = 3
            elif operation != last + 1: expected = 1
            elif not -(1 << 31) <= value + delta < 1 << 31: expected = 2
            else: expected = 8 if fault else 0
            assert lib.sd_apply(C.byref(j), operation, delta, fault, C.byref(out)) == expected
            if operation == last + 1 and expected in [0, 8] and fault in [0, 4]:
                last, old_delta, value = operation, delta, value + delta
            assert out.value == (value if expected == 0 else 123456)
            # Discard volatile state exactly as a model reset would.
            for row in j.pending:
                for i in range(32): row[i] = 0
            receipt = Receipt(999, 999, 999)
            assert lib.sd_recover(C.byref(j), C.byref(receipt)) == 0
            assert (receipt.last, receipt.delta, receipt.value) == (last, old_delta, value)
            cases['journal'] += 1
    print(json.dumps(dict(passed=True, cases=cases, total=sum(cases.values()), scope='Actual C compared with independent Python representations under the declared serialized/atomic-flush model; same-author execution, not independent learner assessment.')))


if __name__ == '__main__':
    main()
