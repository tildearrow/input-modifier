#include "imodd.h"

int processDev(std::vector<Device*>& populate, string name) {
  // check if weird device
  int tempfd;
  char tempname[256];
  bool doNotCreate;
  unsigned int cap;
  doNotCreate=false;
  tempfd=open((S(DEVICE_DIR)+S("/")+name).c_str(),O_RDONLY);
  if (tempfd<0) {
    imLogW("couldn't open %s: %s\n",name.c_str(),strerror(errno));
    if (errno==EACCES) return 3;
    return 0;
  }
  if (ioctl(tempfd,EVIOCGBIT(0,sizeof(unsigned int)),&cap)<0) {
    imLogW("couldn't get event bits for %s: %s\n",name.c_str(),strerror(errno));
    close(tempfd);
    return 0;
  }
  if (ioctl(tempfd,EVIOCGNAME(255),tempname)<0) {
    imLogW("couldn't get name for %s: %s\n",name.c_str(),strerror(errno));
    close(tempfd);
    return 0;
  }
  // super weird devices are anything that don't do key/rel/abs events
  if (allowWeird<2) if (!(cap&(1<<EV_KEY|1<<EV_REL|1<<EV_ABS))) {
    close(tempfd);
    return 0;
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
      return 0;
    }
  }
  close(tempfd);
  imLogD("device: %s\n",name.c_str());
  // check if device already exists in populate array
  // here is why I have to do this:
  // - some devices (especially Razer ones) tend to actually be
  //   many input devices in one.
  //   I don't know why would they do that, but my only optionh
  //   is to work around it...
  for (auto& i: populate) {
    if (i->getName()==S(tempname)) {
      imLogD("NOTE: they match.\n");
      i->addPath(S(DEVICE_DIR)+S("/")+name);
      doNotCreate=true;
      break;
    }
  }
  if (doNotCreate) return 2;
  imLogD("new device.\n");
  populate.push_back(new Device(S(tempname),S(DEVICE_DIR)+S("/")+name));
  return 1;
}

int scanDev(std::vector<Device*>& populate) {
  DIR* scanDir;
  struct dirent* subject;
  bool permden;
  permden=true;
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
    if (processDev(populate,subject->d_name)!=3) {
      permden=false;
    }
  }
  if (populate.empty()) return 0;
  if (permden) return -2;
  return populate.size();
}

int scanAndLoadPlugins(std::vector<LoadedPlugin*>& populate) {
  LoadedPlugin* p;
  struct dirent* subject;
  DIR* plugDir;
  
  plugDir=opendir(PLUGIN_DIR);
  if (plugDir==NULL) {
    imLogW("error while trying to scan plugins: %s",strerror(errno));
    return -1;
  }
  
  while ((subject=readdir(plugDir))!=NULL) {
    if (subject->d_type!=DT_REG) continue;
    p=new LoadedPlugin;
    if (!p->loadFile(PLUGIN_DIR+S("/")+S(subject->d_name))) {
      imLogW("plugin not loaded.\n");
      delete p;
      continue;
    }
    imLogI("initializing plugin %s...\n",p->info->name.c_str());
    if (!p->init()) {
      imLogW("couldn't init plugin %s!\n",p->info->name.c_str());
      delete p;
      continue;
    }
    // register comamnds
    plugCmds.insert(plugCmds.end(),p->info->commands.begin(),p->info->commands.end());
    populate.push_back(p);
  }
  
  closedir(plugDir);
  
  return populate.size();
}
