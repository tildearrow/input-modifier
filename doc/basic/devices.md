# devices

after you've started the input-modifier daemon, you might be wondering how to control the program.

this is done by using a little utility called `imod-cli`, which is a command-line interface to input-modifier.

(don't worry, a GUI will come at some point)

to open up the utility, type the following in a terminal:

```
imod-cli
```

if you've done this correctly, you should be at the input-modifier prompt:

```
>
```

now you can begin typing commands.

## but what is better for me to do first?

to list your devices. to do so, type the following at the prompt:

```
> listdevices
```

this will return something like:

```
0: Keyboard
1: Mouse
Ready.
```

the numbers at the left side are device indexes, which may change at any time (and as such discouraged to use in furthercoming commands).

the names at the right side are your device's names, which you can use in the next commands.

## enabling devices

these devices are in the "disabled" state when they are first detected by input-modifier.
this means keybindings, macros, or the like won't work.

to enable a device, type the following:

```
> enable <device>
```

where `<device>` is your device's name. note that you don't have to type the full name, for convenience.

as an example:

```
> enable Keyboard
```

enables the device "Keyboard". you may also type:

```
> enable key
```

and it will have an equivalent effect.

to disable a device, in case you don't want it to do keybindings anymore, do:

```
> disable <device>
```

for example:

```
> disable keyboard
```

following is [keybinds](keybinds.md). the moment you've been waiting for.

## device name notes

if your device's name contains spaces, then typing the device's name with spaces won't work.

however, you may still type only a portion of its name. as an example, if your device's called "Razer Ornata Chroma", you only have to type "ornata" or similar.

```
> enable ornata
```
