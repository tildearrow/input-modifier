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
  DBusMessageIter replyArgs;
  unsigned int replyIndex;
  string name;
  bool errorInited;
  template<typename... A> inline void nothing(A&&...) {}

  template<typename T> bool addArg(T a);
  
  public:
    bool connect();
    bool requestName(string name);
    void printArg(int type, DBusBasicValue& what) {
      switch (type) {
        case DBUS_TYPE_BYTE:
          imLogI("- char %d\n",what.byt);
          break;
        case DBUS_TYPE_BOOLEAN:
          imLogI("- bool %s\n",what.bool_val?"true":"false");
          break;
        case DBUS_TYPE_INT16:
          imLogI("- short %d\n",what.i16);
          break;
        case DBUS_TYPE_UINT16:
          imLogI("- unsigned short %d\n",what.u16);
          break;
        case DBUS_TYPE_INT32:
          imLogI("- int %d\n",what.i32);
          break;
        case DBUS_TYPE_UINT32:
          imLogI("- unsigned int %d\n",what.u32);
          break;
        case DBUS_TYPE_INT64:
          imLogI("- long %lcd\n",what.i64);
          break;
        case DBUS_TYPE_UINT64:
          imLogI("- unsigned long %ld\n",what.u64);
          break;
        case DBUS_TYPE_DOUBLE:
          imLogI("- double %f\n",what.dbl);
          break;
        case DBUS_TYPE_STRING:
          imLogI("- string %s\n",what.str);
          break;
        case DBUS_TYPE_OBJECT_PATH:
          imLogI("- object %s\n",what.str);
          break;
        case DBUS_TYPE_SIGNATURE:
          imLogI("- sign %s\n",what.str);
          break;
        case DBUS_TYPE_UNIX_FD:
          imLogI("- fd %d\n",what.i32);
          break;
        default:
          imLogI("- ???\n");
          break;
      }
    }
    void processRecursion(DBusMessageIter* it) {
      DBusMessageIter anoth;
      DBusBasicValue arg;
      int argType;
      imLogI("{\n");
      do {
        // get argument
        argType=dbus_message_iter_get_arg_type(it);
        if (argType==DBUS_TYPE_ARRAY || argType==DBUS_TYPE_STRUCT || argType==DBUS_TYPE_DICT_ENTRY) {
          dbus_message_iter_recurse(it,&anoth);
          processRecursion(&anoth);
        } else {
          if (argType!=DBUS_TYPE_INVALID) {
            dbus_message_iter_get_basic(it,&arg);
            printArg(argType,arg);
          }
        }
      } while (dbus_message_iter_next(it));
      imLogI("}\n");
    }
    template<typename... A> bool call(string who, string where, string what, string method, A&&... arg) {
      msg=dbus_message_new_method_call(who.c_str(),where.c_str(),what.c_str(),method.c_str());
      if (msg==NULL) {
        imLogW("message for call %s is null!\n",method.c_str());
        return false;
      }

      dbus_message_iter_init_append(msg,&args);
      nothing(addArg(arg)...);
    
      reply=dbus_connection_send_with_reply_and_block(conn,msg,2000,&error);
      dbus_message_unref(msg);
      if (reply==NULL) {
        imLogW("reply is null.\n");
        return false;
      }
      if (dbus_message_iter_init(reply,&replyArgs)) {
        processRecursion(&replyArgs);
      } else {
        imLogD("no arguments\n");
      }
     
      imLogI("success...\n");
      dbus_message_unref(reply);
      return true;
    }
    microBus(): conn(NULL), msg(NULL), reply(NULL), replyIndex(0), name(""), errorInited(false) {}
    ~microBus();
};
