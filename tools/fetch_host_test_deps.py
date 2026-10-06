"""Isolated, hash-pinned MSYS2 libraries for Windows EPUB integration tests.

Requires Git for Windows' libzstd.dll. No installers or package scripts run.
"""
import ctypes
import hashlib
import io
import os
from pathlib import Path
import shutil
import tarfile
import urllib.request

root = Path(__file__).resolve().parents[1] / '.tools' / 'hostdeps'
git = Path(os.environ['LOCALAPPDATA']) / 'Programs/Git/mingw64/bin'
cookie = os.add_dll_directory(str(git))
zstd = ctypes.CDLL(str(git / 'libzstd.dll'))
zstd.ZSTD_getFrameContentSize.argtypes = [ctypes.c_void_p, ctypes.c_size_t]
zstd.ZSTD_getFrameContentSize.restype = ctypes.c_ulonglong
zstd.ZSTD_decompress.argtypes = [ctypes.c_void_p, ctypes.c_size_t, ctypes.c_void_p, ctypes.c_size_t]
zstd.ZSTD_decompress.restype = ctypes.c_size_t
zstd.ZSTD_isError.argtypes = [ctypes.c_size_t]
zstd.ZSTD_isError.restype = ctypes.c_uint
packages = [
 ('libarchive-3.8.9-6', '591e1e7fb90adc7503bf0edb59e1f6c98d9ffce76b9568a554ccdfe1ed0e4380'),
 ('expat-2.8.5-1', '254d05d2e89acbbb998636dd478fc1d640ce3ec4e63326f17398958d2b68dfeb'),
 ('libb2-0.98.1-3', '3c898f08c5f19e25dc6d7e39aa36b6f323141f0e62c5c50f089cd3f89711854c'),
 ('lz4-1.10.0-1', 'a4c5a3bcd26111554c87591275b8a681bfa4473d1607647e24c22ef6213c055c'),
]
root.mkdir(parents=True, exist_ok=True)
for package, expected in packages:
    name = 'mingw-w64-x86_64-' + package + '-any.pkg.tar.zst'
    data = urllib.request.urlopen('https://mirror.msys2.org/mingw/mingw64/' + name, timeout=60).read(16 * 1024 * 1024)
    assert hashlib.sha256(data).hexdigest() == expected, 'Package hash mismatch'
    source = ctypes.create_string_buffer(data)
    size = zstd.ZSTD_getFrameContentSize(source, len(data))
    if size == 2**64 - 1:  # Streaming frames omit their decompressed size.
        size = 128 * 1024 * 1024
    assert 0 < size <= 128 * 1024 * 1024, 'Unbounded package'
    target = ctypes.create_string_buffer(size)
    actual = zstd.ZSTD_decompress(target, size, source, len(data))
    assert not zstd.ZSTD_isError(actual) and actual <= size
    with tarfile.open(fileobj=io.BytesIO(target.raw[:actual]), mode='r:') as archive:
        for entry in archive:
            if not entry.isfile():
                continue
            path = Path(entry.name)
            allowed = entry.name.startswith('mingw64/include/') or entry.name.startswith('mingw64/share/licenses/') or path.name.endswith(('.dll', '.dll.a'))
            if not allowed:
                continue
            destination = (root / path).resolve()
            assert destination.is_relative_to(root.resolve())
            destination.parent.mkdir(parents=True, exist_ok=True)
            with archive.extractfile(entry) as stream, destination.open('wb') as output:
                shutil.copyfileobj(stream, output)
    print(package, 'verified and extracted')
for name in ['libbz2-1.dll', 'libcrypto-3-x64.dll', 'libgcc_s_seh-1.dll', 'libiconv-2.dll', 'liblzma-5.dll', 'libpcre2-8-0.dll', 'libzstd.dll', 'zlib1.dll', 'libwinpthread-1.dll']:
    shutil.copy2(git / name, root / 'mingw64/bin' / name)
print('Host libraries ready:', root)
