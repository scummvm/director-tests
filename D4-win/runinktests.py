#!/usr/bin/env python

import os
import subprocess
import sys

BASE_PATH = os.path.abspath(os.path.join(__file__, ".."))

BASELINE_PATH = os.path.join(BASE_PATH, "scrbase")
TEST_PATH = os.path.join(BASE_PATH, "scrtest")
DIFF_PATH = os.path.join(BASE_PATH, "scrdiff")

os.makedirs(TEST_PATH, exist_ok=True)
os.makedirs(DIFF_PATH, exist_ok=True)

SCUMMVM_PATH = os.getenv("SCUMMVM_PATH", "scummvm")


# taken from ScummVM's engine/director/types.h
INK_TYPES: list[str] = [
    "kInkTypeCopy",
    "kInkTypeTransparent",
    "kInkTypeReverse",
    "kInkTypeGhost",
    "kInkTypeNotCopy",
    "kInkTypeNotTrans",
    "kInkTypeNotReverse",
    "kInkTypeNotGhost",
    "kInkTypeMatte",
    "kInkTypeMask",
    "kInkTypeBlend",
    "kInkTypeAddPin",
    "kInkTypeAdd",
    "kInkTypeSubPin",
    "kInkTypeBackgndTrans",
    "kInkTypeLight",
    "kInkTypeSub",
    "kInkTypeDark"
]
# transcribed from the "channel" command in the ScummVM debugger
INK1B_SHAPE_DIMS: tuple[int, int] = (53, 52)
INK1B_SHAPE_OFFSETS: list[tuple[int, int]] = [
    (54, 150),
    (119, 150),
    (182, 151),
    (243, 152),
    (304, 152),
    (366, 151),
    (431, 151),
    (495, 151),
    (559, 150),
    (52, 217),
    (117, 217),
    (183, 218),
    (248, 217),
    (309, 215),
    (373, 216),
    (431, 215),
    (496, 215),
    (559, 214),
]

INK8B_SHAPE_DIMS: tuple[int, int] = (49, 57)
INK8B_SHAPE_OFFSETS: list[tuple[int, int]] = [
    (55, 163),
    (112, 164),
    (172, 164),
    (237, 162),
    (303, 162),
    (374, 162),
    (442, 161),
    (502, 163),
    (569, 162),
    (48, 235),
    (108, 237),
    (173, 236),
    (234, 235),
    (297, 237),
    (372, 237),
    (435, 234),
    (501, 232),
    (569, 229),
]


def main():
    subprocess.call([SCUMMVM_PATH, f"--start-movie=RUNINKS.DIR", "--debugflags=fast", "director"])
    files = [f for f in os.listdir(".") if f.startswith("INK") and f.endswith(".DIR")]
    for f in files:
        b, _ = os.path.splitext(f)
        subprocess.run(["compare", "-metric", "NCC", "-fuzz", "10%", f"{BASELINE_PATH}/{b}.BMP", f"{TEST_PATH}/{b}.png", f"{DIFF_PATH}/{b}.png"], capture_output=True)


    def format_result(basename: str, bpp: int, index: int, result: bool) -> str:
        fg = ("B", "W", "C")[index // 3]
        bg = ("B", "W", "C")[index % 3]
        result_str = '\x1b[38;5;10;1mPASSED\x1b[0m' if result else '\x1b[38;5;9;1mFAILED\x1b[0m'
        return f"- {basename}.DIR - {bpp}-bit, {fg} foreground, {bg} background - {result_str}"

    def run_compare(basename: str, width: int, height: int, xpos: int, ypos: int) -> bool:
        args = ["compare", "-metric", "AE", "-fuzz", "10%", "-crop", f"{width}x{height}+{xpos}+{ypos}", f"{BASELINE_PATH}/{basename}.BMP", f"{TEST_PATH}/{basename}.png", f"NULL:"]
        ax = subprocess.run(args, capture_output=True)
        return ax.stderr == b'0' or ax.stderr == b'0 (0)'

    total = 0
    failed = 0
    for type_index, type_name in enumerate(INK_TYPES):
        print(f"{type_name} (shape {type_index}):")
        for i in range(1, 10):
            b = f"INK1B-{i}"
            width = INK1B_SHAPE_DIMS[0]
            height = INK1B_SHAPE_DIMS[1]
            xpos = INK1B_SHAPE_OFFSETS[type_index][0] - INK1B_SHAPE_DIMS[0] // 2
            ypos = INK1B_SHAPE_OFFSETS[type_index][1] - INK1B_SHAPE_DIMS[1] // 2
            result = run_compare(b, width, height, xpos, ypos)
            failed += 1 if not result else 0
            total += 1
            print(format_result(b, 1, i - 1, result))
        for i in range(1, 10):
            b = f"INK8B-{i}"
            width = INK8B_SHAPE_DIMS[0]
            height = INK8B_SHAPE_DIMS[1]
            xpos = INK8B_SHAPE_OFFSETS[type_index][0] - INK8B_SHAPE_DIMS[0] // 2
            ypos = INK8B_SHAPE_OFFSETS[type_index][1] - INK8B_SHAPE_DIMS[1] // 2
            result = run_compare(b, width, height, xpos, ypos)
            failed += 1 if not result else 0
            total += 1
            print(format_result(b, 8, i - 1, result))

        print()


    print(f"{failed}/{total} tests failed ({100*failed/total:.4}%)")
    if failed:
        sys.exit(1)
    else:
        print("Good jorb!")
        sys.exit(0)

if __name__ == "__main__":
    main()
