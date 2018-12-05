# insmacroact

adds an action to a macro.

## usage

```
insmacroact <macro> <type> ...
```

the command's arguments vary depending on the action type.
these are documented below.

## action types

### key

this is a key action.

value can be:

- 1: press
- 0: release

```
insmacroact <macro> key <key> <value>
```

### relative

this is a relative event (e.g. a mouse movement) action.

```
insmacroact <macro> rel <relEvent> <value>
```

### wait

this waits a specific amount of time before executing the next action.

```
insmacroact <macro> wait <time>
```

## example(s)

```
insmacroact macro1 key KEY_A 1
insmacroact macro1 wait 0.25
insmacroact macro1 key KEY_A 0
insmacroact macro1 key KEY_SPACE 1
insmacroact macro1 wait 0.05
insmacroact macro1 key KEY_SPACE 0
```
