#include "imodd.h"

std::vector<Device*> dev;
std::vector<Macro*> macros;
SocketInterface sock;

DeviceListener* list;

string configDir;

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

void childHandler(int data) {
  int childStatus;
  pid_t pid;
  while (1) {
    pid=waitpid(-1,&childStatus,WNOHANG|__WALL);
    if (pid<=0) return;
    if (childStatus!=0) {
      imLogI("last program exited with code %d\n",childStatus);
    }
  }
}

int main(int argc, char** argv) {
  struct sigaction sintH, stermH, ststpH, chldH;
  int tempuifd;
  memset(&sintH,0,sizeof(struct sigaction));
  memset(&stermH,0,sizeof(struct sigaction));
  memset(&ststpH,0,sizeof(struct sigaction));
  memset(&chldH,0,sizeof(struct sigaction));
  // check permissions of uinput
  tempuifd=open("/dev/uinput",O_RDWR);
  if (tempuifd<0) {
    imLogW("can't access /dev/uinput: %s\n",strerror(errno));
    imLogW("fixing this problem for you...\n");
    if (system(_PREFIX "/bin/imod-uinput-helper")!=0) // or
    if (system("./imod-uinput-helper")!=0) {
      imLogE("error while trying to fix this problem for you...\n");
      return 1;
    }
    tempuifd=open("/dev/uinput",O_RDWR);
    if (tempuifd<0) {
      imLogE("still can't access /dev/uinput: %s\n",strerror(errno));
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
  }
  close(tempuifd);
  // set config dir
  if (getenv("HOME")==NULL) {
    imLogE("i don't know where's your home directory...\n");
    return 1;
  }
  configDir=getenv("HOME");
  configDir+="/.config/input-modifier";
  if (access(configDir.c_str(),0)!=F_OK) {
    imLogI("creating config directory (%s)...\n",configDir.c_str());
    if (mkdir(configDir.c_str(),0755)<0) {
      imLogE("error while creating config dir: %s\n",strerror(errno));
      return 1;
    }
  }
  configDir+="/";
  
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
  // initialize device listener
  list=new DeviceListener;
  if (!list->init()) {
    imLogE("giving up.\n");
    return 1;
  }
  // set signal handlers
  sintH.sa_handler=exitHandler;
  sigaction(SIGINT,&sintH,NULL);
  stermH.sa_handler=exitHandler;
  sigaction(SIGTERM,&stermH,NULL);
  ststpH.sa_handler=stopHandler;
  sigaction(SIGTSTP,&ststpH,NULL);
  chldH.sa_handler=childHandler;
  sigaction(SIGCHLD,&chldH,NULL);
  // initialize interfaces
  if (!sock.init()) {
    return 1;
  }
  // initialize devices (and input threads)
  imLogI("initializing devices...\n");
  for (auto i: dev) {
    i->init();
    i->loadState(configDir+i->getSaneName());
    if (i->enabled) {
      i->activate();
    }
  }
  imLogI("loading macros...\n");
  loadMacros(configDir+S("macros.json"));
  imLogI("done! running.\n");
  while (1) {
    pause();
    if (whatToDo==doQuit) {
      list->deactivate();
      for (auto i: dev) {
        if (i->enabled) {
          i->deactivate();
        }
        i->saveState(configDir+i->getSaneName());
        saveMacros(configDir+S("macros.json"));
      }
      break;
    }
    if (whatToDo==doSuspend) {
      for (auto i: dev) {
        if (i->enabled) {
          i->deactivate();
        }
        i->saveState(configDir+i->getSaneName());
        saveMacros(configDir+S("macros.json"));
      }
      raise(SIGSTOP);
      ststpH.sa_handler=stopHandler;
      sigaction(SIGTSTP,&ststpH,NULL);
      for (auto i: dev) {
        if (i->enabled) {
          i->activate();
        }
      }
    }
    whatToDo=doNothing;
  }
  imLogI("exiting.\n");
  return 0;
}
