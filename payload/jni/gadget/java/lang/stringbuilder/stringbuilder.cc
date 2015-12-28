
#include "logcat.h"
#include "../stringbuilder.h"


using namespace art;

OriginalCall toString;

void toStringPreCall()
{
    return;
}

void toStringPostCall(void* objref)
{
    const String* str = reinterpret_cast<const String*>(objref);
    std::string content = String::ToModifiedUtf8(str);
    LOGD("[+] StringBuilder.toString() ret=%s\n", content.c_str());
}

bool Gadget_java_lang_StringBuilder::toStringPatcher(JNIEnv* env)
{
    // TODO: Handle the class or method not found exception.
    // Resolve the target ArtMethod* structure.
    jclass clazz = env->FindClass("java/lang/StringBuilder");
    jmethodID method_id = env->GetMethodID(clazz, "toString",
                                           "()Ljava/lang/String;");
    ArtMethod* art_method = reinterpret_cast<ArtMethod*>(method_id);

    // Swap the entry point to the quick compiled code.
    uint64_t entry = ArtMethod::GetEntryPointFromQuickCompiledCode(art_method);
    toString = reinterpret_cast<OriginalCall>(entry);
    entry = reinterpret_cast<uint64_t>(toStringTrampoline);
    ArtMethod::SetEntryPointFromQuickCompiledCode(art_method, entry);

    return true;
}
