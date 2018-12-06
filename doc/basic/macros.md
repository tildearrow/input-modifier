# macros

besides key rebinding, input-modifier is also capable of macros.

macros are sequences of timed keypresses which can be anything from pressing a simple key combo to typing text to executing a winning move/strategy.

## creating a macro

to create a macro, type:

```
> newmacro macro1
```

## recording to a macro

the easiest way to prepare a macro is to simply record to it. to do so:

```
> recordmacro keyboard macro1
```

(make sure you've created the macro first)

this will begin recording from your keyboard.

to finish recording, press ESC.

## assigning your macro

to assign your macro to a key (e.g. right control), do the following:

```
> addaction keyboard KEY_RIGHTCTRL macro macro1
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

yes, you can, but it is harder. more on that in the advanced section.

onwards to [profiles](profiles.md).
