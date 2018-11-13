#include "imodd.h"

extern std::vector<Device*> dev;

#define IndexedCommand \
  size_t index; \
  if (dev.empty()) { \
    dprintf(output,"error: no devices.\n"); \
    return 0; \
  } \
  try { \
    index=stoul((*args)[1]); \
  } catch (std::exception& err) { \
    dprintf(output,"error: invalid number.\n"); \
    return 0; \
  } \
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

Command(cmd_copymap) {
  if (args->size()<3) {
    dprintf(output,"usage: copymap <device> <source> <newname>\n");
    return 0;
  }
  
  IndexedCommand
  
  if (dev[index]->findMap((*args)[2])<0) {
    dprintf(output,"error: source map does not exist.\n");
    return 0;
  }
  
  if (dev[index]->findMap((*args)[3])>=0) {
    dprintf(output,"error: a map with the same destination name already exists.\n");
    return 0;
  }
  
  dev[index]->copyMap((*args)[2],(*args)[3]);
  return 1;
}

Command(cmd_delmap) {
  if (args->size()<3) {
    dprintf(output,"usage: delmap <device> <name>\n");
    return 0;
  }
  
  IndexedCommand
  
  if (dev[index]->findMap((*args)[2])<0) {
    dprintf(output,"error: map does not exist.\n");
    return 0;
  }
  
  if (dev[index]->mappings.size()<=1) {
    dprintf(output,"error: there must be at least 1 mapping.\n");
    return 0;
  }
  
  dev[index]->delMap((*args)[2]);
  return 1;
}

