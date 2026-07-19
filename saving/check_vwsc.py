#!/usr/bin/env python3
"""Structural checks for the VWSC score chunk of a saved Director movie.

For D6+ movies, validates the wrapper the ScummVM score writer emits:
header, sprite-detail index with trailing sentinel, entry 0 frames
stream (walked frame by frame) and detail entries. For D4/D5 movies,
validates the classic frames stream header.

Usage: check_vwsc.py <movie file> [--min-frames N]
Exit code 0 = pass, 1 = structural failure.
"""

import re
import struct
import sys


def fail(msg):
    print(f"FAIL: {msg}")
    sys.exit(1)


def find_vwsc(data):
    le = data[:4] == b'XFIR'
    if not le and data[:4] != b'RIFX':
        fail("not a RIFX/XFIR movie")
    u32 = (lambda o: struct.unpack_from("<I", data, o)[0]) if le \
        else (lambda o: struct.unpack_from(">I", data, o)[0])
    u16 = (lambda o: struct.unpack_from("<H", data, o)[0]) if le \
        else (lambda o: struct.unpack_from(">H", data, o)[0])
    mmap_off = u32(24)
    d = mmap_off + 8
    hlen, elen = u16(d), u16(d + 2)
    count = u32(d + 8)
    base = d + hlen
    for i in range(count):
        eo = base + i * elen
        tag = data[eo:eo + 4]
        if le:
            tag = tag[::-1]
        if tag == b'VWSC':
            size, off = u32(eo + 4), u32(eo + 8)
            return data[off + 8:off + 8 + size]
    fail("no VWSC resource found")


def be32(p, o):
    return struct.unpack_from(">I", p, o)[0]


def sbe32(p, o):
    return struct.unpack_from(">i", p, o)[0]


def be16(p, o):
    return struct.unpack_from(">H", p, o)[0]


def walk_frames(p, start, end):
    """Yield frame sizes; assert chunk framing consistency."""
    sizes = []
    q = start
    while q < end:
        fs = be16(p, q)
        if fs < 2:
            fail(f"frame size {fs} < 2 at 0x{q:x}")
        # walk the chunks inside the frame
        c = q + 2
        while c < q + fs:
            csize, coff = be16(p, c), be16(p, c + 2)
            if c + 4 + csize > q + fs:
                fail(f"chunk (size {csize} off {coff}) overruns frame at 0x{q:x}")
            c += 4 + csize
        sizes.append(fs)
        q += fs
    if q != end:
        fail(f"frame walk overran entry 0: 0x{q:x} != 0x{end:x}")
    return sizes


def check(path, min_frames):
    data = open(path, 'rb').read()
    p = find_vwsc(data)

    mem_size, ver, list_start = be32(p, 0), sbe32(p, 4), be32(p, 8)

    if ver >= 0:
        # D4/D5 classic stream: sanity-check the 20-byte header only
        frames_size = be32(p, 0)
        if frames_size != len(p):
            fail(f"D4/D5 framesStreamSize {frames_size} != chunk size {len(p)}")
        print(f"PASS: D4/D5 VWSC, {len(p)} bytes")
        return

    # D6+ wrapper
    if mem_size != len(p):
        fail(f"memSize {mem_size} != chunk size {len(p)}")
    if list_start != 12:
        fail(f"listStart {list_start} != 12")

    num_entries = sbe32(p, 12)
    list_size = sbe32(p, 16)
    max_data_len = be32(p, 20)
    if list_size != num_entries + 1:
        fail(f"listSize {list_size} != numEntries {num_entries} + 1 (sentinel missing)")

    offs = [be32(p, 24 + 4 * i) for i in range(list_size)]
    for i in range(1, list_size):
        if offs[i] < offs[i - 1]:
            fail(f"offsets not monotonic at index {i}")
    fdo = 24 + list_size * 4
    if fdo + offs[-1] != len(p):
        fail(f"frameDataOffset {fdo} + last offset {offs[-1]} != chunk size {len(p)}")
    if max_data_len != offs[-1]:
        fail(f"maxDataLen {max_data_len} != total data length {offs[-1]}")

    # entry 0: frames stream
    e0 = fdo + offs[0]
    e0_size = be32(p, e0)
    # The offset table may pad the entry to a word boundary, so the gap
    # can exceed the frames-stream size field by a byte or two. The size
    # field is authoritative (it's what the loader reads).
    entry_gap = offs[1] - offs[0]
    if e0_size > entry_gap or entry_gap - e0_size > 3:
        fail(f"entry0 size field {e0_size} inconsistent with entry gap {entry_gap}")
    f1_off = be32(p, e0 + 4)
    if f1_off != 20:
        fail(f"frame1Offset {f1_off} != 20")
    num_frames = be32(p, e0 + 8)

    sizes = walk_frames(p, e0 + 20, e0 + e0_size)
    if len(sizes) < min_frames:
        fail(f"only {len(sizes)} frames, expected >= {min_frames}")
    if num_frames and len(sizes) != num_frames:
        print(f"note: header numOfFrames {num_frames} != walked {len(sizes)} (loader recomputes)")

    detail_data = sum((offs[i + 1] - offs[i]) for i in range(1, num_entries))
    print(f"PASS: D6+ VWSC ver={ver}, {num_entries} entries, {len(sizes)} frames "
          f"(sizes {sizes[:6]}), detail bytes {detail_data}")


def detail_hash(path):
    """Hash of the concatenated sprite-detail entries (1..N-1).

    These entries carry sprite info, behaviors and names and are copied
    verbatim by the writer, so the hash must match source vs saved even
    though entry 0 (the frames stream) is re-serialized and its size
    differs. Returns None for non-D6+ movies.
    """
    import hashlib
    data = open(path, 'rb').read()
    p = find_vwsc(data)
    if sbe32(p, 4) >= 0:
        return None
    num_entries = sbe32(p, 12)
    list_size = sbe32(p, 16)
    offs = [be32(p, 24 + 4 * i) for i in range(list_size)]
    fdo = 24 + list_size * 4
    h = hashlib.sha256()
    for i in range(1, num_entries):
        chunk = p[fdo + offs[i]:fdo + offs[i + 1]]
        h.update(struct.pack('>I', len(chunk)))
        h.update(chunk)
    return h.hexdigest()


if __name__ == '__main__':
    if len(sys.argv) < 2:
        print(__doc__)
        sys.exit(2)
    # --frames-only prints just the walked frame count (for harness diffing)
    if '--frames-only' in sys.argv:
        import io
        import contextlib
        buf = io.StringIO()
        try:
            with contextlib.redirect_stdout(buf):
                check(sys.argv[1], 1)
        except SystemExit:
            print("-1")
            sys.exit(1)
        m = re.search(r'(\d+) frames', buf.getvalue())
        print(m.group(1) if m else "-1")
        sys.exit(0)
    if '--detail-hash' in sys.argv:
        try:
            h = detail_hash(sys.argv[1])
        except SystemExit:
            print("ERR")
            sys.exit(1)
        print(h if h else "NONE")
        sys.exit(0)
    minf = 1
    if '--min-frames' in sys.argv:
        minf = int(sys.argv[sys.argv.index('--min-frames') + 1])
    check(sys.argv[1], minf)
