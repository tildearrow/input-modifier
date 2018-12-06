# keybinds

this is probably to you the most important part of this entire documentation.

## remapping keys

say you want to remap the tab key to the A key.

to do that, type the following:

```
> addaction keyboard KEY_TAB key KEY_A
```

(if you're wondering what are the key names, see the following: [input-event-codes.h](https://github.com/torvalds/linux/blob/master/include/uapi/linux/input-event-codes.h))

## multiple keys

yes, you can.

```
> addaction keyboard KEY_TAB key KEY_LEFTCTRL
> addaction keyboard KEY_TAB key KEY_LEFTALT
> addaction keyboard KEY_TAB key KEY_ESC
```

after doing this, you've just bound the tab key to Ctrl-Alt-Esc

## quick triggering

say you want the side button in your mouse to do fast clicks.

```
> addaction mouse BTN_SIDE turbo BTN_LEFT 0.04 0.04
```

the first 0.04 is the press time, and the next one is the release time.
all units in seconds.

## disabling keys

say you want to disable the Meta/Windows key. you may do so by typing the following:

```
> addaction keyboard KEY_LEFTMETA disable
```

## TODO
