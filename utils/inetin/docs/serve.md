# serve

`serve` is a lightweight HTTP/HTTPS static file server supporting CORS headers, custom ports, and directory selection.

## Usage

```bash
serve [OPTIONS]
```

## Options

- `-h`, `--help` : Display help message and exit.
- `--cors` : Enable Cross-Origin Resource Sharing (CORS headers).
- `--https` : Enable secure HTTPS mode (requires local `cert.pem` and `key.pem`).
- `--port <NUMBER>` : Specify custom port number (default: 8080).
- `--folder <PATH>` : Specify the directory path to serve (default: `"."`).

## Examples

Serve the current directory on port 3000 with CORS enabled:
```bash
serve --port 3000 --cors
```

Serve a specific folder via HTTPS:
```bash
serve --folder ./dist --https
```
