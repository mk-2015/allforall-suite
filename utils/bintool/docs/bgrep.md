# bgrep

Search a binary file for a fixed byte pattern or ASCII string using grep-like behavior.

## Usage

bgrep [OPTIONS] <file> <pattern>
bgrep [OPTIONS] -e <pattern> <file>

## Options

- -e, --expr <pattern>
  Supply a search expression directly
- -x, --hex
  Search for a hex byte sequence such as `00 ff aa` or `00ffaa`
- -i, --ignore-case
  Match text case-insensitively
- -n, --line-number
  Print match offsets in hex/decimal form
- -v, --invert-match
  Print non-matching offsets instead of matches
- -C, --context <bytes>
  Print a byte context window around each match
- -m, --max-count <n>
  Stop after N matches
- -h, --help
  Show the help menu

## Examples

- bgrep my.bin "HELLO"
- bgrep -i my.bin "hello"
- bgrep -x my.bin 00ffaa
- bgrep -x my.bin "00 ff aa"
- bgrep -C 16 -m 5 my.bin "SIGNATURE"

## Output

Each match is printed with its byte offset:

offset=0x10 (16 bytes)

## Notes

- This is still a fixed-pattern search tool, not a full regex engine.
- It is designed for locating signatures, markers, and byte sequences inside binary files.
- The output model is grep-like, but offset-based because binary data is not line-oriented.
