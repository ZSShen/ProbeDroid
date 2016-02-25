#ifndef _JNI_INTERNAL_H_
#define _JNI_INTERNAL_H_


// A mirror of the Runtime managed JNI handle.
struct JNIEnvExt : public JNIEnv
{
    // Original Member: Thread* const self
    void* thread_;

    // Original Member: JavaVMExt* vm
    void* jvm_;

    // Cookie used when using the local indirect reference table.
    uint32_t local_ref_cookie_;

    // Original Member: IndirectReferenceTable locals
    void* local_refs_table_;
};


#endif