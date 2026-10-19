"""Compare actual capture bytes, results and ownership counts with integer models."""
import argparse
import json
from pathlib import Path
import runpy
import subprocess

ROOT = Path(__file__).resolve().parents[1]


def expected(spec):
    rows = []
    for n in range(1, 9):
        values = spec['values'][:n]
        for fail in (0, 1):
            for argument in spec['arguments']:
                spill = n > 4
                rejected = bool(fail and spill)
                value = sum(values) + argument
                arithmetic_rejected = not -(1 << 31) <= value < (1 << 31)
                status = 6 if rejected else 0
                call = 3 if rejected else 2 if arithmetic_rejected else 0
                output = 91 if rejected or arithmetic_rejected else value
                encoded = '-' if rejected else b''.join(v.to_bytes(4, 'little', signed=True) for v in values).hex()
                rows.append(f'{n} {fail} {argument} {status} {call} {output} {int(spill)} {int(spill and not rejected)} {encoded} {int(spill and not rejected)} {int(not rejected)} 0')
    return rows


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--driver')
    args = parser.parse_args()
    runpy.run_path(str(ROOT / 'labs/dispatch/tests/verify_fixtures.py'))
    spec = json.loads((ROOT / 'fixtures/dispatch/captures.json').read_text(encoding='utf8'))
    rows = expected(spec)
    assert len(rows) == 48 and spec['values'][0] == -(1 << 31)
    if args.driver:
        actual = subprocess.run([args.driver, 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert actual == rows, next(((i, a, b) for i, (a, b) in enumerate(zip(actual, rows)) if a != b), ('row-count', len(actual), len(rows)))
        print('dispatch captures: 48 actual C traces match independent byte/arithmetic/ownership model')
        ext = Path(args.driver).with_name('dispatch-extensions')
        extension_spec = json.loads((ROOT / 'fixtures/dispatch/extensions.json').read_text(encoding='utf8'))
        extension_rows = []
        for cut in extension_spec['clone_failure_cuts']:
            for n in extension_spec['capture_lengths']:
                failed = cut in (1, 2) or cut == 3 and n > 16
                attempts = 1 if cut == 1 else 2
                drops = 1 if cut == 2 else 2 if cut == 3 and n > 16 else 0
                extension_rows.append(f'C {cut} {n} {6 if failed else 0} {attempts} {drops} {2 if failed else 4} {int(n > 16 and not failed)}')
        for status in (0, 1):
            for n in range(extension_spec['maximum_progress'] + 1):
                extension_rows.append(f'P {status} {n} {6 if status else 0} {n} 1 {n * (129 + n) // 2}')
        actual_extension = subprocess.run([str(ext), 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert actual_extension == extension_rows, next(((i, a, b) for i, (a, b) in enumerate(zip(actual_extension, extension_rows)) if a != b), ('extension-row-count', len(actual_extension), len(extension_rows)))
        print('dispatch extensions: 42 actual C traces match independent acquisition-prefix and partial-progress models')
        resolution_spec = json.loads((ROOT / 'fixtures/dispatch/resolution.json').read_text(encoding='utf8'))
        ancestors = [set(values) for values in resolution_spec['ancestors']]
        keys = [tuple(key) for key in resolution_spec['registration_keys']]
        resolution_rows = []
        for bits in range(16):
            registered = [(i, key) for i, key in enumerate(keys) if bits & (1 << i)]
            for left in range(4):
                for right in range(4):
                    applicable = [(i, key) for i, key in registered if key[0] in ancestors[left] and key[1] in ancestors[right]]
                    maximal = [(i, key) for i, key in applicable if not any(other != key and key[0] in ancestors[other[0]] and key[1] in ancestors[other[1]] for _, other in applicable)]
                    status = 9 if not maximal else 10 if len(maximal) > 1 else 0
                    value = 91 if status else 8 if maximal[0][0] % 2 else 12
                    resolution_rows.append(f'R {bits} {left} {right} {status} {value}')
        resolution = Path(args.driver).with_name('dispatch-resolution')
        actual_resolution = subprocess.run([str(resolution), 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert actual_resolution == resolution_rows, next(((i, a, b) for i, (a, b) in enumerate(zip(actual_resolution, resolution_rows)) if a != b), ('resolution-row-count', len(actual_resolution), len(resolution_rows)))
        print('dispatch resolution: 256 actual cached/uncached C traces match independent partial-order relation model')
        dictionary_spec = json.loads((ROOT / 'fixtures/dispatch/dictionary.json').read_text(encoding='utf8'))
        assert dictionary_spec['maximum_length'] == 5 and dictionary_spec['oracle_rows'] == 1092 and dictionary_spec['input_values'] == [-2, 0, 3]
        dictionary_rows = []
        for n in range(6):
            for code in range(3 ** n):
                script = code
                values = []
                for _ in range(n):
                    values.append([-2, 0, 3][script % 3])
                    script //= 3
                for policy in range(3):
                    value, status, events = 1, 0, []
                    for operand in values:
                        status = 6 if policy == 2 and operand < 0 else 0
                        events.extend([value, operand, status])
                        if status:
                            break
                        value = value - operand if policy == 1 else value + operand
                    dictionary_rows.append(' '.join(map(str, ['D', n, code, policy, status, 91 if status else value, len(events) // 3, *events])))
        dictionary = Path(args.driver).with_name('dispatch-dictionary')
        actual_dictionary = subprocess.run([str(dictionary), 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert actual_dictionary == dictionary_rows, 'Dictionary operand/status trace differs from the independent left-fold model'
        print('dispatch dictionary: 1092 actual operand/status traces match independent ordered-fold model')
        intrusive_spec = json.loads((ROOT / 'fixtures/dispatch/intrusive.json').read_text(encoding='utf8'))
        assert intrusive_spec['oracle_scripts'] == 4096 and intrusive_spec['oracle_rows'] == 16384
        intrusive_rows = []
        for code in range(4096):
            script, lists = code, [[], []]
            for step in range(4):
                action, script = script % 8, script // 8
                item, kind, remove = action % 2, action // 2 % 2, action // 4
                present = item in lists[kind]
                status = (0 if present else 8) if remove else (4 if present else 0)
                if status == 0:
                    if remove:
                        lists[kind].remove(item)
                    else:
                        lists[kind].append(item)
                folded = []
                for members in lists:
                    value = 0
                    for member in members:
                        value = 5 * value + member + 1
                    folded.append(value)
                intrusive_rows.append(f'I {code} {step} {status} {folded[0]} {folded[1]}')
        intrusive = Path(args.driver).with_name('dispatch-intrusive')
        actual_intrusive = subprocess.run([str(intrusive), 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert actual_intrusive == intrusive_rows, 'Dual-hook and relocated-index history differs from independent ordered-list model'
        print('dispatch intrusive: 16384 actual transitions match independent two-membership model')
        module_spec = json.loads((ROOT / 'fixtures/dispatch/modules.json').read_text(encoding='utf8'))
        module_rows = []
        for a, left in enumerate(module_spec['values']):
            for b, right in enumerate(module_spec['values']):
                for failure in module_spec['read_failure_positions']:
                    value = left + right
                    status = 6 if failure else 0 if -(1 << 31) <= value < (1 << 31) else 2
                    calls = 1 if failure == 1 else 2
                    module_rows.append(f'M {a} {b} {failure} {status} {91 if status else value} {calls} {left} {right if calls == 2 else 0}')
        assert len(module_rows) == module_spec['oracle_rows_per_implementation'] == 75
        for name in ['dispatch-module-original', 'dispatch-module-alternative']:
            module = Path(args.driver).with_name(name)
            actual_module = subprocess.run([str(module), 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
            assert actual_module == module_rows, name + ' differs from independent callback-effect and bounded-sum model'
        print('dispatch modules: 150 actual traces from two private implementations match independent service model')
        broker_spec = json.loads((ROOT / 'fixtures/dispatch/broker.json').read_text(encoding='utf8'))
        def broker_state():
            return {'counters': [0, 100], 'next': 1, 'grants': {int(k): dict(v, rights=set(v['rights'])) for k, v in broker_spec['initial_grants'].items()}}
        def broker_step(state, op, target, identifier, rights, epoch, amount, sequence):
            answer, result, returned = 0, 0, identifier
            asked = {bit for bit in [1, 2] if rights & bit}
            if (op != 3 and rights) or (op in [1, 3] and amount):
                answer = 1
            elif sequence != state['next'] or state['next'] > 65535:
                answer = 8
            else:
                state['next'] += 1
                grant = state['grants'].get(identifier)
                if grant is None or grant['holder'] != 0 or grant['target'] != target:
                    answer = 7
                elif epoch != grant['epoch']:
                    answer = 8
                elif op == 3:
                    if not asked <= grant['rights']:
                        answer = 7
                    elif 4 in state['grants']:
                        answer = 5
                    else:
                        state['grants'][4] = {'holder': 0, 'target': target, 'rights': asked, 'epoch': epoch}
                        returned = 4
                elif (1 if op == 1 else 2) not in grant['rights']:
                    answer = 7
                elif op != 1 and state['counters'][target] + amount > 1000:
                    answer = 2
                else:
                    if op != 1:
                        state['counters'][target] += amount
                    result = state['counters'][target]
            return answer, result, returned
        broker_rows = []
        for operation in range(1, 5):
            for target in range(2):
                for identifier in range(5):
                    for rights in range(4):
                        for epoch in range(5):
                            for amount in [0, 7, 1001]:
                                state = broker_state()
                                result = broker_step(state, operation, target, identifier, rights, epoch, amount, 1)
                                broker_rows.append(' '.join(map(str, ['B', operation, target, identifier, rights, epoch, amount, *result, *state['counters'], state['next']])))
        assert len(broker_rows) == broker_spec['fresh_rows']
        for code in range(1024):
            state, script = broker_state(), code
            for step in range(5):
                action, script = script % 4, script // 4
                result = broker_step(state, 4 if action == 2 else 3 if action == 3 else 2, int(action == 2), 2 if action == 1 else 1, int(action == 3), 1, 0 if action == 3 else 7, step + 1)
                broker_rows.append(' '.join(map(str, ['H', code, step, result[0], *state['counters'], result[2]])))
        assert len(broker_rows) == broker_spec['oracle_rows'] == 7520
        broker = Path(args.driver).with_name('dispatch-broker')
        actual_broker = subprocess.run([str(broker), 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert actual_broker == broker_rows, 'Broker states differ from independent server grant/set model'
        print('dispatch broker: 7520 actual states match independent authority/set/sequence model')
        plugin_spec = json.loads((ROOT / 'fixtures/dispatch/plugin_compat.json').read_text(encoding='utf8'))
        directory = Path(args.driver).parent
        libraries = [str(directory / f'libdispatch-version-{v}.so') for v in range(10)]
        for index, client in enumerate(plugin_spec['clients'], 1):
            expected_plugin = [f"P {client['known']} {v} {status} {client['features'][v]}" for v, status in enumerate(client['statuses'])]
            result = subprocess.run([str(directory / f'dispatch-plugin-client{index}'), 'plugin_compat', *libraries], check=True, text=True, capture_output=True)
            assert result.stdout.splitlines()[:-1] == expected_plugin, 'Actual plugin admission differs from independent feature-set/extent matrix'
        print('dispatch plugin:20 actual cross-version admission/feature states match independent fixture matrix')
        async_spec = json.loads((ROOT / 'fixtures/dispatch/async.json').read_text(encoding='utf8'))
        expected_async = []
        for n in async_spec['counts']:
            for failure in range(7):
                status = 6 if failure == 5 else 1 if failure == 6 else 2 if failure == 2 and n else 6 if failure == 1 and n > 4 else 6 if failure in [3, 4] else 0
                success = status == 0
                first = 91 if not n else (600 if failure == 2 and n == 1 else 1) * (2 if success else 1)
                expected_async.append(f"R {n} {failure} {status} {1 + int(success)} {0 if success else 2} {int(n > 4) * (1 + int(success))} {2 + (2 if success else 1 if failure == 4 else 0)} {2 if success else 1 if failure == 4 else 0} {first}")
        actual_async = subprocess.run([str(directory / 'dispatch-async'), 'oracle', str(directory / 'libdispatch-async-plugin1.so'), str(directory / 'libdispatch-async-plugin2.so'), str(directory / 'libdispatch-bad.so')], check=True, text=True, capture_output=True).stdout.splitlines()
        assert actual_async == expected_async and len(actual_async) == async_spec['oracle_rows'], 'Reload ownership/failure-prefix states differ from independent transaction model'
        print('dispatch asynchronous reload:63 actual allocation/migration/registration/loader prefixes match independent ownership model')
        law_spec = json.loads((ROOT / 'fixtures/dispatch/laws.json').read_text(encoding='utf8'))
        law_rows = []
        for n in range(17):
            for cut in range(3):
                for wrong in range(2):
                    status = 6 if cut else 1 if wrong and n else 0
                    checksum = 16 * 99 if status else sum(i if i % 3 else 0 for i in range(n)) + (16 - n) * 99
                    law_rows.append(f'L {n} {cut} {wrong} {status} {91 if status else n} 1 {int(cut != 1)} {checksum}')
        law_rows += [f'V {value} {failure} {6 if failure else 0} {91 if failure else value} 1' for value in range(-8, 9) for failure in range(2)]
        law_driver = str(directory / 'dispatch-laws')
        law_actual = subprocess.run([law_driver, 'oracle_laws'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert law_actual == law_rows and len(law_actual) == law_spec['copy_law_rows'] + law_spec['measure_rows']
        closure_rows = []
        for mode in range(3):
            for initial in law_spec['closure_values']:
                for word in range(law_spec['closure_words']):
                    script, value, active = word, initial, True
                    for step in range(law_spec['closure_word_length']):
                        argument, script = script % 9 - 4, script // 9
                        status, out = (0 if active else 3), 91
                        if active:
                            out = value + argument
                            if mode == 1: value = out
                            if mode == 2: active = False
                        closure_rows.append(f'H {mode} {initial} {word} {step} {status} {out} {value} {int(active)}')
        closure_actual = subprocess.run([law_driver, 'oracle_closures'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert closure_actual == closure_rows and len(closure_rows) == law_spec['closure_rows']
        deferred_actual = subprocess.run([str(directory / 'dispatch-deferred'), 'oracle'], check=True, text=True, capture_output=True).stdout.splitlines()
        assert deferred_actual == [f'D {choice} {int(choice > 1)} 1 1 2' for choice in range(5)]
        print('dispatch law/closure transfer:102 cross-method plus34 direct-call traces,59049 actual closure transitions and5 real retained-environment schedules match independent models')



    else:
        print('dispatch capture fixture: 48 model rows generated; target comparison requires --driver')
