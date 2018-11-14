# listbinds

list a device's events with assigned actions.

## usage

```
listbinds <device> [keymap]
```

if no keymap is specified, the current keymap is assumed.

## example(s)

```
> listbinds 0
- KEY_TAB
- KEY_RIGHTCTRL
Ready.
```

this lists events with actions on the current mapping.

```
> listbinds 0 Secondary
- KEY_1
- KEY_2
- KEY_3
- KEY_W
- KEY_A
- KEY_S
- KEY_D
Ready.
```

this lists events with actions on the "Secondary" mapping.
