# WARNING

- ***This project is still in development! Don't use in production!***
- ***I am not responsible for damages that occur if production environments are damaged!***

## Dependencies

- **devutils:**
  - [cpp-httplib](https://github.com/yhirose/cpp-httplib)
- **bintool:**
  - OpenSSL (system)
  - [LIEF](https://lief-project.github.io/) (system)

## AllForAll

- An open-source tool library for OS administrators.
- Written in C++ for blazing-fast applications and code readability, welcoming contributors.
- A massive toolkit (coming soon...).

## Build & Installation

### Supported Platforms

1. Windows
2. Linux
3. BSD

### Requirements

- Internet connection
- GCC and Make

### Steps

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

## bintool

- See [bintool docs](./inner/bintool/docs/index.md).

## devutils

- See [devutils docs](./inner/devutils//docs/index.md).
