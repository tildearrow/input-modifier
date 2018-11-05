#include "imodd.h"

std::vector<Device*> dev;

enum {
  doNothing=0,
  doQuit,
  doSuspend,
} whatToDo;

void exitHandler(int data) {
  imLogI("received quit request.\n");
  whatToDo=doQuit;
}

void stopHandler(int data) {
  imLogI("received suspend request.\n");
  whatToDo=doSuspend;
}

int main(int argc, char** argv) {
  struct sigaction sintH, stermH, ststpH;
  // scan devices
  if (scanDev(dev)==-2) {
    imLogE("there are devices but I can't open any.\n");
    imLogE("try adding yourself to the 'input' group:\n");
    if (getenv("USER")==NULL) {
      imLogE("...wait, what? who are you?\n");
      imLogE("anyway, here is the command:\n");
      imLogE("sudo usermod -a -G input <USERNAME>\n");
      return 1;
    }
    imLogE("sudo usermod -a -G input %s\n",getenv("USER"));
    return 1;
  }
  // set signal handlers
  sintH.sa_handler=exitHandler;
  sigaction(SIGINT,&sintH,NULL);
  stermH.sa_handler=exitHandler;
  sigaction(SIGTERM,&stermH,NULL);
  ststpH.sa_handler=stopHandler;
  sigaction(SIGTSTP,&ststpH,NULL);
  // initialize devices (and input threads)
  imLogI("initializing devices...\n");
  for (auto i: dev) {
    i->init();
    i->activate();
  }
  imLogI("done! running.\n");
  while (1) {
    pause();
    if (whatToDo==doQuit) break;
    if (whatToDo==doSuspend) raise(SIGSTOP);
    whatToDo=doNothing;
  }
  imLogI("exiting.\n");
  return 0;
}
