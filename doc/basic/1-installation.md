# installation (distro-specific)

## Arch Linux

install [input-modifier](https://aur.archlinux.org/packages/input-modifier/). from the AUR (either manually or with your favorite helper).

# installation (universal installer)

download the installer from [here](https://github.com/tildearrow/input-modifier/releases) (don't download the source code versions!).

after downloading input-modifier, please follow these instructions for a proper install.

## extracting

input-modifier comes in a universal binary package, besides some distribution-specific packages. this means it will work with any distribution. this is possible because input-modifier has no dependencies other than udev, which is found in almost every single modern distro.

input-modifier doesn't run as root, to allow for multi-user support in the future.

to install input-modifier, first extract the installer. and type the following in a terminal:

```
tar -xvf <RELEASE>
```

(replace <RELEASE> with the name of the input-modifier package, e.g. `imod-v1.2-x86_64.tar.gz`)

this will extract the installer to a directory named `input-modifier`.

to run the installer, type:

```
cd input-modifier
sudo ./install
```

this will install input-modifier to your system after you accept the prompts.

## input group

input-modifier opens input devices (of course), so it requires you to be in the `input` group.

you can add yourself to that group by typing the following:

```
sudo usermod -a -G input $USER
```

after this, you'll have to re-login before being able to use input-modifier, since new groups don't apply until you do this.

under special circumstances, a reboot may be necessary.

after this, you're ready to go. see [startup](2-startup.md).

## what about an AppImage?

this can't happen, because of the udev rule that has to be installed.

plus input-modifier has no dependencies, so it wouldn't make sense.
