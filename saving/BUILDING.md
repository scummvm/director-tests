Building Saving Test Fixtures
=============================

The `.dir` fixtures are hand-authored in Macromedia Director and committed
as binaries. The `.ls` files in this directory are the builder handlers
used to generate the D8 fixtures, kept so the binaries can be regenerated
or audited.

D4/D5 fixtures
--------------

Authored in Director 5 (Windows). Open a new movie, build the cast and
score by hand or via the Message window, and save.

D8 fixtures
-----------

Requires Macromedia Director 8 (Windows), e.g. via Wine:

    WINEPREFIX=~/d8work/wine wine \
      "C:\Program Files (x86)\Macromedia\Director 8\Director.exe"

`beginRecording`/`endRecording` only work inside a compiled handler, not
from the Message window directly, so each fixture is built by a movie
script:

1. **File > New > Movie**, then **File > Save As** to the target path
   (e.g. `score_d8_win.dir`). `saveMovie` with no argument then saves in
   place.
2. Create a **movie script** cast member: **Window > Script** (or Insert
   a Script cast member). Confirm the type in the Message window:

       put member(1).type
       -- #script

   It must be `#script`, not `#text`. A text member will not compile the
   handler.
3. Paste the matching `.ls` source into the Script Editor and close it
   (it auto-compiles). Straight ASCII quotes are required; pasting through
   some clipboards turns `"` into smart quotes and breaks compilation.
   Pasting from a plain-text editor avoids this.
4. In the Message window, invoke the handler (`buildIt` for the score
   fixtures, `buildText` for the text fixture). It creates the cast
   members, records the score, and saves.

The builder creates plain bitmap/text and two **inert** button cast
members (named `dup_button` / `save_button` but with no scripts attached)
purely to populate the cast; the saving path under test is the score
(VWSC) writer, which re-serializes on every save regardless.

Director's `startUp` handler (LINGO.INI) runs before any movie context
exists, so headless authoring via LINGO.INI is not possible — the GUI
build above is the supported method.

Expected contents
-----------------

Reopen in Director to confirm before committing (`put the lastFrame`):

- `score_d8_win.dir`: `box` (bitmap), `dup_button`, `save_button`; 8
  frames, sprite 1 = box moving on frames 1-3 then static, sprites 2-3
  the buttons.
- `text_d8_win.dir`: `sample_text` (text), `dup_button`, `save_button`;
  8 frames, sprite 1 = the text moving on frames 1-2 then static.
