#include "imodd.h"

std::vector<Device*> dev;
std::vector<Macro*> macros;
std::vector<LoadedPlugin*> plug;
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

void pipeHandler(int data) {
  imLogW("caught broken pipe signal!\n");
}

void testExtern() {
  imLogI("hello from the daemon!\n");
}

int main(int argc, char** argv) {
  struct sigaction sintH, stermH, ststpH, chldH, spipeH;
  int tempuifd;
  struct stat uistat;
  memset(&sintH,0,sizeof(struct sigaction));
  memset(&stermH,0,sizeof(struct sigaction));
  memset(&ststpH,0,sizeof(struct sigaction));
  memset(&chldH,0,sizeof(struct sigaction));
  memset(&spipeH,0,sizeof(struct sigaction));
  // check permissions of uinput
  tempuifd=open("/dev/uinput",O_RDWR);
  if (tempuifd<0) {
    imLogE("can't access /dev/uinput: %s\n",strerror(errno));
    if (stat("/dev/uinput",&uistat)<0) {
      if (errno==ENOENT) {
        imLogE("the uinput module is not loaded, or not available in your kernel.\n");
        imLogE("try doing:\n");
        imLogE("sudo modprobe uinput\n");
        return 1;
      } else {
        imLogE("can't even stat /dev/uinput: %s\n",strerror(errno));
        imLogE("this is not good...\n");
        imLogE("please report this bug now at https://github.com/tildearrow/input-modifier/issues/new\n");
        imLogE("thank you.\n");
        return 1;
      }
    }
    // I want to ask. why did they prefix every struct's items if it's inside the struct?!
    // it's like, i'm typing... sf::sf_RenderWindow w=w.sf_rw_create(sf::sf_VideoMode::sf_vm_getDesktopMode(),"app",sf::sf_Style::sf_st_Close);!!!
    // check for group
    if (uistat.st_mode&060) {
      imLogE("try adding yourself to the 'input' group:\n");
      if (getenv("USER")==NULL) {
        imLogE("...wait, what? who are you?\n");
        imLogE("anyway, here is the command:\n");
        imLogE("sudo usermod -a -G input <USERNAME>\n");
        return 1;
      }
      imLogE("sudo usermod -a -G input %s\n",getenv("USER"));
      return 1;
    } else {
      imLogE("it seems that the uinput udev rule didn't work out well.\n");
      imLogE("please report this bug now at https://github.com/tildearrow/input-modifier/issues/new\n");
      imLogE("thank you.\n");
      return 1;
    }
  }
  close(tempuifd);
  // set config dir
  if (getenv("XDG_CONFIG_HOME")==NULL) {
    if (getenv("HOME")==NULL) {
      imLogE("i don't know where's your home directory...\n");
      return 1;
    }
    configDir=getenv("HOME");
    configDir+="/.config/input-modifier";
  } else {
    configDir=getenv("XDG_CONFIG_HOME");
    configDir+="/input-modifier";
  }
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
    imLogE("couldn't initialize the device listener!\n");
    return 1;
  }
  // set signal handlers
  sintH.sa_handler=exitHandler;
  stermH.sa_handler=exitHandler;
  ststpH.sa_handler=stopHandler;
  chldH.sa_handler=childHandler;
  spipeH.sa_handler=pipeHandler;
  
  if (sigaction(SIGINT,&sintH,NULL)<0   ||
      sigaction(SIGTERM,&stermH,NULL)<0 ||
      sigaction(SIGTSTP,&ststpH,NULL)<0 ||
      sigaction(SIGCHLD,&chldH,NULL)<0  ||
      sigaction(SIGPIPE,&spipeH,NULL)<0) {
    imLogE("failure to set signal handlers!\n");
    return 1;
  }
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
  imLogI("loading plugins...\n");
  scanAndLoadPlugins(plug);
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
