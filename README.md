# input-modifier

uinput based key bindings and macros tool for Linux.

# installation

extract release **as root**:

```
sudo tar -xvf <RELEASE>.tar.gz
```

(replace \<RELEASE\> with the tarball name)

(extracting as root is necessary for the program to work correctly!)

# running

run imodd:

```
cd input-modifier/bin
./imodd
```

# usage

i'll write documentation in a few days. sorry, I don't have much time.
but here is a good start

(NOTE: for now you have to write the keycode values as per [linux/input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h))

```
./imod-cli
> listdevices
0: My Device
Ready.
> enable 0
Ready.
> addaction 0 <KEYCODE> key <NewKeycode>
Ready.
> addaction 0 <KEYCODE> turbo <NewKeycode> 0.05 0.05
Ready.
> addaction 0 <KEYCODE> execute <ProgramPath> <Arguments...>
Ready.
```

(times for turbo and relconst (which I will document later) are in seconds)

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
