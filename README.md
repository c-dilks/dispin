# dispin
dihadron spin asymmetry analysis

# Dependencies

## coatjava
- if on ifarm, you have access to `coatjava` via `module load`; see
  documentation on CLAS12 environment setup for details
- if local, you can obtain `coatjava` by downloading a release tarball from the
  `clas12-offline-software` repository, then setup your environment:
  ```
  export COATJAVA="/path/to/coatjava"
  export CLARA_SERVICES="${COATJAVA}/lib/services"
  export DISTRO_DIR="$COATJAVA"
  export CLAS12DIR="$COATJAVA"
  export DATAMINING="$COATJAVA"
  export PATH="${PATH}:${COATJAVA}/bin"
  export CLASSPATH="${CLASSPATH}:${COATJAVA}/lib/clas/*"
  ```

## j2root
- if you clone this repository with the option `--recurse-submodules`, it will
  clone a copy of [`j2root`](https://github.com/drewkenjo/j2root) to the
  directory `deps/j2root`
- `cd` to this directory and follow the `README.md` within
- this repository provides the ability to write data to `root` files with
  groovy
- notes:
  - added `export JAVA_HOME="/usr/lib/jvm/default-java"` to `~/.zshrc`
  - troubleshooting:
    - scons unable to find `JavaH`; solution: roll back to java JDK 1.8.0
      - this is because javah is not in 1.11.0 (`openjdk-11-jdk`), as it was
        removed in a previous version
      - install `openjdk-8-jdk`, and optionally uninstall `openjdk-11-jdk`
      - then ran `sudo update-java-alternatives --list` to find paths to JDK,
        followed by `sudo update-java-alternatives --set /path/to/java-1.8.0`
      - fix symlink `usr/lib/jvm/default-java` to point to 1.8.0

## clasqaDB
- if you clone this repository with the option `--recurse-submodules`, it will
  clone a copy of [`clasqaDB`](https://github.com/c-dilks/clasqaDB) to the
  directory `deps/clasqaDB`
- this repository contains the file quality assurance (QA) database, and
  uses the groovy accessors to query information in an event loop


# Usage
- first call `source env.sh`
