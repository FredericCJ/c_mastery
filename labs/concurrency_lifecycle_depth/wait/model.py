"""Independent FIFO/credit ledger for all length-five histories over eight events.

The oracle uses a list of waiting participants and conservation totals, not CMX
task structures, wrapper tokens or C helper calls. Generation/overflow/native
memory behavior is outside this finite explorer and has separately named tests.
"""
import argparse
import itertools
import subprocess


def expected(history):
    waiting = []
    credits, live, now, posts, acquired = 0, 1, 0, 0, 0
    results, sequences, terminals, causes = [4, 4], [0, 0], [0, 0], [0, 0]
    observations = []

    def complete(task, result, cause):
        waiting.remove(task)
        results[task] = result
        causes[task] = cause
        terminals[task] += 1

    for event in history:
        if event < 2:
            task = event
            if not live or task in waiting:
                result = 4
            elif credits:
                credits -= 1
                acquired += 1
                sequences[task] += 1
                terminals[task] += 1
                results[task], causes[task], result = 0, 1, 0
            elif now:
                result = 2
            else:
                waiting.append(task)
                sequences[task] += 1
                results[task], causes[task], result = 1, 0, 1
        elif event == 2:
            if not live:
                result = 4
            elif waiting:
                complete(waiting[0], 0, 1)
                posts += 1
                acquired += 1
                result = 0
            elif credits == 1:
                result = 5
            else:
                credits += 1
                posts += 1
                result = 0
        elif event in (3, 5):
            task = (event - 3) // 2
            if task not in waiting:
                result = 11
            else:
                now = 1
                for participant in list(waiting):
                    complete(participant, 2, 2)
                result = 0
        elif event in (4, 6):
            task = (event - 4) // 2
            if task not in waiting:
                result = 11
            else:
                complete(task, 3, 3)
                result = 0
        else:
            if not live:
                result = 4
            else:
                for task in list(waiting):
                    complete(task, 3, 4)
                live = 0
                result = 0
        assert posts == credits + acquired
        state = [result, credits, live, now]
        for task in range(2):
            state += [int(task in waiting), results[task], sequences[task], terminals[task], causes[task]]
        state += [posts, acquired]
        observations.append(','.join(map(str, state)) + ';')
    return ''.join(observations)


def main(binary):
    histories = list(itertools.product(range(8), repeat=5))
    inputs = ''.join(''.join(map(str, history)) + '\n' for history in histories)
    result = subprocess.run([binary], input=inputs, capture_output=True, text=True, timeout=45)
    if result.returncode:
        raise SystemExit('CW_CHECK:trace executable failed, not a model counterexample: ' + str(result.returncode))
    actual = result.stdout.splitlines()
    if len(actual) != len(histories):
        raise SystemExit('CW_CHECK:missing trace observations')
    for history, observation in zip(histories, actual):
        wanted = expected(history)
        if observation != wanted:
            raise SystemExit(f'CW_CHECK:history={history};observed={observation};expected={wanted}')
    print('CW_MODEL:32768 histories;163840 observed transitions;two consumers;one credit;timeout/cancel/close')


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('binary')
    main(parser.parse_args().binary)
