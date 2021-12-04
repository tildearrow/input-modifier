# input-modifier

uinput based key bindings and macros tool for Linux.

The purpose of this project is to allow the user to assign actions to keys, which can come in handy if you want to do repetitive actions easily (e.g. tap a key repeatedly by simply holding it or launch a sequence of keys with a single keypress), launch applications using keys, disabling keys you don't want to be pressed by accident, mapping gaming keypads, etc.

**Comparison with Xdotool**

The advantages over [Xdotool](https://github.com/jordansissel/xdotool) are:

- Interacts with the evdev input layer, which is lower-level than X.
- Works outside X, e.g. VT or Wayland.

The disadvantages are:

- You can't directly map a key to closing a window, or something that has to do with X (however, you can execute xdotool from input-modifier).

**Comparison with AutoKey**

The advantages over [AutoKey](https://github.com/autokey/autokey) are: (although I've never used it)

- Is written in C++, which means less overhead than Python.
- Allows for doing things like remapping keys (besides executing applications and running macros) without resorting to complex hacks.

The disadvantages are:

- No GUI yet.
- Does not allow for DE-specific actions or X11-specific ones (but I am pretty sure it may be possible to use both programs in tandem).


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
