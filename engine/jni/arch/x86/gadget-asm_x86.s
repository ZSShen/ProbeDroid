###
#   The MIT License (MIT)
#   Copyright (C) 2016 ZongXian Shen <andy.zsshen@gmail.com>
#
#   Permission is hereby granted, free of charge, to any person obtaining a
#   copy of this software and associated documentation files (the "Software"),
#   to deal in the Software without restriction, including without limitation
#   the rights to use, copy, modify, merge, publish, distribute, sublicense,
#   and/or sell copies of the Software, and to permit persons to whom the
#   Software is furnished to do so, subject to the following conditions:
#
#   The above copyright notice and this permission notice shall be included in
#   all copies or substantial portions of the Software.
#
#   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
#   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
#   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
#   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
#   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
#   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
#   IN THE SOFTWARE.
###

#--------------------------------------------------------------------------#
#                            Constant Definition                           #
#--------------------------------------------------------------------------#
.equ TLS_OFST_JNIENVEXT, 0x98
.equ TLS_OFST_DELIVER_EXCEPTION, 0x234


#--------------------------------------------------------------------------#
#                       Utility Gadget Implementation                      #
#--------------------------------------------------------------------------#
.global GetJniEnv
.type GetJniEnv, @function
GetJniEnv:
    push %ebp
    movl %esp, %ebp
    pusha
    pushf

    movl 8(%ebp), %esi
    movl %fs:TLS_OFST_JNIENVEXT, %eax
    movl %eax, (%esi)

    popf
    popa
    leave
    ret


.global GetFuncDeliverException
.type GetFuncDeliverException, @function
GetFuncDeliverException:
    push %ebp
    movl %esp, %ebp
    pusha
    pushf

    movl 8(%ebp), %esi
    movl %fs:TLS_OFST_DELIVER_EXCEPTION, %eax
    movl %eax, (%esi)

    popf
    popa
    leave
    ret


.global SetFuncDeliverException
.type SetFuncDeliverException, @function
SetFuncDeliverException:
    push %ebp
    movl %esp, %ebp
    pusha
    pushf

    movl 8(%ebp), %eax
    movl %eax, %fs:TLS_OFST_DELIVER_EXCEPTION

    popf
    popa
    leave
    ret


.global AddIndirectReference
.type AddIndirectReference, @function
AddIndirectReference:
    push %ebp
    movl %esp, %ebp

    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    pushf

    movl  8(%ebp), %eax     # The pointer to the designated table
    movl 12(%ebp), %ebx     # The cookie
    movl 16(%ebp), %ecx     # The pointer to the designated object
    push %ecx
    push %ebx
    push %eax
    call *g_indirect_reference_table_add
    add $12, %esp

    popf
    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx

    leave
    ret


.global RemoveIndirectReference
.type RemoveIndirectReference, @function
RemoveIndirectReference:
    push %ebp
    movl %esp, %ebp

    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    pushf

    movl  8(%ebp), %eax     # The pointer to the designated table
    movl 12(%ebp), %ebx     # The cookie
    movl 16(%ebp), %ecx     # The designated indirect reference
    push %ecx
    push %ebx
    push %eax
    call *g_indirect_reference_table_remove
    add $12, %esp

    popf
    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx

    leave
    ret


.global DecodeJObject
.type DecodeJObject, @function
DecodeJObject:
    push %ebp
    movl %esp, %ebp

    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    pushf

    movl  8(%ebp), %eax     # The pointer to the current thread
    movl 12(%ebp), %ebx     # The designated indirect reference
    push %ebx
    push %eax
    call *g_thread_decode_jobject
    add $8, %esp

    popf
    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx

    leave
    ret


.global ComposeInstrumentGadgetTrampoline
.type ComposeInstrumentGadgetTrampoline, @function
ComposeInstrumentGadgetTrampoline:
    push %edx               # The first argument
    push %eax               # The ArtMethod* pointer
    push %ecx               # The receiver pointer
    call ComposeInstrumentGadget
    addl $12, %esp
    ret


.global ArtQuickInstrumentTrampoline
.type ArtQuickInstrumentTrampoline, @function
ArtQuickInstrumentTrampoline:
                            # Since we need %esi to release the register pressure,
    push %esi               # we stash its content on the stack temporarily.

    subl $12, %esp          # Create the space for 3 local DWORDs.

    push %esp               # The stack pointer (should be accessed via
                            # %esp + 5 + 3 + 1 to retrieve the passed arguments.)
    push %ebx               # The second argument
    push %edx               # The first argument
    push %eax               # The ArtMethod* pointer
    push %ecx               # The receiver pointer

    movl %esp, %esi         # Pass the address of the first QWORD which should
    addl $20, %esi          # be updated with the return value.
    push %esi

    addl $8, %esi           # Pass the address of the second DOWRD which should
    push %esi               # be updated with the data type of the return value.

    call ArtQuickInstrument

    addl $28, %esp          # To fit the calling convention defined by Android
    movl 8(%esp), %esi      # Runtime, we must store the return value to
    cmpl kNoData, %esi      # different registers based on its data type.
    je EXIT

    cmpl kQwordDouble, %esi
    je QWORD_DOUBLE
    cmpl kQwordLong, %esi
    je QWORD_LONG
    cmpl kDwordFloat, %esi
    je DWORD_FLOAT

DWORD_INT:                  # For the non floating point data type with width
    movl (%esp), %eax       # less than or equal to 32 bit, put the return value
    jmp EXIT                # in %eax.

DWORD_FLOAT:
    movd (%esp), %xmm0      # For the floating point data type with width equal
    jmp EXIT                # to 32 bit, put the return value in %xmm0.

QWORD_LONG:                 # For the non floating point data type with width
    movl (%esp), %eax       # equal to 64 bit, put the return value in %edx:%eax.
    movl 4(%esp), %edx
    jmp EXIT

QWORD_DOUBLE:               # For the floating point data type with width equal
    movq (%esp), %xmm0      # to 64 bit, put the return value in %xmm0.

EXIT:
    addl $12, %esp
    pop %esi
    ret


.global ArtQuickDeliverExceptionTrampoline
.type ArtQuickDeliverExceptionTrampoline, @function
ArtQuickDeliverExceptionTrampoline:
    push %eax
    call ArtQuickDeliverException
    add $4, %esp
    ret
