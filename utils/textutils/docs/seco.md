# seco

`seco` is a Search and Copy utility that filters lines from standard input based on a query, performs optional text replacement, highlights URLs, computes statistics, and copies the resulting output to the system clipboard (or routes it to stdout or a file).

## Usage

```bash
seco [OPTIONS] [QUERY]
```

## Options

- `-F FILE` : Copy output to `FILE` instead of the clipboard.
- `-S` : Output to stdout instead of the clipboard (overrides `-F`).
- `-C` : Make the search query case-sensitive (case-insensitive by default).
- `-R` : Use regular expressions (builtin C++ regex library) for search matching.
- `-D QUERY` : Disinclude results that contain `QUERY`.
- `-T TEMPL REPL` : Replace all occurrences of `TEMPL` with `REPL` in matching lines.
- `-U` : Invert match; include only results that do NOT match `QUERY`.
- `-u` : Highlight detected URLs in ANSI red.
- `-E` : Append line, word, and character counts in `wc` format at the end of output.
- `-h`, `--help` : Show help message and exit.

## Examples

Search for lines containing "error" and copy them to the clipboard:
```bash
cat log.txt | seco error
```

Search case-insensitively and output to stdout:
```bash
cat data.txt | seco -S -C keyword
```

Search using regular expressions, highlight URLs, and save to a file:
```bash
cat access.log | seco -R -u -F output.txt "HTTP/[0-9\.]+"
```

Invert match (exclude lines containing "debug") and count statistics:
```bash
cat app.log | seco -U -E debug
```
