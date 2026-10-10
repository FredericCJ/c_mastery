param([int]$Port = 8766)
$taskPython = Join-Path $PSScriptRoot '.venv\Scripts\python.exe'
& $taskPython -m http.server $Port --bind 127.0.0.1 --directory (Join-Path $PSScriptRoot 'build\html')
