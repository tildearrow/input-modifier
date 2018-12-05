# installation

after downloading input-modifier, please follow these instructions for a proper install.

## extracting

input-modifier comes in a universal binary package. this means it will work with any distribution. this is possible because input-modifier has no dependencies.

input-modifier doesn't run as root, to allow for multi-user support in the future.

however, you can't use your favorite archiver as usual, since it requires being extracted as root because it uses a helper tool that sets permissions for uinput.

to install input-modifier, open a terminal, and type the following:

```
sudo tar -xvf <RELEASE>
```

(replace <RELEASE> with the name of the input-modifier package, e.g. `imod-v1.2-x86_64.tar.gz`)

this will extract the program to a directory named `input-modifier`.

## input group

input-modifier opens input devices (of course), so it requires you to be in the `input` group.

you can add yourself to that group by typing the following:

```
sudo usermod -a -G input $USER
```

after this, you're ready to go. see [startup](startup.md).

## why not a package?

because I don't have time to publish a package for every single major distribution, or some requirements forbid me from doing so.

but I surely will do so in the near future.

## what about an AppImage?

this can't happen, because of the uinput helper tool that requires to be extracted as root.

plus input-modifier has no dependencies, so it wouldn't make sense.
