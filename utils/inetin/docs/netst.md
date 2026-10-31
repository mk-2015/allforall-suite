# netst

`netst` is a cross-platform network socket status utility (supporting Linux, Windows, and FreeBSD) that enumerates network sockets and associates them with owning processes, PIDs, and arguments.

## Usage

```bash
netst [OPTIONS]
```

## Options

- `-h`, `--help` : Display help message and exit.
- `-p PORT` : Show sockets matching local IP and port (e.g. `127.0.0.1:8080` or `:80`).
- `-i PID` : Show sockets owned by the specified Process ID (PID).
- `-r PORT` : Show process path and command-line arguments for the specified listening/bound port.
- `-pp PORT` : Print only the owning PID for the specified port.
- `-o` : Output socket information in pipe-delimited format (with headers: `protocol|local|remote|state|pid|process_path|process_args`).

## Examples

List all network sockets and their owning processes:
```bash
netst
```

Show sockets matching a specific local port:
```bash
netst -p :443
```

Show sockets owned by a specific PID:
```bash
netst -i 1234
```

Find the process path and arguments bound to port 80:
```bash
netst -r 80
```

Print only the PID bound to port 8080:
```bash
netst -pp 8080
```

Export socket status as pipe-delimited data:
```bash
netst -o
```

## Exit Status

- `0`: Success.
- `2`: Argument parsing error, missing option arguments, or unknown option.
