#include "imodd.h"

int scanDev(std::vector<Device*>& populate) {
  DIR* scanDir;
  int tempfd;
  struct dirent* subject;
  unsigned int cap;
  bool permden, doNotCreate;
  char tempname[256];
  permden=true;
  scanDir=opendir(DEVICE_DIR);
  if (scanDir==NULL) {
    imLogE("error while trying to scan devices: %s",strerror(errno));
    return -1;
  }
  imLogI("scanning devices...\n");
  while ((subject=readdir(scanDir))!=NULL) {
    doNotCreate=false;
    // check if char device
    if (subject->d_type!=DT_CHR) continue;
    // check if event device
    if (strstr(subject->d_name,"event")!=subject->d_name) continue;
    // check if weird device
    tempfd=open((S(DEVICE_DIR)+S("/")+S(subject->d_name)).c_str(),O_RDONLY);
    if (errno!=EACCES) {
      permden=false;
    }
    if (tempfd<0) {
      imLogW("couldn't open %s: %s\n",subject->d_name,strerror(errno));
      continue;
    }
    if (ioctl(tempfd,EVIOCGBIT(0,sizeof(unsigned int)),&cap)<0) {
      imLogW("couldn't get event bits for %s: %s\n",subject->d_name,strerror(errno));
      close(tempfd);
      continue;
    }
    if (ioctl(tempfd,EVIOCGNAME(255),tempname)<0) {
      imLogW("couldn't get name for %s: %s\n",subject->d_name,strerror(errno));
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
      if (strcmp(tempname,"Power Button")==0 ||
          strcmp(tempname,"Sleep Button")==0 ||
          strcmp(tempname,"Video Bus")==0 ||
          strcmp(tempname,"applesmc")==0 ||
          strstr(tempname,"FaceTime")==tempname ||
          strcmp(tempname,"Speakup")==0) {
        close(tempfd);
        continue;
      }
    }
    close(tempfd);
    imLogD("device: %s\n",subject->d_name);
    permden=false;
    // check if device already exists in populate array
    // here is why I have to do this:
    // - some devices (especially Razer ones) tend to actually be
    //   many input devices in one.
    //   I don't know why would they do that, but my only optionh
    //   is to work around it...
    for (auto& i: populate) {
      if (i->getName()==S(tempname)) {
        imLogD("NOTE: they match.\n");
        i->addPath(S(DEVICE_DIR)+S("/")+S(subject->d_name));
        doNotCreate=true;
        break;
      }
    }
    if (doNotCreate) continue;
    imLogD("new device.\n");
    populate.push_back(new Device(S(tempname),S(DEVICE_DIR)+S("/")+S(subject->d_name)));
  }
  if (populate.empty()) return 0;
  if (permden) return -2;
  return populate.size();
}
