
#include "logcat.h"
#include "../string.h"


using namespace art;

OriginalCall indexOf_javalangString_I_;

void indexOf_javalangString_I_PreCall(void* strref, int from_idx)
{
    const String* str = reinterpret_cast<const String*>(strref);
    std::string content = String::ToModifiedUtf8(str);
    LOGD("[+] String.indexOf() Ljava/lang/String=%s, I=%d\n", content.c_str(), from_idx);
}

void indexOf_javalangString_I_PostCall(int idx)
{
    LOGD("[+] String.indexOf() ret=%d\n", idx);
}

bool Gadget_java_lang_String::indexOf_javalangString_I_Patcher(JNIEnv* env)
{
    // TODO: Handle the class or method not found exception.
    // Resolve the target ArtMethod* structure.
    jclass clazz = env->FindClass("java/lang/String");
    jmethodID method_id = env->GetMethodID(clazz, "indexOf",
                                           "(Ljava/lang/String;I)I");
    ArtMethod* art_method = reinterpret_cast<ArtMethod*>(method_id);

    // Swap the entry point to the quick compiled code.
    uint64_t entry = ArtMethod::GetEntryPointFromQuickCompiledCode(art_method);
    indexOf_javalangString_I_ = reinterpret_cast<OriginalCall>(entry);
    entry = reinterpret_cast<uint64_t>(indexOf_javalangString_I_Trampoline);
    ArtMethod::SetEntryPointFromQuickCompiledCode(art_method, entry);

    return true;
}
