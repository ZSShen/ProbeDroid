
#--------------------------------------------------------------------------#
#                            Constant Definition                           #
#--------------------------------------------------------------------------#
.equ TLS_OFST_JNIENVEXT, 0x98


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

    movl %esp, %esi         # Pass the address of the first DOWRD which should
    addl $28, %esi          # be updated with the data type of the return value.
    push %esi

    subl $4, %esi           # Pass the address of the second QWORD which should
    push %esi               # be updated with the return value.

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
    movl 4(%esp), %eax      # less than or equal to 32 bit, put the return value
    jmp EXIT                # in %eax.

DWORD_FLOAT:
    movd 4(%esp), %xmm0     # For the floating point data type with width equal
    jmp EXIT                # to 32 bit, put the return value in %xmm0.

QWORD_LONG:                 # For the non floating point data type with width
    movl 4(%esp), %eax      # equal to 64 bit, put the return value in %edx:%eax.
    movl (%esp), %edx
    jmp EXIT

QWORD_DOUBLE:               # For the floating point data type with width equal
    movq (%esp), %xmm0      # to 64 bit, put the return value in %xmm0.

EXIT:
    addl $12, %esp
    pop %esi
    ret
