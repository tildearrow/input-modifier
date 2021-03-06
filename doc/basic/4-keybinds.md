# keybinds

this is probably to you the most important part of this entire documentation.

input-modifier's main purpose is to let you bind certain keys to actions.

whenever you add actions to a key, that key no longer serves its original purpose.

## remapping keys

say you want to remap the tab key to the A key.

to do that, type the following:

```
> addaction <device> KEY_TAB key KEY_A
```

(replace `<device>` with your device's name; see [devices](3-devices.md) for notes)

(if you're wondering what the key names are, see the following: [input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h))

## I am lost

if you're unsure of a key's name, you can retrieve it by doing:

```
> whichkey <device>
```

and then pressing the key in question.

## multiple keys

yes, you can.

```
> addaction <device> KEY_TAB key KEY_LEFTCTRL
> addaction <device> KEY_TAB key KEY_LEFTALT
> addaction <device> KEY_TAB key KEY_ESC
```

after doing this, you've just bound the tab key to Ctrl-Alt-Esc.

## quick triggering

say you want the side button in your mouse to do fast clicks.

```
> addaction <device> BTN_SIDE turbo BTN_LEFT 0.04 0.04
```

(replace `<device>` with your mouse's name)

the first 0.04 is the press time, and the next one is the release time.
all units in seconds.

## disabling keys

say you want to disable the Meta/Windows key. you may do so by typing the following:

```
> addaction <device> KEY_LEFTMETA disable
```

## finding out which keys are bound

this command will do:

```
> listbinds <device>
```

## I made a mistake

if you made any mistakes you can delete an action.

first find out the action index:

```
> listactions <device> KEY_TAB
```

then delete the action, e.g.:

```
> delaction <device> KEY_TAB 2
```

(`2` being the action index)

## resetting a key

you can reset a key to its default state by typing:

```
> clearactions <device> KEY_TAB
```

## the rest

there are more actions available, but they aren't listed here. once you hit the advanced usage section, you'll learn about them.

alternatively, check out the [addaction reference](../reference/command/addaction.md).

now, let's look at [macros](5-macros.md).
