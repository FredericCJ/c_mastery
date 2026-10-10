$ErrorActionPreference = 'Stop'
$taskPython = Join-Path $PSScriptRoot '.venv\Scripts\python.exe'
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\write_program.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\validate.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\check_fixtures.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\check_contract_guards.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\check_baseline_preservation.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\check_performance_debugging_preservation.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\publish.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\check_html.py')
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
& $taskPython -X utf8 (Join-Path $PSScriptRoot 'tools\check_markdown.py')
exit $LASTEXITCODE
