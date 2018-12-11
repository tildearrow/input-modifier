# plugin behind-the-scenes

every plugin must at least implement 3 functions, `imodGetPluginInfo`, `imodPluginInit` and `imodPluginQuit`.
these are called by input-modifier after it has finished scanning for devices and loaded configs, and when quitting.

you can find all functions that can be implemented in `src/daemon/plugin/skeleton.h`.

## commands

plugins can add their own commands. just fill the TODO

## events

plugins can listen to some actions within the daemon.


