#include "imodd.h"

#define ReleaseKeys() \
  for (int keyi=0; keyi<KEY_CNT; keyi++) { \
    if (pressedKeys[keyi]) { \
      wire.type=EV_KEY; \
      wire.code=keyi; \
      wire.value=0; \
      write(uinputfd,&wire,sizeof(struct input_event)); \
      write(uinputfd,&syncev,sizeof(struct input_event)); \
      pressedKeys[keyi]=false; \
    } \
  }

Device::Device():
  fds(0),
  isBulkDevice(false),
  name(""),
  phys(""),
  uinputfd(-1),
  inThread(-1),
  inited(false),
  active(false),
  enabled(false),
  curmap(NULL) {
  for (int i=0; i<8; i++) {
    fd[i]=-1; path[i]="";
  }
}

Device::Device(string n, string p):
  fds(1),
  isBulkDevice(false),
  name(n),
  phys(""),
  uinputfd(-1),
  inThread(-1),
  inited(false),
  active(false),
  enabled(false),
  curmap(NULL) {
  for (int i=0; i<8; i++) {
    fd[i]=-1; path[i]="";
  }
  path[0]=p;
}

bool Device::addPath(string p) {
  if (fds>=8) return false;
  path[fds++]=p;
  return true;
}

int Device::delPath(string p) {
  for (int i=0; i<fds; i++) {
    if (path[i]==p) {
      if (active) {
        // let the input thread handle closing
        /*
        path[i]="";
        close(fd[i]);
        usleep(20000);
        return 1;
        */
      } else {
        // handle by ourselves
        // remove this file descriptor
        close(fd[i]);
        fd[i]=-1;
        path[i]="";
        if (i!=fds-1) {
          // push down the other descriptors
          for (int j=i+1; j<fds; j++) {
            fd[j]=fd[j-1];
            path[j]=path[j-1];
          }
        }
        fds--;
      }
    }
  }
  if (fds==0) {
    // request deletion
    return 2;
  }
  return 0;
}

string Device::getName() {
  return name;
}

string Device::getSaneName() {
  string ret;
  size_t pos;
  ret=name;
  while ((pos=ret.find(' '))!=string::npos) ret[pos]='-';
  while ((pos=ret.find('/'))!=string::npos) ret[pos]='_';
  return ret;
}

/*
string Device::getPath() {
  return path;
}
*/

