"""Vocabulary lint: glossary validity, coverage, avoided forms, acronym expansion and orphan terms.

Errors fail the build. Findings in accepted (pre-fifth-edition) prose are a
ratchet: the count may never exceed curriculum/vocabulary/_baseline.json and
must reach zero through the reviewed terminology pass. Fifth-edition content
must have no findings at all.
"""
from __future__ import annotations
import argparse
from collections import Counter, defaultdict
from copy import deepcopy
import re
from common import ROOT, dump, load, read_json
from v5_common import RECEIPTS, manifest
import vocabulary

MINIMUM_PER_DOMAIN = 15
MINIMUM_CORE = 120
ACRONYM = re.compile(r'(?<![\w-])([A-Z]{2,6})(?![\w-])')  # letters only: T03, C23, RM0090 are identifiers, not acronyms
ACRONYM_USES = 5
STOP_ACRONYMS = {'ISO', 'IEC', 'IEEE', 'RFC', 'ANSI', 'POSIX', 'GNU', 'GCC', 'LLVM', 'ARM', 'ST', 'TI', 'CPU', 'RAM', 'ROM',
                 'USB', 'LED', 'PC', 'OS', 'IO', 'API', 'ABI', 'ELF', 'JSON', 'CSV', 'HTML', 'URL', 'PDF', 'ID', 'OK', 'TODO',
                 'MCU', 'SMP', 'AMP', 'VM', 'HTTP', 'TCP', 'UDP', 'IP', 'DNS', 'CAN', 'SPI', 'I2C', 'UART', 'USART', 'DMA',
                 'ADC', 'DAC', 'PWM', 'GPIO', 'RTOS', 'ISR', 'NVIC', 'MMIO', 'TLB', 'IPI', 'WCET', 'CRC', 'FCS', 'UB', 'ASLR',
                 'PIE', 'TLS', 'GOT', 'PLT', 'LTO', 'CI', 'MC', 'DC', 'QEMU', 'WSL', 'CMAKE', 'EDF', 'CBS', 'RCU', 'ABA',
                 'COW', 'MPU', 'FPU', 'SWD', 'JTAG', 'GDB', 'LA', 'AES', 'SHA', 'MD5', 'CPI', 'PMU', 'PSI', 'AVX', 'SSE',
                 'SIMD', 'FFT', 'FIR', 'IIR', 'SRAM', 'DTCM', 'ITCM', 'AHB', 'APB', 'RCC', 'HSEM', 'VTOR', 'IWDG', 'BSP',
                 'HAL', 'KASAN', 'EBUSY', 'EINTR', 'EOF', 'NUL', 'NULL', 'VLA', 'SPSC', 'MPSC', 'MPMC', 'FIFO', 'LIFO',
                 'LRU', 'AVL', 'BFS', 'DFS', 'RTO', 'RTT', 'MSS', 'MAC', 'PHY', 'RMII', 'MII', 'PENDSV', 'SYSTICK',
                 'PR', 'T', 'U', 'LAB', 'CAP', 'P', 'LF', 'CRLF', 'ASCII', 'UTF', 'C', 'EDU', 'CVM', 'CMK', 'CM', 'CMX'}
PROSE_ROOTS = [('modules', ROOT / 'curriculum/modules'), ('projects', ROOT / 'curriculum/projects'),
               ('guides', ROOT / 'curriculum/guides'), ('v4', ROOT / 'curriculum/v4'), ('v5', ROOT / 'curriculum/v5')]
CODE = re.compile(r'```.*?```|`[^`\n]*`', re.S)


def prose_files():
    for label, folder in PROSE_ROOTS:
        if folder.is_dir():
            for path in sorted(folder.glob('*.md')):
                yield label, path


def strip_code(text):
    return CODE.sub(' ', text)


def domain_of(path):
    match = re.match(r'(?:PR-)?(T\d{2})', path.stem)
    return match.group(1) if match else None


def is_new_content(path, new_domains):
    domain = domain_of(path)
    return (domain in new_domains) or path.parent.name == 'v5' or 'vocabulary' in path.parts


