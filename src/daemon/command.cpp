#include "imodd.h"

extern std::vector<Device*> dev;

Command(cmd_ls) {
  dprintf(output,"Device List.\n");
  for (size_t i=0; i<dev.size(); i++) {
    dprintf(output,"%zu: %s: %s\n",i,dev[i]->getPath().c_str(),dev[i]->getName().c_str());
  }
  return 1;
}

const AvailCommands cmds[]={
  {"ls", cmd_ls},
  {NULL, NULL}
};
