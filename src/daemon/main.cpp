#include "imodd.h"

std::vector<Device*> dev;
SocketInterface sock;

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
    if (i->enabled) {
      i->activate();
    }
  }
  imLogI("done! running.\n");
  while (1) {
    pause();
    if (whatToDo==doQuit) {
      for (auto i: dev) {
        if (i->active) {
          i->deactivate();
        }
      }
      break;
    }
    if (whatToDo==doSuspend) {
      for (auto i: dev) {
        if (i->active) {
          i->deactivate();
        }
      }
      raise(SIGSTOP);
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
