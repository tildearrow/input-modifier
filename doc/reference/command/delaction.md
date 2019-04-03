# delaction

deletes an action from an event.

## usage

```
delaction <device> [@keymap] <event> <index>
```

if no keymap is specified, the current keymap is altered.

## examples

```
delaction keyboard KEY_TAB 0
```

this removes the first action from the tab key.
