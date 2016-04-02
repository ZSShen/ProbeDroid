/*
 * Copyright (C) 2011 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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