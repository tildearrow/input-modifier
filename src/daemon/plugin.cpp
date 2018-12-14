#include "imodd.h"
// NOTE: not guaranteed to work with anything that isn't GCC or Clang!

bool LoadedPlugin::loadFile(string path) {
  char* lastError;
  handle=dlopen(path.c_str(),RTLD_NOW);
  if (handle==NULL) {
    imLogW("couldn't load plugin %s: %s\n",path.c_str(),dlerror());
    return false;
  }
  
  dlerror();

  *(void**)&getInfo=dlsym(handle,"_Z17imodPluginGetInfov");
  if ((lastError=dlerror())!=NULL) {
    imLogW("couldn't load plugin %s: (getInfo) %s\n",path.c_str(),lastError);
    dlclose(handle);
    handle=NULL;
    return false;
  }
  *(void**)&init=dlsym(handle,"_Z14imodPluginInitv");
  if ((lastError=dlerror())!=NULL) {
    imLogW("couldn't load plugin %s: (init) %s\n",path.c_str(),lastError);
    dlclose(handle);
    handle=NULL;
    return false;
  }
  *(void**)&quit=dlsym(handle,"_Z14imodPluginQuitv");
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
