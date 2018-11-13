#include "imodd.h"
#include <fstream>
#include "../json/json.hpp"

const char* actionNames[]={
  "key",
  "turbo",
  "rel",
  "relconst",
  "abs",
  "execute",
  "switchmap",
  "shiftmap",
  "disable"
};

bool Device::loadState(string path) {
  std::ifstream f;
  nlohmann::json data;
  string actName;
  std::vector<string> argVector;
  std::vector<string> emptyEnv;
  int actType;
  int mapIndex=-1;
  int bindCode;
  f.open(path);
  
  if (f.is_open()) {
    f>>data;
  } else {
    // load default state...
    mappings.push_back(new bindSet("Default"));
    curmap=mappings[0];
    return false;
  }
  
  imLogI("loading state for device %s...\n",name.c_str());
  
  try {
    enabled=data["enabled"];
    
    for (auto& i: data["mappings"]) {
      mapIndex++;
      newMap(i["name"]);
      for (auto& j: i["keybinds"]) {
        bindCode=j["code"];
        mappings[mapIndex]->keybinds[bindCode].doModify=true;
        for (auto& k: j["actions"]) {
          actName=k["type"];
          for (int l=0; l<9; l++) {
            if (S(actionNames[l])==actName) {
              actType=l;
              break;
            }
          }
          switch (actType) {
            case actKey:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actKey,int(k["code"])));
              break;
            case actTurbo:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actTurbo,k["code"],mkts(k["timeOn"][0],k["timeOn"][1]),mkts(k["timeOff"][0],k["timeOff"][1])));
              break;
            case actRel:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actRel,k["code"],k["value"]));
              break;
            case actRelConst:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actRel,k["code"],k["value"],mkts(k["delay"][0],k["delay"][1])));
              break;
            case actAbs:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actAbs,k["code"],k["value"]));
              break;
            case actExecute:
              argVector.clear();
              for (string l: k["args"]) {
                argVector.push_back(l);
              }
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actExecute,k["command"],argVector,emptyEnv));
              break;
            case actSwitchMap:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actSwitchMap,string(k["name"])));
              break;
            case actShiftMap:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actShiftMap,string(k["name"])));
              break;
            case actDisable:
              mappings[mapIndex]->keybinds[bindCode].actions.
                push_back(Action(actDisable,0));
              break;
          }
        }
      }
    }
  } catch (nlohmann::json::exception& err) {
    imLogW("couldn't load state: %s\n",err.what());
    // load default state...
    mappings.push_back(new bindSet("Default"));
    curmap=mappings[0];
    return false;
  }
  
  curmap=mappings[findMap(data["curmap"])];
  return true;
}

bool Device::saveState(string path) {
  using namespace nlohmann;
  std::ofstream f;
  json data;
  json mapPartO;
  json bindPart;
  json actionPart;
  std::vector<json> mapPart;
  imLogI("saving state for device %s...\n",name.c_str());
  
  data["name"]=name;
  data["enabled"]=enabled;
  data["curmap"]=curmap->name;
  data["mappings"]=json::array();
  
  for (auto i: mappings) {
    mapPartO=json::object();
    mapPartO["name"]=i->name;
    mapPartO["keybinds"]=json::array();
    for (int j=0; j<KEY_CNT; j++) {
      if (i->keybinds[j].doModify) {
        bindPart=json::object();
        bindPart["code"]=j;
        bindPart["actions"]=json::array();
        for (auto& k: i->keybinds[j].actions) {
          actionPart["type"]=actionNames[k.type];
          switch (k.type) {
            case actKey:
              actionPart["code"]=k.code;
              break;
            case actTurbo:
              actionPart["code"]=k.code;
              actionPart["timeOn"]={k.timeOn.tv_sec, k.timeOn.tv_nsec};
              actionPart["timeOff"]={k.timeOff.tv_sec, k.timeOff.tv_nsec};
              break;
            case actRel: case actAbs:
              actionPart["code"]=k.code;
              actionPart["value"]=k.value;
              break;
            case actRelConst:
              actionPart["code"]=k.code;
              actionPart["value"]=k.value;
              actionPart["delay"]={k.timeOn.tv_sec, k.timeOn.tv_nsec};
              break;
            case actExecute:
              actionPart["command"]=k.command;
              actionPart["args"]=k.args;
              break;
            case actShiftMap: case actSwitchMap:
              actionPart["name"]=k.command;
              break;
            case actDisable:
              break;
          }
          bindPart["actions"].push_back(actionPart);
        }
        mapPartO["keybinds"].push_back(bindPart);
      }
    }
    data["mappings"].push_back(mapPartO);
  }
  
  f.open(path);
  if (f.is_open()) {
    f<<data;
  } else {
    imLogE("couldn't save state: %s\n",strerror(errno));
    return false;
  }
  
  return true;
}
