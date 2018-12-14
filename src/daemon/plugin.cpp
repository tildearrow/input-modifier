#include "imodd.h"

bool LoadedPlugin::loadFile(string path) {
  char* lastError;
  handle=dlopen(path.c_str(),RTLD_NOW);
  if (handle==NULL) {
    imLogW("couldn't load plugin %s: %s\n",path.c_str(),strerror(errno));
    return false;
  }
  
  dlerror();

  *(void**)&getInfo=dlsym(handle,"imodPluginGetInfo");
  if ((lastError=dlerror())!=NULL) {
    imLogW("couldn't load plugin %s: (getInfo) %s\n",path.c_str(),lastError);
    dlclose(handle);
    handle=NULL;
    return false;
  }
  *(void**)&init=dlsym(handle,"imodPluginInit");
  if ((lastError=dlerror())!=NULL) {
    imLogW("couldn't load plugin %s: (init) %s\n",path.c_str(),lastError);
    dlclose(handle);
    handle=NULL;
    return false;
  }
  *(void**)&quit=dlsym(handle,"imodPluginQuit");
  if ((lastError=dlerror())!=NULL) {
    imLogW("couldn't load plugin %s: (quit) %s\n",path.c_str(),lastError);
    dlclose(handle);
    handle=NULL;
    return false;
  }

  info=getInfo();
  if (info==NULL) {
    imLogW("couldn't load plugin %s: couldn't get plugin info\n",path.c_str());
    dlclose(handle);
    handle=NULL;
    return false;
  }

  return true;
}
