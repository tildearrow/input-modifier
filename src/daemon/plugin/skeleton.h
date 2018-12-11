// plugin skeleton.
// see doc/reference/internal/plugin-bts.md for more information.
#include "../imodd.h"

IMOD_EXPORT bool imodPluginInit();
IMOD_EXPORT bool imodPluginQuit();

IMOD_EXPORT bool imodPluginInputEvent(Device* dev, struct input_event ev);
IMOD_EXPORT bool imodPluginDeviceEvent(Device* dev, bool add);
