import json
import argparse
from pathlib import Path


def hexify_table(filename: Path) -> None:
    with open(filename, "r") as f:
        print("{") 
        data = json.load(f)
        for i in range(0, len(data), 16):
            print(f"\t{', '.join(map(hex, data[i:i+16]))}")
        print("};")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("file", metavar="FILE", type=Path, help="filename of win table")
    args = parser.parse_args()
    hexify_table(args.file)
