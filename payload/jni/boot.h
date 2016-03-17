#ifndef _BOOT_H_
#define _BOOT_H_


#include <jni.h>
#include <sys/stat.h>

#include "scoped_dl.h"


namespace boot {

class Bootstrap
{
  public:
    Bootstrap()
      : dex_path_(nullptr),
        handle_(nullptr)
    {}

    bool ResolveInjectorDeliveredSymbols();
    bool CraftDexPrivatePath();
    bool CreateDexPrivateDir();
    bool CacheJVM();
    bool CacheHotJavaTypes();
    bool LoadAnalysisModule();
    bool ResolveArtSymbol();
    bool DeployInstrumentGadgetComposer();

  private:
    static constexpr int kPrivateDexPerm = S_IRWXU | S_IRWXG | S_IXOTH;
    static constexpr const char* kDirDexData = "data";
    static constexpr const char* kDirInstrument = "instrument";
    static constexpr const char* kDexFileTemp = "DexFile.dex";
    static constexpr const char* kPathLibArt = "/system/lib/libart.so";

    std::unique_ptr<char> dex_path_;
    ScopedDl handle_;
};

}

#endif