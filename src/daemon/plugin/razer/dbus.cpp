#include "razer.h"

#define DID_WE_FAIL_IF_SO_QUIT(ONE) \
  if (dbus_error_is_set(&error)) { \
    imLogW(ONE ": %s...\n",error.message); \
    dbus_error_free(&error); \
    return false; \
  }

#define DID_WE_FAIL(ONE) \
  if (dbus_error_is_set(&error)) { \
    imLogW(ONE ": %s...\n",error.message); \
    dbus_error_free(&error); \
  }

bool microBus::connect() {
  if (!errorInited) {
    dbus_error_init(&error);
    errorInited=true;
  }
  conn=dbus_bus_get(DBUS_BUS_SESSION,&error);
  DID_WE_FAIL_IF_SO_QUIT("couldn't connect to bus")
  if (conn==NULL) {
    imLogE("what? how did we get past this point with the bus NULL?\n");
    return false;
  }
  return true;
}

bool microBus::requestName(string name) {
  int value;
  if (conn==NULL) return false;
  value=dbus_bus_request_name(conn,name.c_str(),DBUS_NAME_FLAG_REPLACE_EXISTING,&error);
  DID_WE_FAIL_IF_SO_QUIT("couldn't request that name")
  if (value!=DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER) {
    imLogW("the requested name is not the reply primary owner, or so is how I understand it...\n");
    return false;
  }
  return true;
}

bool microBus::call(string who, string where, string what, string method, ...) {
  va_list va;
  msg=dbus_message_new_method_call(who.c_str(),where.c_str(),what.c_str(),method.c_str());
  if (msg==NULL) {
    imLogW("message for call %s is null!\n",method.c_str());
    return false;
  }

  // TODO: arguments

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


microBus::~microBus() {
  //if (conn!=NULL) dbus_connection_close(conn);
}
