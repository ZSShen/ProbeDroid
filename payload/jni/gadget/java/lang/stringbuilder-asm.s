
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

    call *toString

    pusha
    pushf
    push %eax
    call toStringPostCall
    addl $4, %esp
    popf
    popa

    ret
