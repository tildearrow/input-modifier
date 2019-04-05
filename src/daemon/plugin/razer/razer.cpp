#include "razer.h"

PluginInfo pinfo;
microBus dbus;

Command(cmdDBusTest) {
  if (args->size()<5) {
    dprintf(output,"usage: dbus-test bus path interface method\n");
    return 1;
  }
  dbus.call((*args)[1],(*args)[2],(*args)[3],(*args)[4]);
  return 0;
}

IMOD_EXPORT PluginInfo* imodPluginGetInfo() {
  pinfo.name="OpenRazer integration";
  pinfo.author="tildearrow";

  pinfo.listenToEvent[plugInputEvent]=false;
  pinfo.listenToEvent[plugDevicePlugEvent]=false;
  
  pinfo.commands.push_back(AvailCommands("dbus-test",cmdDBusTest));
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
