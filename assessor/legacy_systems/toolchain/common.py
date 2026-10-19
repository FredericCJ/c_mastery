"""Author-side build experiments. Commands are argv vectors, never shell programs."""
from pathlib import Path
import hashlib, json, os, platform, subprocess

def sha(path):
    return hashlib.sha256(Path(path).read_bytes()).hexdigest()

class Campaign:
    def __init__(self, root, output):
        self.root, self.output = Path(root), Path(output)
        self.root.mkdir(parents=True, exist_ok=False)
        self.commands, self.observations, self.checks = [], [], []
        self.group = ''

    def write(self, path, text):
        path = self.root / path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding='utf-8', newline='\n')
        return path

    def run(self, args, cwd=None, expected=0, contains=None, env=None, timeout=90):
        argv = list(map(str, args))
        directory = Path(cwd or self.root)
        try:
            result = subprocess.run(argv, cwd=directory, capture_output=True,
                text=True, timeout=timeout, env=dict(os.environ, LC_ALL='C', **(env or {})))
            rc, out, err = result.returncode, result.stdout, result.stderr
        except FileNotFoundError as error:
            rc, out, err = 'launch-unavailable', '', str(error)
        ok = (rc == expected if expected != 'nonzero' else isinstance(rc,int) and rc != 0)
        ok = ok and (contains is None or contains in out + err)
        row = dict(group=self.group, argv=argv, cwd=str(directory), exit=rc,
            expected=expected, diagnostic=contains, stdout=out, stderr=err,
            environment={'LC_ALL':'C', **(env or {})}, passed=ok)
        self.commands.append(row)
        if not ok:
            raise RuntimeError(json.dumps(row, indent=2))
        return out + err

    def require(self, condition, name, observed=None):
        self.checks.append(dict(group=self.group, name=name, passed=bool(condition), observed=observed))
        if not condition:
            raise RuntimeError(name + ': ' + repr(observed))

    def observe(self, objective, claim, evidence, variant=False, limitations=None):
        self.observations.append(dict(objective=objective, variant=variant, claim=claim,
            evidence=evidence, limitations=limitations or [], group=self.group))

    def finish(self, pins, error=None):
        # Generated original inputs stay inspectable in the reproducible build tree;
        # receipts include their content as well as hashes. Never copy the host environment.
        inputs={}
        for p in self.root.rglob('*'):
            if not p.is_file() or any(x in p.parts for x in ('CMakeFiles','.git')):continue
            if p.suffix in ('.c','.h','.S','.cmake','.ninja','.json','.txt','.py') or p.name=='Makefile':
                if p.stat().st_size < 100000:
                    inputs[p.relative_to(self.root).as_posix()]={'sha256':sha(p),'content':p.read_text(errors='replace')}
        result=dict(schema_version=1, role='author reference experiments; no learner credit',
            environment=platform.uname()._asdict(), input_sha256=pins,
            commands=self.commands, checks=self.checks, observations=self.observations,
            generated_inputs=inputs, error=error, passed=error is None,
            limits=['Bounded Linux/toolchain experiments; no physical target claim.',
                    'A successful reference run does not establish independent learner performance.',
                    'Source identity and reproducibility do not establish trust in a toolchain.'])
        self.output.parent.mkdir(parents=True,exist_ok=True)
        self.output.write_text(json.dumps(result,indent=2)+'\n',encoding='utf-8')
        return result
