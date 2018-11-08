#include "imodd.h"

Device::Device():
  fd(-1),
  name(""),
  phys(""),
  path(""),
  uinputfd(-1),
  inThread(-1),
  active(false),
  curmap(NULL) {
}

Device::Device(string p):
  fd(-1),
  name(""),
  phys(""),
  path(p),
  uinputfd(-1),
  inThread(-1),
  active(false),
  curmap(NULL) {
}

bool Device::init() {
  char cstr[4096];
  unsigned char cap[256];
  imLogD("opening device %s\n",path.c_str());
  fd=open(path.c_str(),O_RDONLY);
  if (fd<0) {
    imLogW("couldn't open device path %s: %s\n",path.c_str(),strerror(errno));
    return false;
  }
  ioctl(fd,EVIOCGNAME(4095),cstr);
  name=cstr;
  ioctl(fd,EVIOCGPHYS(4095),cstr);
  phys=cstr;
  ioctl(fd,EVIOCGID,&info);
  // event caps
  ioctl(fd,EVIOCGBIT(0,256),&cap);
  for (int i=0; i<EV_CNT; i++) {
    evcaps[i]=!!(cap[i/8]&(1<<(i&7)));
  }
  // sync caps
  if (evcaps[EV_SYN]) {
    ioctl(fd,EVIOCGBIT(EV_SYN,256),&cap);
    for (int i=0; i<SYN_CNT; i++) {
      syncaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // key caps
  if (evcaps[EV_KEY]) {
    ioctl(fd,EVIOCGBIT(EV_KEY,256),&cap);
    for (int i=0; i<KEY_CNT; i++) {
      keycaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // relative caps
  if (evcaps[EV_REL]) {
    ioctl(fd,EVIOCGBIT(EV_REL,256),&cap);
    for (int i=0; i<REL_CNT; i++) {
      relcaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // absolute caps
  if (evcaps[EV_ABS]) {
    ioctl(fd,EVIOCGBIT(EV_ABS,256),&cap);
    for (int i=0; i<ABS_CNT; i++) {
      abscaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // misc caps
  if (evcaps[EV_MSC]) {
    ioctl(fd,EVIOCGBIT(EV_MSC,256),&cap);
    for (int i=0; i<MSC_CNT; i++) {
      msccaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // switch caps
  if (evcaps[EV_SW]) {
    ioctl(fd,EVIOCGBIT(EV_SW,256),&cap);
    for (int i=0; i<SW_CNT; i++) {
      swcaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // LED caps
  if (evcaps[EV_LED]) {
    ioctl(fd,EVIOCGBIT(EV_LED,256),&cap);
    for (int i=0; i<LED_CNT; i++) {
      ledcaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // sound caps
  if (evcaps[EV_SND]) {
    ioctl(fd,EVIOCGBIT(EV_SND,256),&cap);
    for (int i=0; i<SND_CNT; i++) {
      sndcaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // repeat caps
  if (evcaps[EV_REP]) {
    ioctl(fd,EVIOCGBIT(EV_REP,256),&cap);
    for (int i=0; i<REP_CNT; i++) {
      repcaps[i]=!!(cap[i/8]&(1<<(i&7)));
    }
  }
  // absolute properties
  if (evcaps[EV_ABS]) for (int i=0; i<ABS_CNT; i++) {
    if (abscaps[i]) {
      ioctl(fd,EVIOCGABS(i),&absinfo[i]);
    }
  }
  imLogD("%s: %s\n",path.c_str(),name.c_str());
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

  // initialize map if needed
  if (mappings.empty()) {
    mappings.push_back(new bindSet);
    curmap=mappings[0];
    // enable to test.
    /*
    curmap->keybinds[KEY_A].doModify=true;
    curmap->keybinds[KEY_A].actions.push_back(Action(actTurbo,KEY_H,mkts(0,40000000),mkts(0,40000000)));
    */
  }
  return true;
}

void* devThread(void* dev) {
  ((Device*)dev)->run();
  return NULL;
}

bool Device::activate() {
  uinputfd=open("/dev/uinput",O_RDWR);
  if (uinputfd<0) {
    imLogW("couldn't open /dev/uinput for device %s: %s\n",path.c_str(),strerror(errno));
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
    imLogW("couldn't create thread for device %s: %s\n",path.c_str(),strerror(errno));
    close(uinputfd);
    return false;
  }
  active=true;
  imLogI("device %s activated.\n",name.c_str());
  return true;
}

bool Device::deactivate() {
  if (active) {
    imLogD("killing.\n");
    pthread_cancel(inThread);
    pthread_join(inThread,NULL);
    imLogD("%s: ungrabbing.\n",name.c_str());
    if (ioctl(fd,EVIOCGRAB,0)<0) {
      imLogE("%s: couldn't ungrab: %s\n",name.c_str(),strerror(errno));
    }
    close(uinputfd);
    uinputfd=-1;
    imLogI("%s: going down.\n",name.c_str());
    threadRunning=false;
    return true;
  }
  return false;
}

void Device::run() {
  struct timespec ctime;
  struct input_event event;
  struct input_event syncev;
  struct input_event wire;
  activeTurbo* smallest;
  fd_set devfd;
  int amount, count;
  threadRunning=true;
  syncev.type=EV_SYN;
  syncev.code=0;
  syncev.value=0;
  if (ioctl(fd,EVIOCGRAB,1)<0) {
    imLogE("%s: couldn't grab: %s\n",name.c_str(),strerror(errno));
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
    FD_SET(fd,&devfd);
    amount=pselect(fd+1,&devfd,NULL,NULL,(runTurbo.empty())?(NULL):(&ctime),NULL);
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
    count=read(fd,&event,sizeof(struct input_event));
    if (count<0) {
      if (errno==EINTR) { // we got a signal
        break;
      } else {
        imLogW("read error: %s\n",strerror(errno));
        break;
      }
    } 
    imLogD("%s: %d %d %d\n",path.c_str(),event.type,event.code,event.value);
    // EVENT REWIRING BEGIN //
    if (curmap==NULL) {
      wire=event;
      write(uinputfd,&wire,sizeof(struct input_event));
    } else {
      switch (event.type) {
        case EV_KEY:
          if (curmap->keybinds[event.code].doModify) {
            for (auto i: curmap->keybinds[event.code].actions) {
              switch (i.type) {
                case actKey:
                  wire.type=EV_KEY;
                  wire.code=i.code;
                  wire.value=event.value;
                  write(uinputfd,&wire,sizeof(struct input_event));
                  write(uinputfd,&syncev,sizeof(struct input_event));
                  break;
                case actTurbo:
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
                case actSwitchMap: // TODO
                  break;
                case actShiftMap: // TODO
                  break;
                default:
                  imLogW("unknown/unimplemented action %d...\n",i.type);
                  break;
              }
            }
          } else {
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
  imLogD("%s: ungrabbing.\n",name.c_str());
  if (ioctl(fd,EVIOCGRAB,0)<0) {
    imLogE("%s: couldn't ungrab: %s\n",name.c_str(),strerror(errno));
  }
  close(uinputfd);
  uinputfd=-1;
  imLogI("%s: going down.\n",name.c_str());
  threadRunning=false;
}
