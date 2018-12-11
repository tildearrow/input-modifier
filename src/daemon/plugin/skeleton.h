// plugin skeleton.
// see doc/reference/internal/plugin-bts.md for more information.
#include "../imodd.h"

IMOD_EXPORT PluginInfo* imodPluginGetInfo();
IMOD_EXPORT bool imodPluginInit();
IMOD_EXPORT bool imodPluginQuit();

IMOD_EXPORT bool imodPluginInputEvent(Device* dev, struct input_event ev);
IMOD_EXPORT bool imodPluginDevicePlugEvent(Device* dev, bool add);
IMOD_EXPORT bool imodPluginChangeProfileEvent(Device* dev, string name);
// TODO: settings struct!
IMOD_EXPORT bool imodPluginChangeKeymapEvent(Device* dev, string name, void* settings);
IMOD_EXPORT bool imodPluginActionEvent(Device* dev, std::vector<Action>& actions);
