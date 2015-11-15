
#--------------------------------------------------------------------------#
#                            Constant Definition                           #
#--------------------------------------------------------------------------#
.equ TLS_OFST_QUICK_INVOKE_VIRTUAL, 0x22c
.equ TLS_OFST_QUICK_RESOLVE_STRING, 0x150


#--------------------------------------------------------------------------#
#                   Linkage with Cpp Site Global Bariable                  #
#--------------------------------------------------------------------------#
.global InvokeVirtualOriginal
.global ResolveStringOriginal
.global ResolveStringProfiling


#--------------------------------------------------------------------------#
#                       Hooking Gadget Implementation                      #
#--------------------------------------------------------------------------#
.section .text

.global HookInvokeVirtual
.type HookInvokeVirtual, @function
HookInvokeVirtual:
    # Prologue for saving context.
    push %ebp
    movl %esp, %ebp
    pusha
    pushf

    # The first argument should be filled with the address of original function.
    movl 8(%ebp), %esi

    # The second argument is the address of hooking function.
    movl 12(%ebp), %edi

    # Acquire the address of original function.
    movl %fs:TLS_OFST_QUICK_INVOKE_VIRTUAL, %eax
    movl %eax, (%esi)

    # Replace that TLS entry with the address of hooking function.
    movl %edi, %fs:TLS_OFST_QUICK_INVOKE_VIRTUAL

    # Epilogue for restoring context.
    popf
    popa
    leave
    ret


.global HookResolveString
.type HookResolveString, @function
HookResolveString:
    # Prologue for saving context.
    push %ebp
    movl %esp, %ebp
    pusha
    pushf

    # The first argument should be filled with the address of original function.
    movl 8(%ebp), %esi

    # The second argument is the address of hooking function.
    movl 12(%ebp), %edi

    # Acquire the address of original function.
    movl %fs:TLS_OFST_QUICK_RESOLVE_STRING, %eax
    movl %eax, (%esi)

    # Replace that TLS entry with the address of hooking function.
    movl %edi, %fs:TLS_OFST_QUICK_RESOLVE_STRING

    # Epilogue for restoring context.
    popf
    popa
    leave
    ret


.global InvokeVirtualFake
.type InvokeVirtualFake, @function
InvokeVirtualFake:
    call InvokeVirtualOriginal
    ret


.global ResolveStringFake
.type ResolveStringFake, @function
ResolveStringFake:
    # Now it is just a dummy hook.
    pusha
    pushf
    subl $8, %esp
    movl %ecx, 0(%esp)
    movl %eax, 4(%esp)
    call ResolveStringProfiling
    addl $8, %esp
    popf
    popa

    call *ResolveStringOriginal

    ret
