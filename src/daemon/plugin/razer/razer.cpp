#include "razer.h"

PluginInfo pinfo;
microBus dbus;

IMOD_EXPORT PluginInfo* imodPluginGetInfo() {
  pinfo.name="OpenRazer integration";
  pinfo.author="tildearrow";

  pinfo.listenToEvent[plugInputEvent]=false;
  pinfo.listenToEvent[plugDevicePlugEvent]=false;
  return &pinfo;
}

IMOD_EXPORT bool imodPluginInit() {
  if (!dbus.connect()) return false;
  dbus.call("org.razer","/org/razer","razer.daemon","version",3);
  return true;
}

IMOD_EXPORT bool imodPluginQuit() {
  return true;
}
