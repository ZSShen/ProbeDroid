/**
 *   The MIT License (MIT)
 *   Copyright (C) 2016 ZongXian Shen <andy.zsshen@gmail.com>
 *
 *   Permission is hereby granted, free of charge, to any person obtaining a
 *   copy of this software and associated documentation files (the "Software"),
 *   to deal in the Software without restriction, including without limitation
 *   the rights to use, copy, modify, merge, publish, distribute, sublicense,
 *   and/or sell copies of the Software, and to permit persons to whom the
 *   Software is furnished to do so, subject to the following conditions:
 *
 *   The above copyright notice and this permission notice shall be included in
 *   all copies or substantial portions of the Software.
 *
 *   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 *   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *   IN THE SOFTWARE.
 */

#include "gadget.h"


jint GetCreatedJavaVMs(JavaVM** p_jvm, jsize size, jsize* p_count)
{
    using ARTFUNC = jint(*)(JavaVM**, jsize, jsize*);
    return (reinterpret_cast<ARTFUNC>(g_get_created_java_vms))(p_jvm, size, p_count);
}

jobject AddIndirectReference(IndirectReferenceTable* ref_table,
                             uint32_t cookie, void* obj)
{
    using ARTFUNC = jobject(*)(IndirectReferenceTable*, uint32_t, void*);
    return (reinterpret_cast<ARTFUNC>(g_indirect_reference_table_add))
                                     (ref_table, cookie, obj);
}

bool RemoveIndirectReference(IndirectReferenceTable* ref_table,
                             uint32_t cookie, jobject obj_ref)
{
    using ARTFUNC = bool(*)(IndirectReferenceTable*, uint32_t, jobject);
    return (reinterpret_cast<ARTFUNC>(g_indirect_reference_table_remove))
                                     (ref_table, cookie, obj_ref);
}

void* DecodeJObject(void* thread, jobject obj_ref)
{
    using ARTFUNC = void*(*)(void*, jobject);
    return (reinterpret_cast<ARTFUNC>(g_thread_decode_jobject))(thread, obj_ref);
}
