#include "razer.h"

PluginInfo pinfo;

IMOD_EXPORT PluginInfo* imodPluginGetInfo() {
  pinfo.name="OpenRazer integration";
  pinfo.author="tildearrow";

  pinfo.listenToEvent[plugInputEvent]=false;
  pinfo.listenToEvent[plugDevicePlugEvent]=false;
  return &pinfo;
}

IMOD_EXPORT bool imodPluginInit() {
  return true;
}

IMOD_EXPORT bool imodPluginQuit() {
  return true;
}
