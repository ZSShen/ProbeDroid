#ifndef _GADGET_X86_H_
#define _GADGET_X86_H_


#include <cstdint>
#include <vector>
#include <memory>
#include <jni.h>


// TODO: Need to redesign the constructor for exception safetly later.
class InputMarshaller
{
  public:
    InputMarshaller(uint32_t count_input, uint32_t unboxed_input_width,
                    const std::vector<char>& ref_input_type, void* obj_ptr,
                    void* reg_first, void* reg_second, void** stk_ptr)
     : count_input_(count_input),
       unboxed_input_width_(unboxed_input_width),
       obj_ptr_(obj_ptr),
       reg_first_(reg_first),
       reg_second_(reg_second),
       stk_ptr_(stk_ptr),
       boxed_inputs_(nullptr),
       ref_input_type_(ref_input_type),
       unboxed_inputs_((count_input > 0)? new char[unboxed_input_width] : nullptr),
       gen_types_((count_input > 0)? new void*[count_input] : nullptr),
       gen_values_((count_input > 0)? new void*[count_input] : nullptr)
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
    uint32_t count_input_;
    uint32_t unboxed_input_width_;
    void* obj_ptr_;
    void* reg_first_;
    void* reg_second_;
    void** stk_ptr_;
    jobjectArray boxed_inputs_;
    const std::vector<char>& ref_input_type_;
    std::unique_ptr<char[]> unboxed_inputs_;
    std::unique_ptr<void*[]> gen_types_;
    std::unique_ptr<void*[]> gen_values_;
};

#endif