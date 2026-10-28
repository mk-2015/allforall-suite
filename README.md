# WARNING

- ***This project is still in development! Don't use in production!***
- ***I am not responsible for damages that occur if production environments are damaged!***

## Dependencies

- **alltools:**
  - [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- **hpm:**
  - [miniz](https://github.com/richgel999/miniz)
  - sqlite3 (system)
  - libyaml (system)
  - libcurl (system)
  - OpenSSL (system)
  - zlib (system)
- **bintool:**
  - OpenSSL (system)
  - [LIEF](https://lief-project.github.io/) (system)

# All For All

- An open-source tool library for OS administrators.
- Written in C++ for blazing-fast applications and code readability, welcoming contributors.
- A massive toolkit (coming soon...).

## Build & Installation

- **Supported Platforms:**
  1. Windows
  2. Linux
  3. BSD
  4. macOS (Not supported)
- **Requirements:**
  - Internet connection
  - GCC and Make
- **Steps:**
  - **Step 1:** Clone the repository:
    ```bash
    git clone --depth 1 https://github.com/mk-2015/allforall-suite
    ```
  - **Step 2:** Navigate and build:
    ```bash
    cd allforall-suite && make deps install # Note: 'make install' executes 'make all' before copying.
    ```
  - **Step 3:** Reboot to ensure package changes take full effect:
    - Linux/BSD:
      ```bash
      sudo reboot
      ```
    - Windows:
      ```cmd
      shutdown /r /t 0
      ```
  - **Step 4:** Enjoy!

## alltools

- Binary tools for quick automation in bash or batch.
- Written in C++.

### Nothing Tool

- The nothing tool does... as you guessed, nothing.
- **Normal mode:**
  ```bash
  nothing
  ```
  - Does nothing. That's it, don't expect more.

- **Sleeping mode:**
  ```bash
  nothing -s 1000 # Sleeps for 1 second or 1000 milliseconds.
  ```
  - Sleeps while doing nothing. That's it, again.

### Lockout Tool

- Logout from a machine.
- **Normal mode:**
  ```bash
  lockout # Logout
  ```
  - Logs out of the machine using platform-specific APIs (cross-platform).

- **Sleep mode:**
  ```bash
  lockout -s 1000 # Sleep for 1 second or 1000 ms
  ```

- **Shutdown mode:**
  ```bash
  lockout -shut
  ```
  or
  ```bash
  lockout --shutdown
  ```
  - Sleeps for the specified time until shutdown/logout.

### Serve Tool

- Create a server from a machine.
- Start a server at `0.0.0.0:8080/` from the current folder:
  ```bash
  serve
  ```
  - Serves files from port 8080 from the current folder.

  ```bash
  serve --port 443 --cors --https --folder "servethething"
  ```
  - Serves from port 443.
  - Adds CORS headers.
  - Uses HTTPS/SSL.
  - Serves from the `servethething` folder.

### Cursor Tool

- Also known as `'spinner'`. Renders a cool spinner for loading tasks when the completion time is unknown.
- **Flags:**
  - `-d` or `--delay`:
    - Requires a millisecond argument.
    - Time to sleep before the next character.
  - `--cmd <command>`:
    - Required.
    - Command to run until finished.
  - `-th` or `--theme`:
    - Optional.
    - Default theme is `classic`.
    - Available themes:
      - `classic`
      - `num-wheel`
      - `ark`
      - `updot`
  - `-v` or `--verbose`:
    - Enables verbose logging.
  - `-c` or `--copyright`:
    - Shows copyright information.

### Fwipe Tool

- File wiper utility.
- **Flags:**
  - `-f` or `--file <FILE>`:
    - Required.
    - Specify file to delete.
  - `-h` or `--help`:
    - Optional.
    - Bring up the help menu.

### Dwipe Tool

- Directory wiper utility.
- **Flags:**
  - `-d` or `--dir <DIR>`:
    - Required.
    - Specify directory to delete.
  - `-h` or `--help`:
    - Optional.
    - Bring up the help menu.

## hpm

- See [hpm docs](./inner/hpm/README.md).

## bintool

- See [bintool docs](./inner/bintool/docs/index.md).
