"""Require preserved historical bytes and refusal before publication."""
import copy,importlib.util,json,sys
from pathlib import Path
spec=importlib.util.spec_from_file_location('learner_ledger',Path(sys.argv[1]));module=importlib.util.module_from_spec(spec);spec.loader.exec_module(module)
request=json.loads((Path(__file__).parent/'authority_cases.json').read_text())['change_request'];history=[request['original']];incoming=request['incoming'];before=copy.deepcopy(history)
try:result=module.append_revision(history,incoming)
except NotImplementedError:print('LC_LEDGER_CHECK unfinished');raise SystemExit(1)
assert result==[request['original'],request['incoming']] and history==before
result[0]['source']='mutated-copy';result[1]['source']='mutated-new';assert history==before and incoming==request['incoming']
controls=[dict(incoming,revision=1),dict(incoming,revision=3),dict(incoming,revision=True),dict(incoming,source=''),dict(incoming,status='unverified'),dict(incoming,extra=1)]
for candidate in controls:
 try:module.append_revision(history,candidate)
 except ValueError:pass
 else:raise AssertionError('LC_LEDGER_CHECK accepted invalid authority update')
 assert history==before
print('LC_LEDGER preserved_history rejected6 deep_copy')
