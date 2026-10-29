"""Exercise the native workload's real perf-control pipe/ACK protocol."""
import argparse
import os
import select
import subprocess


def read_line(fd):
    data = b""
    while not data.endswith(b"\n") and len(data) < 16:
        if not select.select([fd], [], [], 3)[0]:
            raise AssertionError("native workload never sent requested control boundary")
        byte = os.read(fd, 1)
        if not byte:
            raise AssertionError("control pipe closed before boundary")
        data += byte
    return data


def scenario(binary, bad_ack):
    read_control, write_control = os.pipe()
    read_ack, write_ack = os.pipe()
    env = dict(os.environ, PF_PERF_CONTROL_FD=str(write_control), PF_PERF_ACK_FD=str(read_ack))
    process = subprocess.Popen([binary, "work", "numeric", "256", "0", "8"],
                               pass_fds=(write_control, read_ack), env=env,
                               stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
    try:
        assert read_line(read_control) == b"enable\n"
        os.write(write_ack, b"bad\n\0" if bad_ack else b"ack\n\0")
        if not bad_ack:
            assert read_line(read_control) == b"disable\n"
            os.write(write_ack, b"ack\n\0")
        stdout, stderr = process.communicate(timeout=5)
        assert process.returncode == (2 if bad_ack else 0), (stdout, stderr)
        assert ("perf-control" in stderr) == bad_ack
        assert ('"checksum":522240' in stdout) == (not bad_ack)
    finally:
        if process.poll() is None:
            process.kill()
            process.wait()
        for fd in (read_control, write_control, read_ack, write_ack):
            os.close(fd)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--binary", required=True)
    args = parser.parse_args()
    scenario(args.binary, False)
    scenario(args.binary, True)
    print("PERF_CONTROL_PASS:real-pipes:enable-ack:disable-ack:bad-ack-rejected")


if __name__ == "__main__":
    main()