Command(cmd_switchmap) {
  if (args->size()<3) {
    dprintf(output,"usage: switchmap <device> <name>\n");
    return 0;
  }
  
  IndexedCommand
  
  if (dev[index]->findMap((*args)[2])<0) {
    dprintf(output,"error: map does not exist.\n");
    return 0;
  }
  
  // I think
  dev[index]->curmap=dev[index]->mappings[dev[index]->findMap((*args)[2])];
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
  
  try {
    eventVal=stoi((*args)[eventArg]);
  } catch (std::exception& err) {
    dprintf(output,"error: invalid event number.\n");
    return 0;
  }
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
    if (args->size()<2+(size_t)eventArg+1) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> key <key>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    try {
      intVal=stoi((*args)[eventArg+2]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid key code.\n");
      return 0;
    }
    if (intVal<0 || intVal>KEY_CNT) {
      dprintf(output,"error: keycode out of range.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actKey,intVal));
  } else if ((*args)[eventArg+1]==S("turbo")) {
    // turbo (args: 3)
    if (args->size()<2+(size_t)eventArg+3) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> turbo <key> <timeOn> <timeOff>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    try {
      onTime=stots((*args)[eventArg+3]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid on time.\n");
      return 0;
    }
    if (onTime<mkts(0,0)) {
      dprintf(output,"error: on time can't be negative.\n");
      return 0;
    }
    try {
      offTime=stots((*args)[eventArg+4]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid off time.\n");
      return 0;
    }
    if (offTime<mkts(0,0)) {
      dprintf(output,"error: off time can't be negative.\n");
      return 0;
    }
    try {
      intVal=stoi((*args)[eventArg+2]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid key code.\n");
      return 0;
    }
    if (intVal<0 || intVal>KEY_CNT) {
      dprintf(output,"error: keycode out of range.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actTurbo,intVal,onTime,offTime));
  } else if ((*args)[eventArg+1]==S("rel")) {
    // relative (args: 2)
    if (args->size()<2+(size_t)eventArg+2) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> rel <relEvent> <value>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    try {
      intVal=stoi((*args)[eventArg+2]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid relative code.\n");
      return 0;
    }
    if (intVal<0 || intVal>REL_CNT) {
      dprintf(output,"error: relative event code out of range.\n");
      return 0;
    }
    try {
      intVal1=stoi((*args)[eventArg+3]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid value.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actRel,intVal,intVal1));
  } else if ((*args)[eventArg+1]==S("relconst")) {
    // relative constant (args: 3)
    if (args->size()<2+(size_t)eventArg+3) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> relconst <relEvent> <value> <delay>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    try {
      intVal=stoi((*args)[eventArg+2]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid relative code.\n");
      return 0;
    }
    if (intVal<0 || intVal>REL_CNT) {
      dprintf(output,"error: relative event code out of range.\n");
      return 0;
    }
    try {
      intVal1=stoi((*args)[eventArg+3]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid value.\n");
      return 0;
    }
    try {
      onTime=stots((*args)[eventArg+4]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid delay time.\n");
      return 0;
    }
    if (onTime<mkts(0,0)) {
      dprintf(output,"error: time can't be negative.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actRel,intVal,intVal1,onTime));
  } else if ((*args)[eventArg+1]==S("abs")) {
    // absolute (args: 2)
    if (args->size()<2+(size_t)eventArg+2) {
      dprintf(output,"usage: addaction <device> <@keymap> <event> abs <absEvent> <value>\n");
      return 0;
    }
    mapOp->keybinds[eventVal].doModify=true;
    try {
      intVal=stoi((*args)[eventArg+2]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid absolute code.\n");
      return 0;
    }
    if (intVal<0 || intVal>ABS_CNT) {
      dprintf(output,"error: absolute event code out of range.\n");
      return 0;
    }
    try {
      intVal1=stoi((*args)[eventArg+3]);
    } catch (std::exception& err) {
      dprintf(output,"error: invalid value.\n");
      return 0;
    }
    mapOp->keybinds[eventVal].actions.push_back(Action(actAbs,intVal,intVal1));
  } else if ((*args)[eventArg+1]==S("execute")) {
    // execute (args: at least 1)
    if (args->size()<2+(size_t)eventArg+1) {
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
    if (args->size()<2+(size_t)eventArg+1) {
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
    if (args->size()<2+(size_t)eventArg+1) {
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

Command(cmd_delaction) {
  int eventArg, eventVal, actionVal;
  bindSet* mapOp;
  if (args->size()<4) {
    dprintf(output,"usage: delaction <device> [@keymap] <event> <index>\n");
    return 0;
  }

  IndexedCommand

  if ((*args)[2][0]=='@') {
    eventArg=3;
  } else {
    eventArg=2;
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
 
  try { 
    eventVal=stoi((*args)[eventArg]);
  } catch (std::exception& err) {
    dprintf(output,"error: invalid event number.\n");
    return 0;
  }
  if (eventVal<0 || eventVal>KEY_CNT) {
    dprintf(output,"error: event number out of range.\n");
    return 0;
  }

  if (!mapOp->keybinds[eventVal].doModify) {
    dprintf(output,"error: this key is not bound.\n");
    return 0;
  }

  try {
    actionVal=stoi((*args)[eventArg+1]);
  } catch (std::exception& err) {
    dprintf(output,"error: invalid action index.\n");
    return 0;
  }
  if (actionVal<0 || (size_t)actionVal>=mapOp->keybinds[eventVal].actions.size()) {
    dprintf(output,"error: action index out of range (0-%lu).\n",mapOp->keybinds[eventVal].actions.size()-1);
    return 0;
  }

  mapOp->keybinds[eventVal].actions.erase(mapOp->keybinds[eventVal].actions.begin()+actionVal);
  if (mapOp->keybinds[eventVal].actions.empty()) {
    mapOp->keybinds[eventVal].doModify=false;
  }
  return 1;
}

Command(cmd_clearactions) {
  int eventArg, eventVal;
  bindSet* mapOp;
  if (args->size()<3) {
    dprintf(output,"usage: clearactions <device> [@keymap] <event>\n");
    return 0;
  }

  IndexedCommand

  if ((*args)[2][0]=='@') {
    eventArg=3;
  } else {
    eventArg=2;
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
  
  try {
    eventVal=stoi((*args)[eventArg]);
  } catch (std::exception& err) {
    dprintf(output,"error: invalid event number.\n");
    return 0;
  }
  if (eventVal<0 || eventVal>KEY_CNT) {
    dprintf(output,"error: event number out of range.\n");
    return 0;
  }

  if (!mapOp->keybinds[eventVal].doModify) {
    dprintf(output,"error: this key is not bound.\n");
    return 0;
  }

  mapOp->keybinds[eventVal].actions.clear();
  mapOp->keybinds[eventVal].doModify=false;
  return 1;
}

Command(cmd_copyactions) {
  int eventArg, eventVal, destVal;
  bindSet* mapOp;
  if (args->size()<3) {
    dprintf(output,"usage: copyactions <device> [@keymap] <event> <destination>\n");
    return 0;
  }

  IndexedCommand

  if ((*args)[2][0]=='@') {
    eventArg=3;
  } else {
    eventArg=2;
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
 
  try { 
    eventVal=stoi((*args)[eventArg]);
  } catch (std::exception& err) {
    dprintf(output,"error: invalid event number.\n");
    return 0;
  }
  if (eventVal<0 || eventVal>KEY_CNT) {
    dprintf(output,"error: event number out of range.\n");
    return 0;
  }
 
  try {
    destVal=stoi((*args)[eventArg+1]);
  } catch (std::exception& err) {
    dprintf(output,"error: invalid destination event number.\n");
    return 0;
  }
  if (destVal<0 || destVal>KEY_CNT) {
    dprintf(output,"error: destination event number out of range.\n");
    return 0;
  }

  if (eventVal==destVal) {
    dprintf(output,"error: source and destination cannot be equal.\n");
    return 0;
  }
  
  if (!mapOp->keybinds[eventVal].doModify) {
    dprintf(output,"error: source key is not bound.\n");
    return 0;
  }

  mapOp->keybinds[destVal].doModify=true;
  mapOp->keybinds[destVal].actions=mapOp->keybinds[eventVal].actions;
  return 1;
}

Command(cmd_listactions) {
  int eventArg, eventVal, ic;
  bindSet* mapOp;
  if (args->size()<3) {
    dprintf(output,"usage: listactions <device> [@keymap] <event>\n");
    return 0;
  }
  
  IndexedCommand

  if ((*args)[2][0]=='@') {
    eventArg=3;
  } else {
    eventArg=2;
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
  
  try {
    eventVal=stoi((*args)[eventArg]);
  } catch (std::exception& err) {
    dprintf(output,"error: invalid event number.\n");
    return 0;
  }
  if (eventVal<0 || eventVal>KEY_CNT) {
    dprintf(output,"error: event number out of range.\n");
    return 0;
  }

  if (!mapOp->keybinds[eventVal].doModify) {
    dprintf(output,"error: this key is not bound.\n");
    return 0;
  }

  ic=0;
  for (auto& i: mapOp->keybinds[eventVal].actions) {
    switch (i.type) {
      case actKey:
        dprintf(output,"%d: key: %s\n",ic,keynames[i.code]);
        break;
      case actTurbo:
        dprintf(output,"%d: turbo: %s (%ss on, %ss off)\n",ic,keynames[i.code],tstos(i.timeOn).c_str(),tstos(i.timeOff).c_str());
        break;
      case actRel:
        dprintf(output,"%d: relative: %s (%d)\n",ic,relnames[i.code],i.value);
        break;
      case actRelConst:
        dprintf(output,"%d: relconst: %s (%d, delay %ss)\n",ic,relnames[i.code],i.value,tstos(i.timeOn).c_str());
        break;
      case actAbs:
        dprintf(output,"%d: absolute: %s (%d)\n",ic,absnames[i.code],i.value);
        break;
      case actExecute:
        dprintf(output,"%d: execute: %s\n",ic,i.command.c_str());
        for (auto& j: i.args) {
          dprintf(output,"- %s\n",j.c_str());
        }
        break;
      case actSwitchMap:
        dprintf(output,"%d: switchmap: %s\n",ic,i.command.c_str());
        break;
      case actShiftMap:
        dprintf(output,"%d: shiftmap: %s\n",ic,i.command.c_str());
        break;
      case actDisable:
        dprintf(output,"%d: disable\n",ic);
        break;
    }
    ic++;
  }

  return 1;
}

Command(cmd_listbinds) {
  bindSet* mapOp;
  if (args->size()<2) {
    dprintf(output,"usage: listbinds <device> [keymap]\n");
    return 0;
  }

  IndexedCommand

  if (args->size()>=3) {
    int whereToOperate=dev[index]->findMap((*args)[2]);
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

  dprintf(output,"bound keys:\n");
  for (int i=0; i<KEY_CNT; i++) {
    if (mapOp->keybinds[i].doModify) {
      dprintf(output,"- %s\n",keynames[i]);
    }
  }

  return 1;
}

Command(cmd_listdevices) {
  for (size_t i=0; i<dev.size(); i++) {
    dprintf(output,"%zu: %s\n",i,dev[i]->getName().c_str());
  }
  return 1;
}

Command(cmd_enable) {
  if (args->size()<2) {
    dprintf(output,"usage: enable <device>\n");
    return 0;
  }

  IndexedCommand

  if (dev[index]->enabled) {
    dprintf(output,"error: that device is already enabled.\n");
    return 0;
  }

  dev[index]->activate();
  dev[index]->enabled=true;
  return 1;
}

Command(cmd_disable) {
  if (args->size()<2) {
    dprintf(output,"usage: disable <device>\n");
    return 0;
  }

  IndexedCommand

  if (!dev[index]->enabled) {
    dprintf(output,"error: that device is already disabled.\n");
    return 0;
  }

  dev[index]->deactivate();
  dev[index]->enabled=false;
  return 1;
}

Command(cmd_version) {
  dprintf(output,
  "input-modifier (version " IMOD_VERSION ")\n"
  "copyright 2018 tildearrow\n\n"
  "this program is free software; you can redistribute it and/or modify\n"
  "it under the terms of the GNU General Public License as published by\n"
  "the Free Software Foundation; either version 2 of the License, or\n"
  "(at your option) any later version.\n\n"

  "this program is distributed in the hope that it will be useful,\n"
  "but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
  "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  see the\n"
  "GNU General Public License for more details.\n\n"

  "you should have received a copy of the GNU General Public License along\n"
  "with this program; if not, write to the Free Software Foundation, Inc.,\n"
  "51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.\n\n"

  "to be honest, this program may not be useful at the current moment, but\n"
  "this may change in a future version.\n");
  return 1;
}

/*
Command(cmd_tstest) {
  struct timespec ts;
  if (args->size()<2) {
    dprintf(output,"number\n");
    return 0;
  }
  try {
    ts=stots((*args)[1]);
  } catch (std::exception& err) {
    dprintf(output,"error in your input.\n");
    return 0;
  }
  dprintf(output,"sec: %ld, nsec %ld\n",ts.tv_sec,ts.tv_nsec);
  return 1;
}
*/

const AvailCommands cmds[]={
  {"addaction", cmd_addaction},
  {"delaction", cmd_delaction},
  {"clearactions", cmd_clearactions},
  {"copyactions", cmd_copyactions},
  {"listactions", cmd_listactions},
  {"listbinds", cmd_listbinds},
  {"listmaps", cmd_listmaps},
  {"listdevices", cmd_listdevices},
  {"enable", cmd_enable},
  {"disable", cmd_disable},
  /*
  {"showsettings", cmd_showsettings},
  {"settings", cmd_settings},*/
  {"newmap", cmd_newmap},
  {"copymap", cmd_copymap},
  {"delmap", cmd_delmap},
  {"switchmap", cmd_switchmap},
/*
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
*/
  {"version", cmd_version},
  {NULL, NULL}
};
