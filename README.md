# input-modifier

uinput based key bindings and macros tool for Linux.

# installation

extract release:

```
tar -xvf <RELEASE>.tar.gz
```

run the installer:

```
cd input-modifier
sudo ./install
```

add yourself to the `input` group if not done previously:

```
sudo usermod -a -G input $USER
```

# running

run imodd:

```
imodd
```

# usage

documentation can be found [here](doc/README.md).

# to-do

this is pretty much unusable in a *real crazy* environment as of now, but otherwise it works.

- X11 integration (per-application profile bind, warping cursor and the like).

- bug fixes.

- proper packages.

- passing environment variables to execute.

- changing the weird devices setting.
  - I will talk about this later

- ignoring non-input devices such as the Firefly or Polaris.

- fix pressing non-bit-set keys.

- implement binds for relative and switch events.

- add feature here.
