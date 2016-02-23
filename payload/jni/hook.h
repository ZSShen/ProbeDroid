#ifndef _HOOK_H_
#define _HOOK_H_


#include <jni.h>


namespace hook {

enum {
    HOOK_SUCCESS = 0,
    HOOK_FAILURE = 1
};

class Penetrator
{
  public:
    Penetrator()
      : module_path_(nullptr),
        dex_path_(nullptr)
    {}

    bool CraftAnalysisModulePath();
    bool CraftDexPrivatePath();
    bool CreateDexPrivateDir();
    bool CacheJVM();
    bool LoadAnalysisModule();

  private:
    void LogJNIException(JNIEnv*, jthrowable);

    static constexpr int kPrivateDexPerm = S_IRWXU | S_IRWXG | S_IXOTH;
    static constexpr const char* kDirDexData = "data";
    static constexpr const char* kDirInstrument = "instrument";
    static constexpr const char* kRecursiveExcept = "Exception occurs during "
                                                "exception message generation.";

    char* module_path_;
    std::unique_ptr<char> dex_path_;
    JavaVM* jvm_;
};

}

#endif