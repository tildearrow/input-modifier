# mappings

input-modifier is also capable of having multiple "mappings" for a profile, with the ability to switch them on the fly.

## listing mappings

to see mappings for a device, type, e.g.:

```
> listmaps <device>
```

at first the only mapping available is called "Default".

## creating a mapping

to create a new mapping, type something like:

```
> newmap <device> map2
```

(`map2` being the mapping's name)

(note that mapping names can't have spaces)

## copying a mapping

if you want to copy a mapping, type something like:

```
> copymap <device> oldMap newMap
```

## deleting a mapping

if you want to get rid of a mapping, you can do so:

```
> delmap <device> map2
```

but first make sure that's not the current mapping.

## switching between mappings (using the command line)

```
> switchmap <device> map2
```

## binding a mapping to a key

```
> addaction <device> KEY_RIGHTMETA switchmap map2
```

## mapping shifting

mapping shifting is an ability of input-modifier in where you hold a key to temporarily activate a mapping.

```
> addaction <device> KEY_RIGHTMETA shiftmap map2
```

## adding actions to a mapping without changing mappings

yes, you can. as an example:

```
> addaction <device> @mapName KEY_LSHIFT macro macro1
```

(replace `@mapName` with the mapping's name preceded by an at symbol (`@`), for example, `@map2`)

the commands `listactions`, `clearactions`, `delaction` and `listbinds` also have this feature:

```
> listbinds <device> @map2
```
