#ifndef _GADGET_X86_H_
#define _GADGET_X86_H_


#include <cstdint>
#include <vector>
#include <memory>
#include <jni.h>

#include "indirect_reference_table.h"
#include "jni_internal.h"


static const constexpr int32_t kStackAlignment = 5 + 3 + 1;
static const int32_t kMinJniArgCount = 3;


// TODO: Need to redesign the constructor for exception safety later.
class InputMarshaller
{
  public:
    InputMarshaller(JNIEnv *env, int32_t count_input, int32_t unboxed_input_width,
                    const std::vector<char>& ref_input_type, void* receiver,
                    void* reg_first, void* reg_second, void** stk_ptr)
      : count_input_(count_input),
        unboxed_input_width_(unboxed_input_width),
        receiver_(receiver),
        reg_first_(reg_first),
        reg_second_(reg_second),
        stk_ptr_(stk_ptr),
        env_(reinterpret_cast<JNIEnvExt*>(env)),
        cookie_(env_->local_ref_cookie_),
        ref_table_(reinterpret_cast<IndirectReferenceTable*>(&(env_->local_refs_table_))),
        thread_(env_->thread_),
        boxed_inputs_(nullptr),
        ref_input_type_(ref_input_type),
        unboxed_inputs_((count_input > 0)? new void*[unboxed_input_width] : nullptr),
        gen_types_(new void*[count_input + kMinJniArgCount]),
        gen_values_(new void*[count_input + kMinJniArgCount])
    {}

    void Flatten();
    bool BoxInputs();
    void UnboxInputs();

    jobjectArray GetBoxedInputs()
    {
        return boxed_inputs_;
    }

    void** GetGenericTypeArray()
    {
        return gen_types_.get();
    }

    void** GetGenericValueArray()
    {
        return gen_values_.get();
    }

  private:
    int32_t count_input_;
    int32_t unboxed_input_width_;
    void* receiver_;
    void* reg_first_;
    void* reg_second_;
    void** stk_ptr_;

    JNIEnvExt* env_;
    uint32_t cookie_;
    IndirectReferenceTable* ref_table_;
    void* thread_;

    jobjectArray boxed_inputs_;
    const std::vector<char>& ref_input_type_;
    std::unique_ptr<void*[]> unboxed_inputs_;
    std::unique_ptr<void*[]> gen_types_;
    std::unique_ptr<void*[]> gen_values_;
};

#endif