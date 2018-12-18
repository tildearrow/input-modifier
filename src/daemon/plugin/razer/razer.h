// OpenRazer integration plugin
#include "../../imodd.h"
#include <dbus/dbus.h>

IMOD_EXPORT PluginInfo* imodPluginGetInfo();
IMOD_EXPORT bool imodPluginInit();
IMOD_EXPORT bool imodPluginQuit();

// a very tiny binding to libdbus, designed specifically for this plugin...
class microBus {
  DBusConnection* conn;
  DBusError error;
  DBusMessage* msg;
  DBusMessageIter args;
  DBusPendingCall* pending;
  unsigned int replyIndex;
  string name;
  public:
    bool connect();
    bool requestName(string name);
    bool call(string object, ...);
};