def scan_avoid(entries, new_domains, dispositions):
    findings, errors = [], []
    excluded = {(d['file'], d['form']) for d in dispositions.get('dispositions', [])}
    for entry in entries:
        for item in entry.get('avoid', []):
            pattern = re.compile(r'(?<![\w-])(?:' + item['form'] + r')(?![\w-])', re.IGNORECASE if item.get('ignore_case', True) else 0)
            when = re.compile(item['when'], re.IGNORECASE) if item.get('when') else None
            unless = re.compile(item['unless'], re.IGNORECASE) if item.get('unless') else None
            for label, path in prose_files():
                relative = path.relative_to(ROOT).as_posix()
                if (relative, item['form']) in excluded:
                    continue
                text = strip_code(path.read_text(encoding='utf-8-sig'))
                for number, line in enumerate(text.split('\n'), 1):
                    if not pattern.search(line):
                        continue
                    if when and not when.search(line):
                        continue
                    if unless and unless.search(line):
                        continue
                    finding = {'file': relative, 'line': number, 'form': item['form'], 'term': entry['term'],
                               'replace_with': item['replace_with'], 'text': line.strip()[:160]}
                    (errors if is_new_content(path, new_domains) else findings).append(finding)
    return findings, errors


def scan_acronyms(entries, new_domains):
    """Acronyms used often in a domain need a glossary entry (term, alias or symbol)."""
    known = set()
    for entry in entries:
        known.add(entry['term'].upper())
        known.update(str(a).upper() for a in entry.get('aliases', []))
        known.update(str(s).upper() for s in entry.get('symbols', []))
        for item in entry.get('expands', []):
            known.add(str(item).upper())
    counts = defaultdict(Counter)
    for label, path in prose_files():
        domain = domain_of(path)
        if not domain:
            continue
        text = strip_code(path.read_text(encoding='utf-8-sig'))
        for match in ACRONYM.finditer(text):
            counts[domain][match.group(1)] += 1
    findings, errors = [], []
    for domain, counter in sorted(counts.items()):
        for acronym, n in counter.most_common():
            if n < ACRONYM_USES or acronym in STOP_ACRONYMS or acronym in known:
                continue
            finding = {'domain': domain, 'acronym': acronym, 'uses': n}
            (errors if domain in new_domains else findings).append(finding)
    return findings, errors


def orphan_terms(entries, new_domains):
    """A term the owner's teaching text never uses is a gap in the prose: a ratcheted finding for
    accepted domains (the terminology pass introduces the term) and an error for fifth-edition domains."""
    texts = {}
    for label, path in prose_files():
        texts[path] = strip_code(path.read_text(encoding='utf-8-sig')).casefold()
    findings, errors = [], []
    for entry in entries:
        names = [entry['term']] + list(entry.get('aliases', [])) + list(entry.get('symbols', []))
        if entry.get('link') is False:
            continue
        candidates = [p for p in texts if entry['owner'] == 'core' or domain_of(p) == entry['owner']]
        base = re.sub(r'\s*\([^)]*\)$', '', entry['term'])  # "exception (Arm)" matches "exception"
        if not any(any(n.casefold() in texts[p] for n in names + [base]) for p in candidates):
            item = {'term': entry['term'], 'owner': entry['owner']}
            (errors if entry['owner'] in new_domains else findings).append(item)
    return findings, errors


def coverage(entries, taxonomy, new_domains):
    per_owner = Counter(e['owner'] for e in entries)
    findings, errors = [], []
    for domain in [d['id'] for d in taxonomy]:
        n = per_owner.get(domain, 0)
        if n < MINIMUM_PER_DOMAIN:
            item = {'domain': domain, 'terms': n, 'minimum': MINIMUM_PER_DOMAIN}
            (errors if domain in new_domains else findings).append(item)
    if per_owner.get('core', 0) < MINIMUM_CORE:
        findings.append({'domain': 'core', 'terms': per_owner.get('core', 0), 'minimum': MINIMUM_CORE})
    return dict(per_owner), findings, errors


