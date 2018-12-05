# addaction

adds an action to an event.

## usage

```
addaction <device> [@keymap] <event> <type> ...
```

if no keymap is specified, the current keymap is altered.

the command's arguments vary depending on the action type.
these are documented below.

## action types

### key

this is a simple keypress action. useful for rebinding.

```
addaction <device> [@keymap] <event> key <key>
```

### turbo

this action turns a key into a "retrigger"-like button.

```
addaction <device> [@keymap] <event> turbo <key> <timeOn> <timeOff>
```

the specified key will be fired in the following pattern:

```
ON > (timeOn seconds) > OFF > (timeOff seconds) > ON > (timeOn seconds) > OFF > (timeOff seconds) > ...
```

until the mapped key is released.

### relative

this maps a key to a relative event (e.g. a mouse movement).

```
addaction <device> [@keymap] <event> rel <relEvent> <value>
```

when the mapped key is pressed, the relative event fires once with a specified value.

### relative constant

this makes a key trigger a specific relative event constantly until it is released.

this can be useful for moving the cursor with a keyboard, as an example.

```
addaction <device> [@keymap] <event> relconst <relEvent> <value> <delay>
```

`delay` specifies the time between relative events.

### absolute

this maps a key to an absolute event.

this is not well done at the moment, but eventually it may be used for creating a virtual joystick.

```
addaction <device> [@keymap] <event> abs <absEvent> <value>
```

### execute

this action executes a program upon a keypress.

```
addaction <device> [@keymap] <event> execute <command> [args ...]
```

NOTE: `command` in this case is a full path, so don't write `gaming-app`, but instead something like `/usr/bin/gaming-app`.

### switch map

this action switches to a different mapping.

```
addaction <device> [@keymap] <event> switchmap <map>
```

### shift map

this action switches to a different mapping until the mapped key is released.

```
addaction <device> [@keymap] <event> shiftmap <map>
```

### macro

this action triggers a macro.

```
addaction <device> [@keymap] <event> macro <name>
```

## example(s)

```
addaction keyboard KEY_UP key KEY_W
addaction keyboard KEY_LEFT key KEY_A
addaction keyboard KEY_DOWN key KEY_S
addaction keyboard KEY_RIGHT key KEY_D
```

this turns the arrow keys into WASD ones.

```
addaction keypad KEY_TAB turbo BTN_LEFT 0.03 0.03
```

this maps the tab key to do very fast left clicks.

```
addaction keyboard KEY_RIGHTSHIFT execute /usr/bin/konsole
```

this opens a terminal on pressing the right shift key.

```
newmap Secondary
addaction keyboard @Secondary KEY_1 key BTN_LEFT
addaction keyboard @Secondary KEY_2 key BTN_MIDDLE
addaction keyboard @Secondary KEY_3 key BTN_RIGHT
addaction keyboard @Secondary KEY_W relconst REL_Y -6 0.02
addaction keyboard @Secondary KEY_A relconst REL_X -6 0.02
addaction keyboard @Secondary KEY_S relconst REL_Y 6 0.02
addaction keyboard @Secondary KEY_D relconst REL_X 6 0.02
addaction keyboard KEY_RIGHTCTRL shiftmap Secondary
```

this makes the right control key turn the left hand side into a mouse as long as it is pressed.
