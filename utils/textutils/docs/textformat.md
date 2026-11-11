# textformat

`textformat` transforms text case and naming conventions between formats such as CamelCase, PascalCase, SnakeCase, KebabCase, and more using modern C++20.

## Usage

```bash
textformat [OPTIONS] [INPUT [OUTPUT]]
```

## Options

- `-a FORMAT` : Source text format (default: `auto`)
- `-b FORMAT` : Destination text format (required)
- `--list` : List all supported formats
- `-h`, `--help` : Show help message and exit

## Supported Formats

- `camel`, `CamelCase` : lowerCamelCase (e.g., `fooBarBaz`)
- `pascal`, `PascalCase` : UpperCamelCase (e.g., `FooBarBaz`)
- `snake`, `SnakeCase` : snake_case (e.g., `foo_bar_baz`)
- `kebab`, `KebabCase` : kebab-case (e.g., `foo-bar-baz`)
- `constant`, `ConstantCase` : CONSTANT_CASE (e.g., `FOO_BAR_BAZ`)
- `title`, `TitleCase` : Title Case (e.g., `Foo Bar Baz`)
- `lower` : lowercase (e.g., `foobarbaz`)
- `upper` : UPPERCASE (e.g., `FOOBARBAZ`)
- `auto` : Automatic tokenization

## Examples

Convert snake_case to PascalCase:
```bash
echo "hello_world_text" | textformat -b PascalCase
```

Convert CamelCase to KebabCase:
```bash
textformat -a CamelCase -b kebab input.txt output.txt
```