def run(entries, resources, taxonomy, new_domains, dispositions):
    errors, pending = vocabulary.validate(entries, resources, taxonomy)
    avoid_findings, avoid_errors = scan_avoid(entries, new_domains, dispositions)
    acronym_findings, acronym_errors = scan_acronyms(entries, new_domains)
    per_owner, coverage_findings, coverage_errors = coverage(entries, taxonomy, new_domains)
    orphan_findings, orphan_errors = orphan_terms(entries, new_domains)
    errors += ['Avoided form in fifth-edition content: ' + str(f) for f in avoid_errors]
    errors += ['Unexpanded acronym in a fifth-edition domain: ' + str(f) for f in acronym_errors]
    errors += ['Fifth-edition domain below the glossary minimum: ' + str(f) for f in coverage_errors]
    errors += ['Fifth-edition term never used in its owner teaching text: ' + str(f) for f in orphan_errors]
    legacy = {'avoided_forms': len(avoid_findings), 'unexpanded_acronyms': len(acronym_findings),
              'coverage_shortfalls': len(coverage_findings), 'unused_terms': len(orphan_findings)}
    return {'errors': errors, 'pending_authorities': pending, 'legacy': legacy, 'avoid_findings': avoid_findings,
            'acronym_findings': acronym_findings, 'coverage_findings': coverage_findings,
            'unused_terms': orphan_findings, 'terms_per_owner': per_owner}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--record-baseline', action='store_true', help='Write the current legacy counts as the ratchet baseline')
    parser.add_argument('--domain', help='Report findings for one domain only (does not change the receipt)')
    args = parser.parse_args()
    taxonomy, modules, projects, resources = load()
    edition = manifest()
    entries, documents = vocabulary.load()
    dispositions_path = vocabulary.VOCAB / '_dispositions.json'
    dispositions = read_json(dispositions_path) if dispositions_path.is_file() else {'dispositions': []}
    result = run(entries, resources, taxonomy, set(edition['new_domains']), dispositions)
    baseline_path = vocabulary.VOCAB / '_baseline.json'
    if args.record_baseline:
        dump(baseline_path, {'schema_version': 5, 'legacy': result['legacy'],
                             'policy': 'Ratchet: counts may only decrease; acceptance of authored domains requires zero.'})
    baseline = read_json(baseline_path)['legacy'] if baseline_path.is_file() else None
    errors = list(result['errors'])
    if baseline is not None:
        for key, value in result['legacy'].items():
            if value > baseline.get(key, 0):
                errors.append(f'Legacy vocabulary findings increased: {key} {baseline.get(key, 0)} -> {value}')
    else:
        errors.append('No vocabulary baseline recorded; run tools/check_vocabulary.py --record-baseline once')
    # Negative controls on synthetic copies of the loaded entries.
    guards = []
    for label, change in [
        ('duplicate-term', lambda e: e.append(dict(e[0], id=e[0]['id'] + '-dup'))),
        ('dangling-authority', lambda e: e[0]['authority'].append({'resource_id': 'R-NOT-REGISTERED', 'locator': 'x'})),
        ('unknown-kind', lambda e: e[0].update(kind='folklore')),
    ]:
        sample = deepcopy(entries)
        if sample:
            change(sample)
            rejected = bool(vocabulary.validate(sample, resources, taxonomy)[0])
        else:
            rejected = False
        guards.append({'fault': label, 'rejected': rejected})
        if not rejected:
            errors.append('Guard accepted ' + label)
    if args.domain:
        selected = [f for f in result['avoid_findings'] if domain_of(ROOT / f['file']) == args.domain]
        print({'domain': args.domain, 'avoided_forms': selected[:50], 'count': len(selected),
               'terms': result['terms_per_owner'].get(args.domain, 0)})
        return
    report = {'schema_version': 5, 'status': 'passed' if not errors else 'failed', 'passed': not errors,
              'entries': len(entries), 'terms_per_owner': result['terms_per_owner'],
              'pending_authorities': len(result['pending_authorities']),
              'pending_authority_list': [{'term': t, 'resource_id': r, 'locator': l} for t, r, l in result['pending_authorities']][:200],
              'legacy': result['legacy'], 'baseline': baseline,
              'avoid_findings': result['avoid_findings'][:400], 'acronym_findings': result['acronym_findings'],
              'coverage_findings': result['coverage_findings'], 'unused_terms': result['unused_terms'],
              'negative_controls': guards, 'errors': errors,
              'scope': 'Glossary validity, coverage, avoided forms, acronym expansion and orphan terms over teaching prose; '
                       'authority locators marked pending await the fifth-edition research gate.'}
    dump(ROOT / RECEIPTS / 'vocabulary.json', report)
    print({k: v for k, v in report.items() if k not in ('avoid_findings', 'pending_authority_list', 'acronym_findings', 'coverage_findings')})
    if errors:
        raise SystemExit(1)


if __name__ == '__main__':
    main()
