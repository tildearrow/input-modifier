#include "razer.h"

PluginInfo pinfo;
microBus dbus;

Command(myCommand) {
  dprintf(output,"hello world!\n");
  return 0;
}

IMOD_EXPORT PluginInfo* imodPluginGetInfo() {
  pinfo.name="OpenRazer integration";
  pinfo.author="tildearrow";

  pinfo.listenToEvent[plugInputEvent]=false;
  pinfo.listenToEvent[plugDevicePlugEvent]=false;
  
  pinfo.commands.push_back(AvailCommands("hello",myCommand));
  return &pinfo;
}

IMOD_EXPORT bool imodPluginInit() {
  if (!dbus.connect()) return false;
  dbus.call("org.razer","/org/razer","razer.daemon","version",3);
  dbus.call("org.razer","/org/razer","razer.devices","getDevices");
  return true;
}

IMOD_EXPORT bool imodPluginQuit() {
  return true;
}
