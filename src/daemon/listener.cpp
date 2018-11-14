#include "imodd.h"

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

void DeviceListener::run() {
  char buf[512];
  struct inotify_event* event;
  int len;

  while (1) {
    len=read(fd,buf,512);
    if (len<=0) {
      break;
    }
    imLogD("we read from the listener.\n");
  }
}
