# Reproduce the actual narrower ABI

The ordinary learner family build requires the native hosted toolchain. The
changed-ABI author campaign additionally uses a private i386 sysroot. The first
default `gcc -m32` probe failed because the host had no32-bit libc headers; it was
not recorded as a passing narrow result. The follow-up extracted packages under
ignored `build/posix-io-toolchain` and left system installation state unchanged.

From WSL Ubuntu, in a repository-local temporary tools directory:

```sh
mkdir -p build/posix-io-toolchain/packages build/posix-io-toolchain/sysroot
cd build/posix-io-toolchain/packages
apt-get download libc6-dev-i386 libc6-i386 lib32gcc-15-dev lib32gcc-s1 libc6-dev linux-libc-dev
for package in ./*.deb; do dpkg-deb -x "$package" ../sysroot; done
cd ../sysroot
ln -s usr/lib lib
ln -s usr/lib32 lib32
ln -s ../lib32/ld-linux.so.2 usr/lib/ld-linux.so.2
```

These are initial-setup commands for a new private directory, not permission to
replace an existing tree. Package versions/hashes are recorded in each campaign;
the observed packages were glibc2.43-2ubuntu2.4, lib32gcc-15-dev15.2.0-16ubuntu1,
lib32gcc-s1 16-20260322-1ubuntu1 and linux-libc-dev7.0.0-38.38. A future version
must be recorded as new toolchain evidence. The extra relative symlinks reproduce
the package layout's merged `/usr` aliases inside this private root only.

Run the producer with the absolute Linux sysroot path:

```sh
python3 tools/run_posix_io.py \
  --i386-sysroot /absolute/repository/build/posix-io-toolchain/sysroot \
  --report reviews/v5/expansion/legacy_posix_io/io_validation.json
```

The producer selects the explicit loader/library path, records compiler commands
and actual dependency hashes, and requires eight profiles: six native plus GCC
i386 with default32-bit `off_t` and `_FILE_OFFSET_BITS=64`. The large sparse
experiment runs on native and large-file ABIs; the default32-bit profile instead
verifies its smaller representability boundary without invoking that unsupported
large extent. Loader/headers/libraries are host verification tools, not target
dependencies or learner-exported assessor material. This setup does not provide
a different operating system's interrupted-close behavior; that remains an
explicit transition model.
