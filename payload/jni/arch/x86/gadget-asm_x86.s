
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
    push %esp               # The stack pointer
    push %ebx               # The second argument
    push %edx               # The first argument
    push %eax               # The ArtMethod* pointer
    push %ecx               # The receiver pointer
    call ComposeInstrumentGadget
    addl $20, %esp
    ret


.global ArtQuickInstrumentTrampoline
.type ArtQuickInstrumentTrampoline, @function
ArtQuickInstrumentTrampoline:
    push %esp               # The stack pointer
    push %ebx               # The second argument
    push %edx               # The first argument
    push %eax               # The ArtMethod* pointer
    push %ecx               # The receiver pointer
    call ArtQuickInstrument
    addl $20, %esp
    ret
