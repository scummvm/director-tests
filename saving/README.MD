Unit Tests for Saving Movies
============================

The objective of these tests is test the saving functionality in ScummVM's Director for various resources.

`text_d4/5_mac/win.dir`: Two buttons (one duplicates a pre-existing text cast member and the other saves the movie) and a text element

`palette_d4/5_mac/win.dir`: Two buttons (one duplicates a pre-existing palette cast member and the other saves the movie), and a palette

`filmloop_d4/5_mac/win.dir`: Two buttons (one duplicates a pre-existing filmloop cast member and the other saves the movie), a filmloop and a bitmap (for reference)

`bitmap_d4/5_mac/win.dir`: Two buttons (one duplicates a pre-existing bitmap cast member and the other saves the movie) and a bitmap

`bitmap_*bpp.dir`: Two buttons (one duplicates a pre-existing bitmap cast member and the other saves the movie) for a bitmap of depth *bpp

`multiple_casts_test_d5.dir`: Two casts each with its own cast members and two buttons to duplicate one cast member from each cast respectively (bitmap and palette).


Automated Round-Trip Harness
----------------------------

`run_roundtrip_tests.sh` saves each fixture through ScummVM's `test_save_movie` hook, structurally validates the VWSC score chunk (`check_vwsc.py`), reloads the save, and asserts:

- All three runs (pristine load, save, reload) exit cleanly (rc 0).
- No new `WARNING:` lines appear on reload that weren't in the pristine load.
- ScummVM's loader computes the same frame count before and after the round-trip.
- For D6+ movies: the VWSC wrapper has a valid header, a monotonic detail index with the trailing sentinel, and a frame stream that walks cleanly. The frame count is unchanged from source to saved, and the sprite-detail entries (behaviors, names, sprite info) are copied byte-for-byte (entry 0 is re-serialized; entries 1..N are verbatim, verified by hash).

Usage:

    SCUMMVM=/path/to/scummvm ./run_roundtrip_tests.sh [movie.dir ...]

With no arguments it runs the D8 fixtures shipped here. The pre-existing
D4/D5 movies can be passed explicitly (`./run_roundtrip_tests.sh
*_d5_win.dir`); the classic D4/D5 writer does not yet round-trip its
unparsed `unkN` channel bytes or the film-loop SCVW score, so those runs
report known failures. The Director version is inferred from the
filename (`_d8_win` -> marker `d8-win`); override with `MARKER=d7-win`
for ad-hoc movies (e.g. game extracts).


D8 Fixtures
-----------

`score_d8_win.dir`: 8-frame score authored in Director 8.0 (embedded `fileVersion` 0x640, detected as v800), a bitmap sprite in motion plus two inert button cast members. Exercises the D6+ VWSC writer and byte-for-byte preservation of the sprite-detail entries.

`text_d8_win.dir`: 8-frame score with a text sprite in motion, plus two inert button cast members. Provides D8 load/save/reload smoke coverage with a text member present (the text CASt bytes are not mutated, so this is not a text-writer test).

See `BUILDING.md` for how these were authored.


Fixtures wanted
---------------

- `bitmap_d8_win.dir` (negative): a bitmap modified at runtime by a movie script, to assert save refusal (D8 has no bitmap writer). Needs a mutation that sets the member's changed flag; the refusal path is already covered empirically by D6/D8 game saves.
- D6 and D7 fixtures (require Director 6 / 7 to author). The D6.5 and D7 writer paths are currently exercised against real game score chunks via `MARKER=`.
