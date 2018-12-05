# recordmacro

records events from a device to a macro.

## usage

```
recordmacro <device> <macro> [stopkey] [recordDelay] [stopAfter]
```

## notes

recordDelay and stopAfter are not yet implemented.

the stop key will be disabled during recording because when pressed it stops recording.

if no stop key is provided, the ESC key will act as stop key.

## examples

```
recordmacro keyboard macro1
```

this records to the macro "macro1".
