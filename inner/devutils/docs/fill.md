# fill

`fill` expands placeholders in a template using supplied values and generates combinations.

## Usage

```bash
fill [OPTIONS] TEMPLATE --fill=NAME:[VALUES...]
```

## Options

- `-h`, `--help` : Display help message.
- `-a forall` : Generate all combinations (default).
- `-a NUM` : Generate `NUM` combinations.
- `-r` : Randomize output order; with `-a NUM`, select `NUM` randomly ordered distinct combinations.
- `--` : Stop processing options.

## Template & Fill Syntax

- **Template:** Use `<name>` to mark a placeholder (e.g., `<first>`, `<last>`).
- **Fills:** Define values using `--fill=NAME:[value1,value2,...]`. Values may be quoted with single or double quotes (supporting backslash escapes).

## Examples

Generate all combinations for email addresses:
```bash
fill '<first>.<last>@<service>' \
  '--fill=first:["jessica", "alan"]' \
  '--fill=last:["jones", "becker"]' \
  '--fill=service:["gmail.com", "yahoo.com"]'
```

Generate 3 random combinations:
```bash
fill -a 3 -r '<first>-<last>' \
  '--fill=first:[jessica,alan]' \
  '--fill=last:[jones,becker]'
```
