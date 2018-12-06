# devices

after you've started the input-modifier daemon, you might be wondering how to control the program.

this is done by using a little utility called `imod-cli`, which is a command-line interface to input-modifier.

(don't worry, a GUI will come at some point)

to open up the utility, open a terminal, go to the input-modifier/bin directory and type the following:

```
./imod-cli
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

you can use your devices' names or indexes later on.

## enabling devices

these devices are in the "disabled" state when they are first detected by input-modifier.
this means keybindings, macros, or the like won't work.

to enable a device, type the following:

```
> enable keyboard
```

that will enable the device "Keyboard". note that you don't have to type the full name, for convenience.

to disable a device, in case you don't want it to do keybindings anymore, do:

```
> disable keyboard
```

following is [keybinds](keybinds.md). the moment you've been waiting for.
