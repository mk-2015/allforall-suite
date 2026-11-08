# watchd

`watchd` monitors a directory for file and directory changes (creation, modification, deletion) and executes a specified command when changes occur.

## Usage

```bash
watchd [directory] [command] [OPTIONS]
```

### Options

- `-s` : Watch subfolders recursively.
- `-f` : Watch files only.
- `-d` : Watch directories only.
- `-n` : Ignore hidden files and folders (default).
- `-N` : Include hidden files and folders in monitoring.

## Examples

# Monitor the current directory recursively for file changes and run "make"

```bash
watchd . "make" -s
```

# Monitor a specific directory for file modifications (excluding directories)

```bash
watchd /var/log "echo 'Logs changed'" -f
```
