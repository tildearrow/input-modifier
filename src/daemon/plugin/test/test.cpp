#include "test.h"

// OK, I'll be honest. every time I write "pinfo", my finger unconsciously slips off to K,
// therefore making me wanna type "pinkfairy" instead!!!!!
// this is all 'cuz I deeply fell in love with this lady but hasn't been back in a
// very long time... yet still am thinking of her and want her to return...

// TODO: test out the dragons

PluginInfo pinfo;

IMOD_EXPORT PluginInfo* imodPluginGetInfo() {
  pinfo.name="plugin test";
  pinfo.author="tildearrow";

  pinfo.listenToEvent[plugInputEvent]=false;
  pinfo.listenToEvent[plugDevicePlugEvent]=false;
  return &pinfo;
}

IMOD_EXPORT bool imodPluginInit() {
  testExtern();
  return true;
}

IMOD_EXPORT bool imodPluginQuit() {
  return true;
}
