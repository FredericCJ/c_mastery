"""Independent integer expectations for the exact finite-resource transfer."""
def expected():
    rows = []
    for terminal in (-1, 0, 1):
        paths = 2 ** 63
        mathematical_sum = terminal * paths
        if -(2 ** 63) <= mathematical_sum < 2 ** 63:
            rows.append(f'summary {terminal} 0 {paths} 0 {mathematical_sum}')
        else:
            rows.append(f'summary {terminal} 3 91 92 93')
        rows.append(f'full {terminal} 2 999 64 1 1')
    # The shared leaf is substituted twice. Each copy is followed by a copied
    # Choose and Emit: four allocations. Failure rolls back only used, not calls.
    rows.append('cut 0 0 7 7 4 2')
    for cut, calls in ((1, 1), (2, 2), (3, 2), (4, 2)):
        rows.append(f'cut {cut} 5 777 3 {cut} {calls}')
    rows.extend(('cut 5 5 777 3 1 1', 'cut 6 1 777 3 1 1'))
    return rows


def check(text):
    actual = text.strip().splitlines()
    if actual != expected():
        raise ValueError({'expected': expected(), 'actual': actual})
    return len(actual)
