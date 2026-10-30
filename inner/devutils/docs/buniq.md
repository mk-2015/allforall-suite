# buniq

`buniq` filters matching records (lines or NUL-delimited entries) from `INPUT`, writing the output to `OUTPUT`. It provides advanced text and binary record filtering, deduplication, grouping, field/character skipping, and case-insensitive comparison capabilities.

## Usage

```bash
buniq [OPTIONS]... [INPUT [OUTPUT]]
```

## Options

- `-c` : Prefix records with group count.
- `-d` : Output only one representative per duplicate group (mutually exclusive with other selection modes).
- `-D` : Output all records from duplicate groups (mutually exclusive with other selection modes).
- `--all-repeated[=METHOD]` : Output all duplicate records. `METHOD` can be `none`, `prepend`, or `separate`.
- `-u` : Output records belonging to singleton (unique) groups.
- `--group[=METHOD]` : Output all groups. `METHOD` can be `separate`, `prepend`, `append`, or `both`.
- `-i` : Perform ASCII case-insensitive comparison.
- `-f N` : Skip `N` whitespace-separated fields before comparison.
- `-s N` : Skip `N` bytes after field skipping.
- `-w N` : Compare at most `N` bytes after skips.
- `-z` : Use NUL (`\0`) as the record delimiter instead of newline (`\n`).
- `-aj` : Group non-adjacent records globally across the entire input.
- `-h`, `--help` : Show help message and exit.
- `--` : Stop option parsing.

Compound flags (e.g., `-ci`, `-cz`) are also supported.

## Examples

Filter adjacent duplicate lines from standard input:
```bash
buniq input.txt output.txt
```

Output only unique (singleton) records case-insensitively:
```bash
buniq -u -i input.txt
```

Group all records globally (even non-adjacent ones) and prefix with counts:
```bash
buniq -c -aj input.txt
```

Compare records skipping the first 2 fields and at most 10 bytes:
```bash
buniq -f 2 -w 10 input.txt
```
