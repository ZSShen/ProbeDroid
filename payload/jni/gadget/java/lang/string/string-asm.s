
#--------------------------------------------------------------------------#
#                   Linkage with Cpp Site Global Variable                  #
#--------------------------------------------------------------------------#
.global indexOf_javalangString_I_PreCall
.global indexOf_javalangString_I_PostCall

#--------------------------------------------------------------------------#
#                         Trampoline Implementation                        #
#--------------------------------------------------------------------------#
.section .text

.global indexOf_javalangString_I_Trampoline
.type indexOf_javalangString_I_Trampoline, @function
indexOf_javalangString_I_Trampoline:
    pusha
    pushf
    push %ebx
    push %edx
    call indexOf_javalangString_I_PreCall
    addl $8, %esp
    popf
    popa

    call *indexOf_javalangString_I_

    pusha
    pushf
    push %eax
    call indexOf_javalangString_I_PostCall
    addl $4, %esp
    popf
    popa

    ret
