#!/usr/bin/env python3
"""Convert EEPROM text dumps to raw binary files.

Expected input format per non-empty line:
	0x0000 -> D8

Blank lines are ignored.
"""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path


LINE_RE = re.compile(
	r"^\s*0x([0-9A-Fa-f]+)\s*->\s*([0-9A-Fa-f]{2})\s*$"
)


def parse_eeprom_dump(input_path: Path) -> bytearray:
	"""Parse dump lines and return a bytearray with reconstructed memory."""
	parsed: dict[int, int] = {}

	with input_path.open("r", encoding="utf-8") as f:
		for line_number, raw_line in enumerate(f, start=1):
			line = raw_line.strip()

			# Requested behavior: skip empty lines.
			if not line:
				continue

			match = LINE_RE.match(line)
			if not match:
				raise ValueError(
					f"Invalid format at line {line_number}: {raw_line.rstrip()}"
				)

			address = int(match.group(1), 16)
			value = int(match.group(2), 16)

			existing = parsed.get(address)
			if existing is not None and existing != value:
				raise ValueError(
					"Conflicting values for address "
					f"0x{address:04X} at line {line_number}"
				)

			parsed[address] = value

	if not parsed:
		raise ValueError("No data found in input file.")

	max_address = max(parsed)
	result = bytearray(max_address + 1)

	for address, value in parsed.items():
		result[address] = value

	return result


def build_output_path(input_path: Path, output_arg: str | None) -> Path:
	if output_arg:
		return Path(output_arg)
	return input_path.with_suffix(input_path.suffix + ".bin")


def parse_args() -> argparse.Namespace:
	parser = argparse.ArgumentParser(
		description=(
			"Convert EEPROM dump text (e.g. '0x0000 -> D8') to a binary file."
		)
	)
	parser.add_argument("input", help="Path to EEPROM dump text file")
	parser.add_argument(
		"-o",
		"--output",
		help="Output binary file path (default: <input>.bin)",
	)
	return parser.parse_args()


def main() -> int:
	args = parse_args()

	input_path = Path(args.input)
	if not input_path.is_file():
		print(f"Input file not found: {input_path}", file=sys.stderr)
		return 1

	output_path = build_output_path(input_path, args.output)

	try:
		data = parse_eeprom_dump(input_path)
	except ValueError as exc:
		print(f"Error: {exc}", file=sys.stderr)
		return 2

	output_path.parent.mkdir(parents=True, exist_ok=True)
	output_path.write_bytes(data)

	print(
		"Wrote "
		f"{len(data)} bytes "
		f"(0x0000..0x{len(data) - 1:04X}) "
		f"to {output_path}"
	)
	return 0


if __name__ == "__main__":
	raise SystemExit(main())