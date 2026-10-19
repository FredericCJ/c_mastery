"""The two construction schedules are deliberately different."""
def check(text):
    expected = ['eval 5 91 1', 'lambda 0 5 6 1', 'lambda 1 2 6 1', 'application 2 1 9']
    actual = text.strip().splitlines()
    if actual != expected:
        raise ValueError({'expected': expected, 'actual': actual})
    return len(actual)
