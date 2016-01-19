
#--------------------------------------------------------------------------#
#                   Linkage with Cpp Site Global Variable                  #
#--------------------------------------------------------------------------#
.global toStringPreCall
.global toStringPostCall

#--------------------------------------------------------------------------#
#                         Trampoline Implementation                        #
#--------------------------------------------------------------------------#
.section .text

.global toStringTrampoline
.type toStringTrampoline, @function
toStringTrampoline:
    pusha
    pushf
    call toStringPreCall
    popf
    popa

    pusha
    pushf
    push %esi
    call StashESI
    addl $4, %esp
    popf
    popa

    pop %esi
    pusha
    pushf
    push %esi
    call StashReturnAddress
    addl $4, %esp
    popf
    popa

    call *toString

    push %esi
    pusha
    pushf
    call RestoreReturnAddress
    movl %eax, 36(%esp)
    popf
    popa

    push %esi
    pusha
    pushf
    call RestoreESI
    movl %eax, 36(%esp)
    popf
    popa
    pop %esi

    pusha
    pushf
    push %eax
    call toStringPostCall
    addl $4, %esp
    popf
    popa

    ret
