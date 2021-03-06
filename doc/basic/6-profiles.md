# profiles

besides keybindings and macros, input-modifier also does profile management.

profiles are per-device, if you were wondering.

## listing profiles

to see your profiles for a device, type, e.g.:

```
> listprofiles <device>
```

at first you'll have a single profile called "Default".

## creating a profile

to create a new profile, type something like:

```
> newprofile <device> profile2
```

(`profile2` being the profile's name)

(note that profile names can't have spaces)

## switching to that profile

now to switch to that profile type:

```
> switchprofile <device> profile2
```

## copying a profile

if you have an existing profile and you want to copy it to a new one, type something like:

```
> copyprofile <device> oldProfile newProfile
```

## deleting a profile

if you want to get rid of a profile, you can do so:

```
> delprofile <device> profile2
```

but first make sure that's not the current profile.

congratulations, you have learned how to use input-modifier! but that's not all. there's more in the [advanced](../advanced/README.md) section!