bool Device::init() {
  char cstr[4096];
  unsigned char cap[256];
  std::bitset<EV_CNT> tempevcaps;
  for (int h=0; h<fds; h++) {
    imLogD("opening device %s\n",path[h].c_str());
    fd[h]=open(path[h].c_str(),O_RDONLY);
    if (fd[h]<0) {
      imLogW("couldn't open device path %s: %s\n",path[h].c_str(),strerror(errno));
      return false;
    }
    ioctl(fd[h],EVIOCGNAME(4095),cstr);
    name=cstr;
    ioctl(fd[h],EVIOCGPHYS(4095),cstr);
    phys=cstr;
    ioctl(fd[h],EVIOCGID,&info);
    // event caps
    ioctl(fd[h],EVIOCGBIT(0,256),&cap);
    tempevcaps.reset();
    for (int i=0; i<EV_CNT; i++) {
      if (!!(cap[i/8]&(1<<(i&7)))) {
        evcaps[i]=true;
      }
      tempevcaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
    // sync caps
    if (tempevcaps[EV_SYN]) {
      ioctl(fd[h],EVIOCGBIT(EV_SYN,256),&cap);
      for (int i=0; i<SYN_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) syncaps[i]=true;
      }
    }
    // key caps
    if (tempevcaps[EV_KEY]) {
      ioctl(fd[h],EVIOCGBIT(EV_KEY,256),&cap);
      for (int i=0; i<KEY_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) keycaps[i]=true;
      }
    }
    // relative caps
    if (tempevcaps[EV_REL]) {
      ioctl(fd[h],EVIOCGBIT(EV_REL,256),&cap);
      for (int i=0; i<REL_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) relcaps[i]=true;
      }
    }
    // absolute caps
    if (tempevcaps[EV_ABS]) {
      ioctl(fd[h],EVIOCGBIT(EV_ABS,256),&cap);
      for (int i=0; i<ABS_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) abscaps[i]=true;
      }
    }
    // misc caps
    if (tempevcaps[EV_MSC]) {
      ioctl(fd[h],EVIOCGBIT(EV_MSC,256),&cap);
      for (int i=0; i<MSC_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) msccaps[i]=true;
      }
    }
    // switch caps
    if (tempevcaps[EV_SW]) {
      ioctl(fd[h],EVIOCGBIT(EV_SW,256),&cap);
      for (int i=0; i<SW_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) swcaps[i]=true;
      }
    }
    // LED caps
    if (tempevcaps[EV_LED]) {
    ioctl(fd[h],EVIOCGBIT(EV_LED,256),&cap);
      for (int i=0; i<LED_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) ledcaps[i]=true;
      }
    }
    // sound caps
    if (tempevcaps[EV_SND]) {
      ioctl(fd[h],EVIOCGBIT(EV_SND,256),&cap);
      for (int i=0; i<SND_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) sndcaps[i]=true;
      }
    }
    // repeat caps
    if (tempevcaps[EV_REP]) {
      ioctl(fd[h],EVIOCGBIT(EV_REP,256),&cap);
      for (int i=0; i<REP_CNT; i++) {
        if (!!(cap[i/8]&(1<<(i&7)))) repcaps[i]=true;
      }
    }
    // absolute properties
    if (tempevcaps[EV_ABS]) for (int i=0; i<ABS_CNT; i++) {
      if (abscaps[i]) {
        ioctl(fd[h],EVIOCGABS(i),&absinfo[i]);
      }
    }
    imLogD("%s: %s\n",path[h].c_str(),name.c_str());
    imLogD("  - phys: %s\n",phys.c_str());
    imLogD("  - vendor/product: %.4x:%.4x\n",info.vendor,info.product);
    imLogD("  - evcaps: %s\n",evcaps.to_string().c_str());
    if (evcaps[EV_SYN]) imLogD("    - syncaps: %s\n",syncaps.to_string().c_str());
    if (evcaps[EV_KEY]) imLogD("    - keycaps: %s\n",keycaps.to_string().c_str());
    if (evcaps[EV_REL]) imLogD("    - relcaps: %s\n",relcaps.to_string().c_str());
    if (evcaps[EV_ABS]) imLogD("    - abscaps: %s\n",abscaps.to_string().c_str());
    if (evcaps[EV_MSC]) imLogD("    - msccaps: %s\n",msccaps.to_string().c_str());
    if (evcaps[EV_SW]) imLogD("    - swcaps: %s\n",swcaps.to_string().c_str());
    if (evcaps[EV_LED]) imLogD("    - ledcaps: %s\n",ledcaps.to_string().c_str());
    if (evcaps[EV_SND]) imLogD("    - sndcaps: %s\n",sndcaps.to_string().c_str());
    if (evcaps[EV_REP]) imLogD("    - repcaps: %s\n",repcaps.to_string().c_str());
    
    close(fd[h]);
    fd[h]=-1;
  }

  // initialize map if needed
  /*if (mappings.empty()) {
    mappings.push_back(new bindSet("Default"));
    curmap=mappings[0];
  }*/
  inited=true;
  return true;
}

void* devThread(void* dev) {
  ((Device*)dev)->run();
  return NULL;
}

