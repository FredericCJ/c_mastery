"""Publication controls for narrative and ordered inspected-passage scopes."""
from copy import deepcopy
import unittest
from common import ROOT, read_json
from publish import inspection_scope_text


class InspectionScopes(unittest.TestCase):
    def test_existing_string_is_byte_identical(self):
        text = '  Only §6.2.4 was inspected.\nNo whole-work reading claim.  '
        self.assertEqual(inspection_scope_text(text), text)

    def test_distinct_passages_keep_order_and_text(self):
        scope = ['Printed pp. 121–122: allocation ownership', 'Appendix B: changed-condition counterexample']
        before = deepcopy(scope)
        self.assertEqual(inspection_scope_text(scope),
                         '1. Printed pp. 121–122: allocation ownership\n'
                         '2. Appendix B: changed-condition counterexample')
        self.assertEqual(scope, before)

    def test_multiline_item_keeps_its_own_list_membership(self):
        scope = ['First passage\ncontinued qualification', 'Second passage']
        self.assertEqual(inspection_scope_text(scope),
                         '1. First passage\n   continued qualification\n2. Second passage')
        from markdown_it import MarkdownIt
        tokens = MarkdownIt('commonmark').parse(inspection_scope_text(scope))
        self.assertEqual(sum(t.type == 'ordered_list_open' for t in tokens), 1)
        self.assertEqual(sum(t.type == 'list_item_open' for t in tokens), 2)

    def test_invalid_shapes_fail_without_invented_prose(self):
        for scope in (None, True, 1, {}, {'scope': 'p1'}, '', ' \n', [], [''],
                      ['p1', None], ['p1', 2], [['p1']], ('p1', 'p2')):
            with self.subTest(scope=scope), self.assertRaises(ValueError):
                inspection_scope_text(scope)

    def test_current_registry_all_scopes_preserved(self):
        path = ROOT/'references/resources.json'
        before = path.read_bytes()
        records = read_json(path)['resources']
        shapes = {'string': 0, 'list': 0}
        for resource in records:
            scope = resource['inspection']['scope']
            original = deepcopy(resource)
            rendered = inspection_scope_text(scope)
            if isinstance(scope, str):
                shapes['string'] += 1
                self.assertEqual(rendered, scope, resource['id'])
            else:
                shapes['list'] += 1
                expected = []
                for number, item in enumerate(scope, 1):
                    lines = item.split('\n')
                    expected.append(str(number) + '. ' + lines[0])
                    expected.extend(' ' * (len(str(number)) + 2) + line for line in lines[1:])
                self.assertEqual(rendered, '\n'.join(expected), resource['id'])
                self.assertNotEqual(rendered, str(scope), resource['id'])
            self.assertEqual(resource, original)
        self.assertGreater(shapes['string'], 0)
        self.assertGreater(shapes['list'], 0)
        self.assertEqual(sum(shapes.values()), len(records))
        regression = next(row for row in records if row['id'] == 'R-EC2-LEAK-SUCCESSOR')
        self.assertIsInstance(regression['inspection']['scope'], list)
        self.assertEqual(path.read_bytes(), before)


if __name__ == '__main__':
    unittest.main()
