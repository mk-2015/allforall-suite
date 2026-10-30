# kp

`kp` (Kut and Paste) searches and replaces text in files or standard input, providing flexible pattern matching, line range selection, regex support, counting, and line-level transformation options.

## Usage

```bash
kp [OPTIONS] [INPUT [OUTPUT]]
```

### Options

- `-s OLD NEW` : Replace `OLD` with `NEW` (mandatory).
- `-i` : Perform case-insensitive matching.
- `-mi` : Match `OLD` inside substrings rather than whole lines.
- `-stdin` : Read input from standard input (`stdin`).
- `-q` : Quiet mode; suppresses standard output (requires `OUTPUT`).
- `-n` : Replace only the first occurrence per line.
- `-g` : Replace all occurrences per line (default).
- `-v` : Invert matching; replace non-matching lines with `NEW`.
- `-c` : Count total replacements without modifying output.
- `-l` : Print line numbers of modified lines to standard error (`stderr`).
- `-f` : Treat `OLD` as an ECMAScript regular expression (default: literal string).
- `-F` : Use regular expression replacement formatting for `NEW` (requires `-f`).
- `-R A:B` : Operate only on lines `A` through `B`, inclusive (`A <= B`).
- `-r A` : Operate only on the first `A` lines.
- `-Rr A` : Operate only on the last `A` lines.
- `-h`, `--help` : Display help message and exit.

## Examples

# Replace all occurrences of "foo" with "bar" in input.txt and write to output.txt
```bash
kp -s foo bar input.txt output.txt
```

# Case-insensitive substring replacement reading from stdin
```bash
echo "Hello World" | kp -stdin -i -mi -s world universe
```

# Count occurrences of a regular expression pattern
```bash
kp -c -f -s "[0-9]+" "" data.txt
```

# Replace text only within lines 10 through 20
```bash
kp -R 10:20 -s error warning log.txt
```
