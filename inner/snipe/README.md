# snipe
Snipe is a package tool to install, manage, and execute packages. It is
a decentralized package manager meaning you yourself! can create and maintain
packages.

## Commands

### init
- It initializes the package directory with the repositiry file much like pacman
- command: `snipe init`

### list
- It lists the installed packages in the ~/.snipe/packages directory
- command: `snipe list`

### For devlopers

#### Installation Format
- The directory structure of `~/.snipe/`

* ~/.snipe/
    - cache/
    - packages/
    * packrepo.lst
    * version

#### Package format
- The directory/package format for any package

* ~/.snipe/packages/package-name-[version]
    - bin/ (for executable packages)
    - include/ (for library packages)
    - lib/ (for library packages)
    - share/ (shareables like configs or fonts)
    * .deps
    * .long_description
    * setup.py
    * .short_description

* URL: https://repo.tld/package/version
* download dirname: package-name-version

##### .deps file
- content style:
```deps
https://repo1.tld/ -- package-name -- version
https://repo2.tld/ -- pkg2 -- version
```

##### How the user will use .deps
- command:`snipe install package[INSTALLOPT] --version:1.19.2`