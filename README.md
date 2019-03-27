# input-modifier

uinput based key bindings and macros tool for Linux.

# installation

## Arch Linux

[AUR](https://aur.archlinux.org/packages/input-modifier/).

## universal installer

download the installer from [here](https://github.com/tildearrow/input-modifier/releases) (select the first element from "Assets").

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

and then re-login.

# running

run imodd:

```
imodd
```

# usage

documentation can be found [here](doc/README.md).

# to-do

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
