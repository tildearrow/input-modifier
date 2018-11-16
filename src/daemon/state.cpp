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

bool Device::loadProfile(string path) {
  int actType;
  int mapIndex=-1;
  int bindCode;
  std::ifstream f;
  nlohmann::json data;
  string actName;
  std::vector<string> argVector;
  std::vector<string> emptyEnv;
  f.open(path);
  
  if (f.is_open()) {
    f>>data;
  } else {
    return false;
  }
  
  imLogI("loading profile %s for device %s...\n",curProfile.c_str(),name.c_str());
  
  // cleanup
  curmap=NULL;
  for (auto& i: mappings) {
    delete i;
  }
  mappings.clear();
  
  // load the profile
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
  curmap=mappings[findMap(data["curmap"])];
  return true;
}

bool Device::saveProfile(string path, string dirpath) {
  using namespace nlohmann;
  std::ofstream f;
  json data;
  json mapPartO;
  json bindPart;
  json actionPart;
  std::vector<json> mapPart;
  
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
            default:
              actionPart["type"]="disable";
              break;
          }
          bindPart["actions"].push_back(actionPart);
        }
        mapPartO["keybinds"].push_back(bindPart);
      }
    }
    data["mappings"].push_back(mapPartO);
  }
  
  if (access(dirpath.c_str(),0)!=F_OK) {
    imLogI("creating profile directory for %s...\n",name.c_str());
    mkdir(dirpath.c_str(),0755);
  }
  
  f.open(path);
  if (f.is_open()) {
    f<<data;
  } else {
    imLogE("couldn't save profile: %s\n",strerror(errno));
    return false;
  }
  return true;
}

bool Device::loadState(string path) {
  std::ifstream f;
  nlohmann::json data;
  string actName;
  std::vector<string> argVector;
  std::vector<string> emptyEnv;
  f.open(path+S(".json"));
  
  if (f.is_open()) {
    f>>data;
  } else {
    // load default state...
    mappings.push_back(new bindSet("Default"));
    curmap=mappings[0];
    saveProfile(path+S("/")+curProfile+S(".json"),path);
    return false;
  }
  
  imLogI("loading state for device %s...\n",name.c_str());
  
  try {
    enabled=data["enabled"];
    curProfile=data["profile"];
    
    if (!loadProfile(path+S("/")+curProfile+S(".json"))) {
      saveProfile(path+S("/")+curProfile+S(".json"),path);
    }
  } catch (nlohmann::json::exception& err) {
    imLogW("couldn't load state: %s\n",err.what());
    // load default state...
    mappings.push_back(new bindSet("Default"));
    curmap=mappings[0];
    curProfile="Default";
    saveProfile(path+S("/")+curProfile+S(".json"),path);
    return false;
  }
  
  return true;
}

bool Device::saveState(string path) {
  using namespace nlohmann;
  std::ofstream f;
  json data;
  imLogI("saving state for device %s...\n",name.c_str());
  
  data["name"]=name;
  data["enabled"]=enabled;
  data["profile"]=curProfile;
  
  saveProfile(path+S("/")+curProfile+S(".json"),path);
  
  f.open(path+S(".json"));
  if (f.is_open()) {
    f<<data;
  } else {
    imLogE("couldn't save state: %s\n",strerror(errno));
    return false;
  }
  
  return true;
}
