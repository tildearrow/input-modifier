# macros

besides key rebinding, input-modifier is also capable of macros.

macros are sequences of timed keypresses which can be anything from pressing a simple key combo to typing text to executing a winning move/strategy.

## creating a macro

to create a macro, type:

```
> newmacro macro1
```

(note that macro names can't have spaces)

## recording to a macro

the easiest way to prepare a macro is to simply record to it. to do so:

```
> recordmacro <device> macro1
```

(make sure you've created the macro first)

this will begin recording from your `<device>`. this means any keypresses in other devices won't be registered.

to finish recording, press ESC.

## assigning your macro

to assign your macro to a key (e.g. right control), do the following:

```
> addaction <device> KEY_RIGHTCTRL macro macro1
```

## I made a mistake

don't worry. you can clear your macro by doing:

```
> clearmacro macro1
```

alternatively, you can delete the macro:

```
> delmacro macro1
```

## showing your macro

to see your macro's actions:

```
> showmacro macro1
```

## listing available macros

to see your macros, type the following:

```
> listmacros
```

## why not manually create the macro?

yes, you can.

it is much harder to do, but it is possible.

to insert a keypress, type e.g.:

```
> insmacroact macro1 key KEY_A 1
```

to insert a key release, type e.g.:

```
> insmacroact macro1 key KEY_A 0
```

to insert a delay, type e.g.:

```
> insmacroact macro1 wait 0.2
```

(time in seconds)

if you make a mistake, you can type:

```
> showmacro macro1
```

then look at the action index you want to delete, and then type:

```
> delmacroact macro1 5
```

(the ability to edit actions in-place is coming soon)

next page is [profiles](profiles.md).
