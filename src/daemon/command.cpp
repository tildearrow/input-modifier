#include "imodd.h"

extern std::vector<Device*> dev;

#define IndexedCommand \
  size_t index; \
  if (dev.empty()) { \
    dprintf(output,"error: no devices.\n"); \
    return 0; \
  } \
  index=stoul((*args)[1]); \
  if (index>=dev.size()) { \
    dprintf(output,"error: device index out of range (0-%lu).\n",dev.size()-1); \
    return 0; \
  } \

Command(cmd_newmap) {
  if (args->size()<3) {
    dprintf(output,"usage: newmap <device> <name>\n");
    return 0;
  }
  
  IndexedCommand
  
  if (dev[index]->findMap((*args)[2])>=0) {
    dprintf(output,"error: a map with the same name already exists.\n");
    return 0;
  }
  dev[index]->newMap((*args)[2]);
  return 1;
}

Command(cmd_listmaps) {
  if (args->size()<2) {
    dprintf(output,"usage: listmaps <device>\n");
    return 0;
  }
  
  IndexedCommand
  
  for (size_t i=0; i<dev[index]->mappings.size(); i++) {
    dprintf(output,"%zu: %s\n",i,dev[index]->mappings[i]->name.c_str());
  }
  return 1;
}

Command(cmd_addaction) {
  int eventArg;
  int eventVal, intVal, intVal1;
  struct timespec onTime, offTime;
  std::vector<string> argVec;
  std::vector<string> envVec;
  string mapName;
  bindSet* mapOp;
  if (args->size()<4) {
    dprintf(output,"usage: addaction <device> [@keymap] <event> <type> ...\n");
    dprintf(output,"action types:\n");
    dprintf(output,"- disable\n"
                   "- key <key>\n"
                   "- turbo <key> <timeOn> <timeOff>\n"
                   "- rel <relEvent> <value>\n"
                   "- relconst <relEvent> <value> <delay>\n"
                   "- abs <absEvent> <value>\n"
                   "- execute <command> [args ...]\n"
                   "- switchmap <map>\n"
                   "- shiftmap <map>\n");
    return 0;
  }
  
  IndexedCommand
  
  if ((*args)[2][0]=='@') {
    eventArg=3;
  } else {
    eventArg=2;
  }
  
  if (eventArg==3) {
    if (args->size()<5) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> <type> ...\n");
      dprintf(output,"action types:\n");
      dprintf(output,"- disable\n"
                     "- key <key>\n"
                     "- turbo <key> <timeOn> <timeOff>\n"
                     "- rel <relEvent> <value>\n"
                     "- relconst <relEvent> <value> <delay>\n"
                     "- abs <absEvent> <value>\n"
                     "- execute <command> [args ...]\n"
                     "- switchmap <map>\n"
                     "- shiftmap <map>\n");
      return 0;
    }
  }
  
  if (eventArg==3) {
    int whereToOperate=dev[index]->findMap(S((*args)[2].c_str()+1));
    if (whereToOperate<0) {
      dprintf(output,"error: map not found.\n");
      return 0;
    }
    mapOp=dev[index]->mappings[whereToOperate];
  } else {
    mapOp=dev[index]->curmap;
  }
  
  if (mapOp==NULL) {
    dprintf(output,"error: no current map.\n");
    return 0;
  }
  
  eventVal=stoi((*args)[eventArg]);
  if (eventVal<0 || eventVal>KEY_CNT) {
    dprintf(output,"error: event number out of range.\n");
    return 0;
  }
  
  if ((*args)[eventArg+1]==S("disable")) {
    // disable (args: 0)
    mapOp->keybinds[eventVal].doModify=true;
    mapOp->keybinds[eventVal].actions.push_back(Action(actDisable,0));
  } else if ((*args)[eventArg+1]==S("key")) {
    // key (args: 1)
    if (args->size()<2+eventArg+1) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> key <key>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    intVal=stoi((*args)[eventArg+2]);
    if (intVal<0 || intVal>KEY_CNT) {
      dprintf(output,"error: keycode out of range.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actKey,intVal));
  } else if ((*args)[eventArg+1]==S("turbo")) {
    // turbo (args: 3)
    if (args->size()<2+eventArg+3) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> turbo <key> <timeOn> <timeOff>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    onTime=stots((*args)[eventArg+3]);
    if (onTime<mkts(0,0)) {
      dprintf(output,"error: on time can't be negative.\n");
      return 0;
    }
    offTime=stots((*args)[eventArg+4]);
    if (offTime<mkts(0,0)) {
      dprintf(output,"error: off time can't be negative.\n");
      return 0;
    }
    intVal=stoi((*args)[eventArg+2]);
    if (intVal<0 || intVal>KEY_CNT) {
      dprintf(output,"error: keycode out of range.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actTurbo,intVal,onTime,offTime));
  } else if ((*args)[eventArg+1]==S("rel")) {
    // relative (args: 2)
    if (args->size()<2+eventArg+2) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> rel <relEvent> <value>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    intVal=stoi((*args)[eventArg+2]);
    if (intVal<0 || intVal>REL_CNT) {
      dprintf(output,"error: relative event code out of range.\n");
      return 0;
    }
    intVal1=stoi((*args)[eventArg+3]);
    mapOp->keybinds[eventVal].actions.push_back(Action(actRel,intVal,intVal1));
  } else if ((*args)[eventArg+1]==S("relconst")) {
    // relative constant (args: 3)
    if (args->size()<2+eventArg+3) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> relconst <relEvent> <value> <delay>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    intVal=stoi((*args)[eventArg+2]);
    if (intVal<0 || intVal>REL_CNT) {
      dprintf(output,"error: relative event code out of range.\n");
      return 0;
    }
    intVal1=stoi((*args)[eventArg+3]);
    onTime=stots((*args)[eventArg+4]);
    if (onTime<mkts(0,0)) {
      dprintf(output,"error: time can't be negative.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actRel,intVal,intVal1,onTime));
  } else if ((*args)[eventArg+1]==S("abs")) {
    // absolute (args: 2)
    if (args->size()<2+eventArg+2) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> abs <absEvent> <value>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    intVal=stoi((*args)[eventArg+2]);
    if (intVal<0 || intVal>ABS_CNT) {
      dprintf(output,"error: absolute event code out of range.\n");
      return 0;
    }
    intVal1=stoi((*args)[eventArg+3]);
    mapOp->keybinds[eventVal].actions.push_back(Action(actAbs,intVal,intVal1));
  } else if ((*args)[eventArg+1]==S("execute")) {
    // execute (args: at least 1)
    if (args->size()<2+eventArg+1) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> execute <command> [args ...]\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    for (size_t i=eventArg+3; i<args->size(); i++) {
      argVec.push_back((*args)[i]);
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actExecute,(*args)[eventArg+2],argVec,envVec));
  } else if ((*args)[eventArg+1]==S("switchmap")) {
    // switch map (args: 1)
    if (args->size()<2+eventArg+1) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> switchmap <map>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    mapName=(*args)[eventArg+2];
    if (dev[index]->findMap(mapName)<0) {
      dprintf(output,"error: binding map not found.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actSwitchMap,mapName));
  } else if ((*args)[eventArg+1]==S("shiftmap")) {
    // shift map (args: 1)
    if (args->size()<2+eventArg+1) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> shiftmap <map>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    mapName=(*args)[eventArg+2];
    if (dev[index]->findMap(mapName)<0) {
      dprintf(output,"error: binding map not found.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actShiftMap,mapName));
  } else {
    dprintf(output,"error: that bind type does not exist.\n");
    return 0;
  }
  
  return 1;
}

Command(cmd_listdevices) {
  for (size_t i=0; i<dev.size(); i++) {
    dprintf(output,"%zu: %s\n",i,dev[i]->getName().c_str());
  }
  return 1;
}

const AvailCommands cmds[]={
  {"addaction", cmd_addaction},/*
  {"delaction", cmd_delaction},
  {"clearactions", cmd_clearactions},
  {"copyactions", cmd_copyactions},
  {"listactions", cmd_listactions},
  {"listbinds", cmd_listbinds},*/
  {"listmaps", cmd_listmaps},
  {"listdevices", cmd_listdevices},/*
  {"enable", cmd_enable},
  {"disable", cmd_disable},
  {"showsettings", cmd_showsettings},
  {"settings", cmd_settings},*/
  {"newmap", cmd_newmap},/*
  {"copymap", cmd_copymap},
  {"delmap", cmd_delmap},
  {"switchmap", cmd_switchmap},
  {"newmacro", cmd_newmacro},
  {"copymacro", cmd_copymacro},
  {"delmacro", cmd_delmacro},
  {"listmacros", cmd_listmacros},
  {"recordmacro", cmd_recordmacro},
  {"playmacro", cmd_playmacro},
  {"newprofile", cmd_newprofile},
  {"copyprofile", cmd_copyprofile},
  {"delprofile", cmd_delprofile},
  {"listprofiles", cmd_listprofiles},
  {"setprofile", cmd_setprofile},
  {"switchprofile", cmd_switchprofile},
  {"version", cmd_version},
*/
  {NULL, NULL}
};