bool Device::activate() {
  for (int i=0; i<fds; i++) {
    imLogD("opening device %s for remapping\n",path[i].c_str());
    fd[i]=open(path[i].c_str(),O_RDONLY);
    if (fd[i]<0) {
      imLogW("couldn't open device path %s: %s\n",path[i].c_str(),strerror(errno));
      return false;
    }
  }
  
  uinputfd=open("/dev/uinput",O_RDWR);
  if (uinputfd<0) {
    imLogW("couldn't open /dev/uinput for device %s: %s\n",name.c_str(),strerror(errno));
    imLogW("fixing this problem...\n");
    if (system(_PREFIX "/bin/imod-uinput-helper")!=0) // or
    if (system("./imod-uinput-helper")!=0) {
      imLogE("error while trying to fix this problem for you...\n");
      return false;
    }
    uinputfd=open("/dev/uinput",O_RDWR);
    if (uinputfd<0) {
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
  strncpy(uiconfig.name,"(mapped) ",UINPUT_MAX_NAME_SIZE);
  strncat(uiconfig.name,name.c_str(),UINPUT_MAX_NAME_SIZE);
  uiconfig.id.bustype=BUS_VIRTUAL;
  uiconfig.id.product=info.product;
  uiconfig.id.vendor=info.vendor;
  uiconfig.id.version=1;

  // copy evbits
  for (int i=0; i<EV_CNT; i++) if (evcaps[i]) ioctl(uinputfd,UI_SET_EVBIT,i);
  if (evcaps[EV_KEY]) for (int i=0; i<KEY_CNT; i++) if (keycaps[i]) ioctl(uinputfd,UI_SET_KEYBIT,i);
  if (evcaps[EV_REL]) for (int i=0; i<REL_CNT; i++) if (relcaps[i]) ioctl(uinputfd,UI_SET_RELBIT,i);
  if (evcaps[EV_ABS]) for (int i=0; i<ABS_CNT; i++) if (abscaps[i]) ioctl(uinputfd,UI_SET_ABSBIT,i);
  if (evcaps[EV_MSC]) for (int i=0; i<MSC_CNT; i++) if (msccaps[i]) ioctl(uinputfd,UI_SET_MSCBIT,i);
  if (evcaps[EV_SW]) for (int i=0; i<SW_CNT; i++) if (swcaps[i]) ioctl(uinputfd,UI_SET_SWBIT,i);
  if (evcaps[EV_LED]) for (int i=0; i<LED_CNT; i++) if (ledcaps[i]) ioctl(uinputfd,UI_SET_LEDBIT,i);
  if (evcaps[EV_SND]) for (int i=0; i<SND_CNT; i++) if (sndcaps[i]) ioctl(uinputfd,UI_SET_SNDBIT,i);

  for (int i=0; i<ABS_CNT; i++) {
    uiconfig.absmax[i]=absinfo[i].maximum;
    uiconfig.absmin[i]=absinfo[i].minimum;
    uiconfig.absfuzz[i]=absinfo[i].fuzz;
    uiconfig.absflat[i]=absinfo[i].flat;
  }

  write(uinputfd,&uiconfig,sizeof(struct uinput_user_dev));
  ioctl(uinputfd,UI_DEV_CREATE);
  // create input processing thread
  if (pthread_create(&inThread,NULL,devThread,this)!=0) {
    imLogW("couldn't create thread for device %s: %s\n",name.c_str(),strerror(errno));
    close(uinputfd);
    return false;
  }
  active=true;
  imLogI("device %s activated.\n",name.c_str());
  return true;
}

bool Device::deactivate() {
  struct input_event syncev;
  struct input_event wire;
  syncev.type=EV_SYN;
  syncev.code=0;
  syncev.value=0;
  if (active) {
    imLogD("killing.\n");
    pthread_cancel(inThread);
    pthread_join(inThread,NULL);
    imLogD("%s: ungrabbing.\n",name.c_str());
    for (int i=0; i<fds; i++) {
      if (ioctl(fd[i],EVIOCGRAB,0)<0) {
        imLogE("%s: couldn't ungrab %s: %s\n",name.c_str(),path[i].c_str(),strerror(errno));
      }
      imLogD("closing fd %d\n",fd[i]);
      close(fd[i]);
      fd[i]=-1;
    }
    ReleaseKeys();
    close(uinputfd);
    uinputfd=-1;
    imLogI("%s: going down.\n",name.c_str());
    threadRunning=false;
    return true;
  }
  return false;
}

void Device::newMap(string name) {
  mappings.push_back(new bindSet(name));
}

void Device::copyMap(string src, string dest) {
  // hope this works
  mappings.push_back(new bindSet(*mappings[findMap(src)]));
  mappings[mappings.size()-1]->name=dest;
}

void Device::delMap(string name) {
  bindSet* bs;
  int mapIndex;
  mapIndex=findMap(name);
  if (mapIndex<0) return;
  bs=mappings[mapIndex];
  mappings.erase(mappings.begin()+mapIndex);
  delete bs;
}

int Device::findMap(string name) {
  for (size_t i=0; i<mappings.size(); i++) {
    if (mappings[i]->name==name) return i;
  }
  return -1;
}

void Device::run() {
  struct timespec ctime;
  struct input_event event;
  struct input_event syncev;
  struct input_event wire;
  struct sigaction chldH;
  activeTurbo* smallest;
  fd_set devfd;
  int amount, count, found, lastread;
  threadRunning=true;
  syncev.type=EV_SYN;
  syncev.code=0;
  syncev.value=0;
  for (int i=0; i<fds; i++) {
    if (ioctl(fd[i],EVIOCGRAB,1)<0) {
      imLogE("%s: couldn't grab %s: %s\n",name.c_str(),path[i].c_str(),strerror(errno));
    }
  }
  ctime=mkts(0,0);
  while (1) {
    if (!runTurbo.empty()) {
      for (auto i: runTurbo) {
        if (smallest==NULL) {
          smallest=&i;
          continue;
        }
        if (smallest->next<i.next) {
          smallest=&i;
        }
      }
      ctime=curTime(CLOCK_MONOTONIC);
      ctime=smallest->next-curTime(CLOCK_MONOTONIC);
      if (ctime<mkts(0,0)) {
        ctime=mkts(0,0);
      }
    }
    FD_ZERO(&devfd);
    for (int i=0; i<fds; i++) FD_SET(fd[i],&devfd);
    amount=pselect(fd[fds-1]+1,&devfd,NULL,NULL,(runTurbo.empty())?(NULL):(&ctime),NULL);
    if (amount==-1) {
      if (errno==EINTR) continue;
      imLogW("%s: pselect: %s\n",strerror(errno));
    }
    if (amount==0) { 
      // do turbo
      for (activeTurbo& i: runTurbo) {
        i.phase=!i.phase;
        i.next=i.next+((i.phase)?(i.timeOn):(i.timeOff));
        wire.type=EV_KEY;
        wire.code=i.code;
        wire.value=i.phase;
        write(uinputfd,&wire,sizeof(struct input_event));
        write(uinputfd,&syncev,sizeof(struct input_event));
      }
      smallest=NULL;
      continue;
    }
    for (int i=0; i<fds; i++) if (FD_ISSET(fd[i],&devfd)) {
      count=read(fd[i],&event,sizeof(struct input_event));
      lastread=i;
      break;
    }
    if (count<0) {
      if (errno==EINTR) { // we got a signal
        break;
      } else {
        imLogW("read error: %s\n",strerror(errno));
        // remove this file descriptor
        close(fd[lastread]);
        fd[lastread]=-1;
        path[lastread]="";
        if (lastread!=fds-1) {
          // push down the other descriptors
          for (int i=lastread+1; i<fds; i++) {
            fd[i]=fd[i-1];
            path[i]=path[i-1];
          }
        }
        fds--;
        // if there are no file descriptors left we need to quit
        if (fds==0) {
          break;
        }
        continue;
      }
    } 
    imLogD("%s: %d %d %d\n",name.c_str(),event.type,event.code,event.value);
    // EVENT REWIRING BEGIN //
    if (!mapStack.empty()) {
      if (event.type==EV_KEY && event.value==0) {
      for (size_t i=0; i<mapStack.size(); i++) {
          if (mapStack[i].code==event.code) {
            // unshift
            curmap=mapStack[i].map;
            imLogD("going to map %s\n",curmap->name.c_str());
            mapStack.erase(mapStack.begin()+i,mapStack.end()-1);
          }
        }
      }
    }
    if (curmap==NULL) {
      if (event.type==EV_KEY && event.value<2) pressedKeys[event.code]=event.value;
      wire=event;
      write(uinputfd,&wire,sizeof(struct input_event));
    } else {
      switch (event.type) {
        case EV_KEY:
          if (curmap->keybinds[event.code].doModify) {
            for (auto i: curmap->keybinds[event.code].actions) {
              switch (i.type) {
                case actKey:
                  if (event.value<2) pressedKeys[i.code]=event.value;
                  wire.type=EV_KEY;
                  wire.code=i.code;
                  wire.value=event.value;
                  write(uinputfd,&wire,sizeof(struct input_event));
                  write(uinputfd,&syncev,sizeof(struct input_event));
                  break;
                case actTurbo:
                  if (event.value<2) pressedKeys[i.code]=event.value;
                  if (event.value==1) {
                    imLogD("running turbo\n");
                    runTurbo.push_back(activeTurbo(i.timeOn,i.timeOff,curTime(CLOCK_MONOTONIC)+i.timeOn,i.code));
                    wire.type=EV_KEY;
                    wire.code=i.code;
                    wire.value=1;
                    write(uinputfd,&wire,sizeof(struct input_event));
                    write(uinputfd,&syncev,sizeof(struct input_event));
                  } else if (event.value==0) {
                    imLogD("disabling turbo\n");
                    for (std::vector<activeTurbo>::iterator j=runTurbo.begin(); j!=runTurbo.end(); j++) {
                      if (j->code==i.code) {
                        if (j->phase) {
                          wire.type=EV_KEY;
                          wire.code=j->code;
                          wire.value=0;
                          write(uinputfd,&wire,sizeof(struct input_event));
                          write(uinputfd,&syncev,sizeof(struct input_event));
                        }
                        runTurbo.erase(j);
                        break;
                      }
                    }
                  }
                  break;
                case actRel:
                  if (event.value==1) {
                    wire.type=EV_REL;
                    wire.code=i.code;
                    wire.value=i.value;
                    write(uinputfd,&wire,sizeof(struct input_event));
                    write(uinputfd,&syncev,sizeof(struct input_event));
                  }
                  break;
                case actRelConst: // TODO
                  break;
                case actAbs:
                  if (event.value==1) {
                    wire.type=EV_ABS;
                    wire.code=i.code;
                    wire.value=i.value;
                    write(uinputfd,&wire,sizeof(struct input_event));
                    write(uinputfd,&syncev,sizeof(struct input_event));
                  }
                  break;
                case actExecute: // TODO: support for environment
                  char** portedArgs;
                  //char** portedEnv;
                  portedArgs=new char*[i.args.size()+2];
                  portedArgs[0]=new char[i.command.size()+1];
                  strcpy(portedArgs[0],i.command.c_str());
                  for (size_t j=0; j<i.args.size(); j++) {
                    portedArgs[j+1]=new char[i.args[j].size()+1];
                    strcpy(portedArgs[j+1],i.args[j].c_str());
                  }
                  portedArgs[i.args.size()+1]=NULL;
                  if (event.value==1) {
                    chldH.sa_handler=childHandler;
                    sigaction(SIGCHLD,&chldH,NULL);
                    if (fork()==0) {
                      execv(i.command.c_str(),portedArgs);
                      imLogW("can't execute: %s\n",strerror(errno));
                      exit(1);
                    }
                  }
                  for (size_t j=0; j<i.args.size()+2; j++) {
                    delete[] portedArgs[j];
                  }
                  delete[] portedArgs;
                  break;
                case actSwitchMap:
                  if (event.value==1) {
                    found=findMap(i.command);
                    if (found>=0) {
                      ReleaseKeys();
                      curmap=mappings[found];
                    }
                  }
                  break;
                case actShiftMap:
                  if (event.value==1) {
                    found=findMap(i.command);
                    if (found>=0) {
                      mapStack.push_back(mapStackElement(curmap,event.code));
                      ReleaseKeys();
                      curmap=mappings[found];
                      imLogD("shifting to map %s\n",curmap->name.c_str());
                    }
                  }
                  break;
                default:
                  imLogW("unknown/unimplemented action %d...\n",i.type);
                  break;
              }
            }
          } else {
            if (event.value<2) pressedKeys[event.code]=event.value;
            wire=event;
            write(uinputfd,&wire,sizeof(struct input_event));
          }
          break;
        case EV_REL:
          if (curmap->relbinds[event.code].doModify) {

          } else {
            wire=event;
            write(uinputfd,&wire,sizeof(struct input_event));
          }
          break;
        case EV_SW:
          if (curmap->swbinds[event.code].doModify) {

          } else {
            wire=event;
            write(uinputfd,&wire,sizeof(struct input_event));
          }
          break;
        default:
          wire=event;
          write(uinputfd,&wire,sizeof(struct input_event));
      }
    }
    // EVENT REWIRING END //
  }
  // quit if we somehow get out of the loop
  imLogD("%s: ungrabbing.\n",name.c_str());
  for (int i=0; i<fds; i++) {
    if (ioctl(fd[i],EVIOCGRAB,0)<0) {
      imLogE("%s: couldn't ungrab %s: %s\n",name.c_str(),path[i].c_str(),strerror(errno));
    }
    close(fd[i]);
    fd[i]=-1;
  }
  ReleaseKeys();
  close(uinputfd);
  uinputfd=-1;
  imLogI("%s: going down.\n",name.c_str());
  threadRunning=false;
}
