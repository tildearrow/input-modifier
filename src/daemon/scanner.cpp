#include "imodd.h"

int scanDev(std::vector<Device*>& populate) {
  DIR* scanDir;
  int tempfd;
  struct dirent* subject;
  unsigned int cap;
  char tempname[256];
  scanDir=opendir(DEVICE_DIR);
  if (scanDir==NULL) {
    imLogE("error while trying to scan devices: %s",strerror(errno));
    return -1;
  }
  imLogI("scanning devices...\n");
  while ((subject=readdir(scanDir))!=NULL) {
    // check if char device
    if (subject->d_type!=DT_CHR) continue;
    // check if event device
    if (strstr(subject->d_name,"event")!=subject->d_name) continue;
    // check if weird device
    tempfd=open((S(DEVICE_DIR)+S("/")+S(subject->d_name)).c_str(),O_RDONLY);
    if (tempfd<0) {
      imLogW("couldn't open %s: %s\n",subject->d_name,strerror(errno));
      continue;
    }
    if (ioctl(tempfd,EVIOCGBIT(0,sizeof(unsigned int)),&cap)<0) {
      imLogW("couldn't get event bits for %s: %s\n",subject->d_name,strerror(errno));
      close(tempfd);
      continue;
    }
    // super weird devices are anything that don't do key/rel/abs events
    if (allowWeird<2) if (!(cap&(1<<EV_KEY|1<<EV_REL|1<<EV_ABS))) {
      close(tempfd);
      continue;
    }
    if (allowWeird<1) {
      // weird devices are sleep/power/volume buttons and the video bus
       if (ioctl(tempfd,EVIOCGNAME(255),tempname)<0) {
         imLogW("couldn't get name for %s: %s\n",subject->d_name,strerror(errno));
         close(tempfd);
         continue;
       }
       if (strcmp(tempname,"Power Button")==0 ||
           strcmp(tempname,"Sleep Button")==0 ||
           strcmp(tempname,"Video Bus")==0) {
         close(tempfd);
         continue;
       }
    }
    close(tempfd);
    imLogD("device: %s\n",subject->d_name);
    populate.push_back(new Device(S(DEVICE_DIR)+S("/")+S(subject->d_name)));
  }
  return 0;
}
