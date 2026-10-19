"""Read-only guest ABI/backend identity; no capability or policy mutation."""
from pathlib import Path
import hashlib,json,platform,resource,subprocess
header=Path('/usr/include/linux/io_uring.h');policy=Path('/proc/sys/kernel/io_uring_disabled')
version=subprocess.run(['dpkg-query','-W','linux-libc-dev:amd64'],capture_output=True,text=True)
print(json.dumps(dict(kernel=platform.release(),architecture=platform.machine(),header_sha256=hashlib.sha256(header.read_bytes()).hexdigest(),header_path=str(header),header_package=version.stdout.strip(),header_query_exit=version.returncode,memlock=resource.getrlimit(resource.RLIMIT_MEMLOCK),io_uring_disabled=policy.read_text().strip() if policy.exists() else 'not exposed',backend='actual raw T62 io_uring syscalls; no liburing or fallback',physical_measurement=False)))
