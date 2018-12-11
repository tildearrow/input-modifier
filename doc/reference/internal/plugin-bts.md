# plugin behind-the-scenes

every plugin must at least implement 2 functions, `imodPluginInit` and `imodPluginQuit`.
these are called by input-modifier after it has finished scanning for devices and loaded configs.

you can find all functions that can be implemented in `src/daemon/plugin/skeleton.h`.

## commands

plugins can add their own commands. TODO though.

## events

TODO as well
