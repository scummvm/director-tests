#!/usr/bin/env bash
# Save -> reload round-trip check for the movies in this directory.
# Needs a ScummVM build with the test_save_movie config hook (not yet in
# stock master); without it no save is written and every fixture fails.
# Usage: SCUMMVM=/path/to/scummvm ./run_roundtrip_tests.sh [movie.dir ...]
set -u

SCUMMVM="${SCUMMVM:-scummvm}"
HERE="$(cd "$(dirname "$0")" && pwd)"
WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

# filename -> custom-target marker; override with MARKER=dN-win
marker_for() {
	if [ -n "${MARKER:-}" ]; then echo "$MARKER"; return; fi
	case "$1" in
		*_d[0-9]*_mac*) echo "$1" | sed -E 's/.*_d([0-9]+)_(mac).*/d\1-\2/' ;;
		*_d[0-9]*_win*) echo "$1" | sed -E 's/.*_d([0-9]+)_(win).*/d\1-\2/' ;;
		*) echo "d5-win" ;;
	esac
}

run_movie() { # <dir> <movie> <log> [save-name]
	local dir="$1" movie="$2" log="$3" save="${4:-}"
	local cfg="$WORK/run.ini"
	{
		echo "[scummvm]"
		echo "savepath=$WORK"
		[ -n "$save" ] && echo "test_save_movie=$save"
	} > "$cfg"
	touch "$dir/$(marker_for "$movie")"
	"$SCUMMVM" -c "$cfg" -d 5 --debugflags=loading,saving,fewframesonly \
		--auto-detect --path="$dir" --start-movie="$movie" > "$log" 2>&1
}

frames_of() { grep -o 'Calculated, total number of frames [0-9]*' "$1" | tail -1 | grep -o '[0-9]*$'; }

# WARNING lines, run path and per-run movie names folded out so pre-existing warnings cancel in the diff.
norm_warnings() { # <log> <src-movie> <saved-movie>
	grep -E '^WARNING' "$1" | sed -E \
		-e "s#${WORK}[^ '\"]*#<WORK>#g" \
		-e "s#${2//./\\.}#<MOVIE>#g" \
		-e "s#${3//./\\.}#<MOVIE>#g"
}

fail=0
pass=0

# Default set: the D8 fixtures this harness ships with. The pre-existing
# D4/D5 movies can be passed explicitly; the classic (D4/D5) writer does
# not round-trip its unparsed unkN channel bytes or the film-loop SCVW
# score cleanly yet, so those runs surface known failures.
movies=("$@")
[ ${#movies[@]} -eq 0 ] && movies=("$HERE"/score_d8_win.dir "$HERE"/text_d8_win.dir)

for m in "${movies[@]}"; do
	name="$(basename "$m")"
	out="${name%.dir}_roundtrip.dir"

	stage="$WORK/${name%.dir}"
	mkdir -p "$stage"
	cp "$m" "$stage/"

	run_movie "$stage" "$name" "$WORK/$name.load.log"
	load_rc=$?
	if [[ $load_rc -ne 0 ]]; then
		echo "FAIL $name: pristine load exit $load_rc"; tail -3 "$WORK/$name.load.log"; fail=$((fail+1)); continue
	fi
	run_movie "$stage" "$name" "$WORK/$name.save.log" "$out"
	save_rc=$?

	# writeToFile() prefixes the save with the target name
	saved="$(ls "$WORK"/*"$out" 2>/dev/null | head -1)"

	if [[ $save_rc -ne 0 ]]; then
		echo "FAIL $name: save run exit $save_rc"; tail -3 "$WORK/$name.save.log"; fail=$((fail+1)); continue
	fi
	if [[ -z "$saved" ]]; then
		echo "FAIL $name: no save produced"; tail -3 "$WORK/$name.save.log"; fail=$((fail+1)); continue
	fi
	if ! python3 "$HERE/check_vwsc.py" "$saved" --min-frames 2 > "$WORK/$name.vwsc" 2>&1; then
		echo "FAIL $name: VWSC structure"; cat "$WORK/$name.vwsc"; fail=$((fail+1)); continue
	fi

	src_frames="$(python3 "$HERE/check_vwsc.py" "$m" --frames-only 2>/dev/null)"
	saved_frames="$(python3 "$HERE/check_vwsc.py" "$saved" --frames-only 2>/dev/null)"
	if [[ "$src_frames" -ge 0 && "$src_frames" != "$saved_frames" ]]; then
		echo "FAIL $name: frame count changed on save: $src_frames -> $saved_frames"; fail=$((fail+1)); continue
	fi

	src_dh="$(python3 "$HERE/check_vwsc.py" "$m" --detail-hash 2>/dev/null)"
	saved_dh="$(python3 "$HERE/check_vwsc.py" "$saved" --detail-hash 2>/dev/null)"
	if [[ "$src_dh" != "NONE" && "$src_dh" != "$saved_dh" ]]; then
		echo "FAIL $name: detail entries not preserved verbatim"; fail=$((fail+1)); continue
	fi

	rstage="$WORK/${name%.dir}_reload"
	mkdir -p "$rstage"
	cp "$saved" "$rstage/$out"
	run_movie "$rstage" "$out" "$WORK/$name.reload.log"
	reload_rc=$?

	if [[ $reload_rc -ne 0 ]]; then
		echo "FAIL $name: reload exit $reload_rc"; fail=$((fail+1)); continue
	fi

	bw="$(comm -23 \
		<(norm_warnings "$WORK/$name.reload.log" "$name" "$out" | sort -u) \
		<(norm_warnings "$WORK/$name.load.log" "$name" "$out" | sort -u))"
	if [[ -n "$bw" ]]; then
		echo "FAIL $name: new reload warnings:"; echo "$bw" | head -5; fail=$((fail+1)); continue
	fi

	f1="$(frames_of "$WORK/$name.save.log")"; f2="$(frames_of "$WORK/$name.reload.log")"
	if [[ -z "$f2" || "$f1" != "$f2" ]]; then
		echo "FAIL $name: frame count ${f1:-none} -> ${f2:-none}"; fail=$((fail+1)); continue
	fi
	echo "PASS $name ($f2 frames round-tripped)"; pass=$((pass+1))
done

echo "----"
echo "passed: $pass, failed: $fail"
exit $((fail > 0))
