"""Register bounded primary passages actually inspected during author research.

This records author inspection, not independent review or document-wide access.
"""
import hashlib
import json
from pathlib import Path

ROOT = Path(__file__).resolve().parents[3]

def write(path, value):
    target = ROOT / path
    target.parent.mkdir(parents=True, exist_ok=True)
    target.write_text(json.dumps(value, indent=2, ensure_ascii=False) + '\n', encoding='utf-8', newline='\n')

def resource(key, title, author, url, locators, reason, domains):
    return dict(id='R-ADMP-' + key, title=title, authors=[author], edition='Named public text inspected 2026-10-17', year=2026, kind='manual', url=url, access='Public primary passages inspected', domains=domains, role='oracle', reason=reason, prerequisites=['C17 representation, lifetime and preprocessing'], locators=locators, assignable=True, inspection=dict(date='2026-10-17', method='Opened and read the named primary web/PDF passages', scope='; '.join(locators), limits='Bounded passages only; no whole-document inspection, language implementation certification or independent review. Rolling C++ draft conditions are not silently attributed to C++20.', evidence_urls=[url]))

def main():
    rows = [
      ('ENUM', 'Rust Reference: enumerated types', 'Rust Project', 'https://doc.rust-lang.org/reference/types/enum.html', ['type.enum.intro; declaration, constructors and variant values'], 'Compare nominal sums with C tags and union discipline.', ['T67']),
      ('MATCH', 'Rust Reference: match expressions', 'Rust Project', 'https://doc.rust-lang.org/reference/expressions/match-expr.html', ['Scrutinee constraints, place/value behavior, bindings and match guards'], 'Separate match binding and guard behavior from C switch diagnostics.', ['T67']),
      ('UTF8', 'Rust str::from_utf8', 'Rust Project', 'https://doc.rust-lang.org/std/str/fn.from_utf8.html', ['Description and Errors; checked versus unchecked conversion'], 'A validating view constructor establishes encoding, not backing lifetime.', ['T67']),
      ('PHANTOM', 'Rust PhantomData', 'Rust Project', 'https://doc.rust-lang.org/std/marker/struct.PhantomData.html', ['Unused lifetime/type parameters; ownership and drop check; layout'], 'Contrast nominal C handles and runtime brands with language-level marker analysis.', ['T67']),
      ('SPAN', 'C++ draft: span', 'ISO C++ working draft', 'https://eel.is/c++draft/views.span', ['span.overview; span.cons valid-range preconditions'], 'Non-owning contiguous views and invalidation differ from ownership.', ['T67']),
      ('STRIDE', 'C++ draft: layout_stride', 'ISO C++ working draft', 'https://eel.is/c++draft/mdspan.layout.stride', ['mdspan.layout.stride.expo; constructors; required_span_size observers'], 'Derive extents, required span size and non-overlapping layout preconditions.', ['T67']),
      ('DURATION', 'C++ draft: duration conversions', 'ISO C++ working draft', 'https://eel.is/c++draft/time.duration', ['time.duration.cast conversion ratio cases and common representation'], 'Derive dimensional conversions and distinguish conversion from overflow checking.', ['T67']),
      ('LENS', 'lens maintainer source: Control.Lens.Type', 'Edward Kmett and contributors', 'https://raw.githubusercontent.com/ekmett/lens/master/src/Control/Lens/Type.hs', ['Lens laws near lines85-106; Prism review/preview laws near lines345-365'], 'Assess three lens laws and prism round trips with independent finite enumeration.', ['T67']),
      ('UNKNOWN', 'Protocol Buffers language guide: proto3', 'Protocol Buffers project', 'https://protobuf.dev/programming-guides/proto3/', ['Unknown Fields; Retaining Unknown Fields'], 'Unknown binary-field retention can be lost by JSON or field-wise reconstruction.', ['T67']),
      ('RFC3629', 'RFC3629: UTF-8', 'F. Yergeau / IETF', 'https://www.rfc-editor.org/rfc/rfc3629', ['Section3 UTF-8 definition; section4 ABNF byte grammar'], 'Reject overlong encodings, surrogate values and scalars above U+10FFFF.', ['T67']),
      ('C11', 'WG14 N1570 C11 committee draft', 'ISO/IEC JTC1/SC22/WG14', 'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n1570.pdf', ['6.5.1.1 paragraphs2-4 generic selection', '6.7.2.1 paragraphs14-17 structure/union layout', '6.7.2.3 paragraphs4-10 tags and distinct types', '6.10.3 paragraphs11-12 and6.10.3.1 macro argument substitution'], 'Pin C generic selection, tagged representations and macro substitution; published C17 access remains separate.', ['T67','T68']),
      ('GENERICS', 'Rust Reference: generic parameters', 'Rust Project', 'https://doc.rust-lang.org/reference/items/generics.html', ['Const generics introduction, allowed types and standalone use restrictions'], 'Contrast const parameters with macro-instantiated storage and C constraints.', ['T68']),
      ('ASSOCIATED', 'Rust Reference: associated items', 'Rust Project', 'https://doc.rust-lang.org/reference/items/associated-items.html', ['Associated types declaration, definition and Container example'], 'Associated type aliases and implementation obligations exceed token substitution.', ['T68']),
      ('MACRO', 'Rust Reference: macros by example', 'Rust Project', 'https://doc.rust-lang.org/reference/macros-by-example.html', ['Transcribing; metavariables; repetitions; mixed-site hygiene'], 'A bounded C token expander can demonstrate binding identity without claiming Rust macro equivalence.', ['T68']),
      ('FORMAT', 'C++ draft: format string checking', 'ISO C++ working draft', 'https://eel.is/c++draft/format.fmt.string', ['basic_format_string consteval constructor and compatibility remarks; runtime format path'], 'Typed C sink arguments are not arbitrary compile-time format-string evaluation.', ['T68']),
      ('VAOPT', 'WG14 N3033: comma omission and deletion', 'WG14 proposal authors', 'https://www.open-std.org/jtc1/sc22/wg14/www/docs/n3033.htm', ['Summary; Problem; Discussion of __VA_OPT__ proposal'], 'Distinguish proposed C23 variable-argument changes from portable C17 and GNU comma deletion.', ['T68']),
      ('HASH', 'Rust Hash trait', 'Rust Project', 'https://doc.rust-lang.org/std/hash/trait.Hash.html', ['Implementing Hash; Hash and Eq; Prefix collisions; Portability'], 'Equality implies equal hash; standard hashes are not stable portable wire identities.', ['T68']),
      ('NINJA', 'Ninja build manual', 'Ninja project', 'https://ninja-build.org/manual.html', ['Comparison to Make command-line dependency; The Ninja log; Build dependencies explicit, implicit and order-only'], 'Distinguish ordering from invalidation and declare hidden inputs to build actions.', ['T68']),
    ]
    records = [resource(*row) for row in rows]
    for domain, family in [('T67','adt'), ('T68','metaprog')]:
        rel = f'research/v5/modern/{family}_resources.json'
        write(rel, dict(schema_version=1, resources=[r for r in records if domain in r['domains'] and not (domain=='T68' and r['id']=='R-ADMP-C11')], gaps=[dict(id=domain+'-C23-TEXT', status='unavailable', source='WG14 N3096', reason='Attempted web retrieval timed out; no source inspection is claimed. C2x compiler probes are implementation observations, not authoritative final-C23 wording.'), dict(id=domain+'-C17-TEXT', status='unavailable', source='Published ISO C17', reason='N1570 draft clauses provide bounded comparisons; published C17 text was not accessed.')]))
        write(f'research/v5/modern/{family}_research_gate.json', dict(status='passage-inspected', domains=[domain], reviewer='author inspection; not independent review', research_input_sha256={rel:hashlib.sha256((ROOT/rel).read_bytes()).hexdigest()}))
    print(json.dumps(dict(primary_records=len(records), status='bounded author inspection registered')))

if __name__ == '__main__':
    main()
