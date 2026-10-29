# whichall

`whichall` searches all directories in the current `PATH` environment variable and prints every occurrence of the requested executable.

## Usage

```bash
whichall [OPTIONS] <binary>
```

### Options

- `-h` — Display help.
- `-j` — Output JSON.
- `-x` — Output XML.
- `-y` — Output YAML.

### Description

For each result, report:
- `path`: Absolute path to the executable.
- `mod`: Last modification timestamp (ISO 8601 UTC).
- `user`: Owner of the file.

### Examples

Find all occurrences of `ls`:
```bash
whichall ls
```

Output as JSON:
```bash
whichall -j ls
```
