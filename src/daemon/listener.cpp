#include "imodd.h"

extern std::vector<Device*> dev;

void* listenThread(void* data) {
  DeviceListener* inst=(DeviceListener*)data;
  inst->run();
  return NULL;
}

bool DeviceListener::init() {
  imLogI("starting device listener...\n");
  fd=inotify_init();
  if (fd<0) {
    imLogE("could not init inotify: %s\n",strerror(errno));
    return false;
  }

  wd=inotify_add_watch(fd,DEVICE_DIR,IN_CREATE|IN_DELETE);
  if (wd<0) {
    imLogE("could not add watch: %s\n",strerror(errno));
    return false;
  }

  // fire listener thread
  pthread_create(&thread,NULL,listenThread,this);
  return true;
}

bool DeviceListener::deactivate() {
  return true;
}

void DeviceListener::run() {
  char buf[512];
  char cstr[4096];
  std::vector<string> newDevices;
  bool doTimeout;
  struct timespec nextSettle, ctime;
  struct inotify_event* event;
  int len, off, tempfd;
  fd_set fdset;

  doTimeout=false;

  while (1) {
    FD_ZERO(&fdset);
    FD_SET(fd,&fdset);
    ctime=nextSettle-curTime(CLOCK_MONOTONIC);
    if (ctime<mkts(0,0)) {
      ctime=mkts(0,0);
    } 
    if (pselect(fd+1,&fdset,NULL,NULL,(doTimeout)?(&ctime):NULL,NULL)==0) {
      imLogD("i must sort out\n");
      for (auto& i: newDevices) {
        // check if this is just a mapped device
        tempfd=open((S(DEVICE_DIR)+S("/")+i).c_str(),O_RDONLY);
        if (tempfd<0) {
          imLogW("error while trying to check out %s: %s\n",event->name,strerror(errno));
          continue;
        }
        ioctl(tempfd,EVIOCGNAME(4095),cstr);
        close(tempfd);
        if (strstr(cstr,"(mapped) ")==cstr) {
          continue;
        }
        imLogD("- %s\n",i.c_str());
      }
      doTimeout=false;
      newDevices.clear();
      continue;
    }
    len=read(fd,buf,512);
    if (len<=0) {
      break;
    }
    imLogD("we read from the listener.\n");
    off=0;
    // to be able to read multiple events
    while (off<len) {
      event=(struct inotify_event*)(buf+off);
      off+=sizeof(struct inotify_event)+event->len+1;
      // ignore if it does not begin with "event"
      if (strstr(event->name,"event")!=event->name) continue;
      if (event->mask&IN_CREATE) {
        imLogD("create: %s\n",event->name);
        newDevices.push_back(event->name);
      }
      if (event->mask&IN_DELETE) {
        imLogD("delete: %s\n",event->name);
        for (size_t i=0; i<newDevices.size(); i++) {
          if (newDevices[i]==S(event->name)) {
            newDevices.erase(newDevices.begin()+i);
            i--;
          }
        }
      }
    }
    // 1 second for settling
    nextSettle=curTime(CLOCK_MONOTONIC)+mkts(1,0);
    doTimeout=true;
  }
}
