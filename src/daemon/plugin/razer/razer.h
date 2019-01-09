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
  DBusMessage* reply;
  unsigned int replyIndex;
  string name;
  bool errorInited;
  template<typename... A> inline void nothing(A&&...) {}

  template<typename T> bool addArg(T a) {
    imLogW("mistake: addArg with unusual type\n");
    return true;
  }
  
  public:
    bool connect();
    bool requestName(string name);
    template<typename... A> bool call(string who, string where, string what, string method, A&&... arg) {
      msg=dbus_message_new_method_call(who.c_str(),where.c_str(),what.c_str(),method.c_str());
      if (msg==NULL) {
        imLogW("message for call %s is null!\n",method.c_str());
        return false;
      }

      // TODO: arguments
      nothing(addArg(arg)...);
    
      reply=dbus_connection_send_with_reply_and_block(conn,msg,2000,&error);
      dbus_message_unref(msg);
      if (reply==NULL) {
        imLogW("reply is null.\n");
        return false;
      }
     
      imLogI("success...\n");
      dbus_message_unref(reply);
      return true;
    }
    microBus(): conn(NULL), msg(NULL), reply(NULL), replyIndex(0), name(""), errorInited(false) {}
    ~microBus();
};
