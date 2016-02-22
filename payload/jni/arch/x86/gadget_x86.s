
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
    # Prologue for saving context.
    push %ebp
    movl %esp, %ebp
    pusha
    pushf

    movl 8(%ebp), %esi
    movl %fs:TLS_OFST_JNIENVEXT, %eax
    movl %eax, (%esi)

    # Epilogue for restoring context.
    popf
    popa
    leave
    ret
