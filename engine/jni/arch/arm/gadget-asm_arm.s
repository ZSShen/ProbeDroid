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
.equ TLS_OFST_DELIVER_EXCEPTION, 0x234


#--------------------------------------------------------------------------#
#                       Utility Gadget Implementation                      #
#--------------------------------------------------------------------------#
.global GetFuncDeliverException
GetFuncDeliverException:
    ldr r1, [r9, #TLS_OFST_DELIVER_EXCEPTION]
    str r1, [r0]
    bx  lr


.global SetFuncDeliverException
SetFuncDeliverException:
    str r0, [r9, #TLS_OFST_DELIVER_EXCEPTION]
    bx  lr


.global ComposeInstrumentGadgetTrampoline
ComposeInstrumentGadgetTrampoline:
    @ r0 stores ArtMethod* pointer
    @ r1 stores the receiver pointer
    @ r2 stores the first argument
    push {lr}
    bl ComposeInstrumentGadget
    pop {lr}
    bx  lr


.global ArtQuickInstrumentTrampoline
ArtQuickInstrumentTrampoline:
    push {r0, r1, r2, r3, lr}
    sub sp, sp, #24

    @ Craft the first argument.
    ldr r0, [sp, #32]
    str r0, [sp, #0]

    @ Craft the second argument.
    ldr r0, [sp, #36]
    str r0, [sp, #4]

    @ Craft the stack pointer.
    add r0, sp, #44
    str r0, [sp, #8]

    @ Craft the return data type.
    add r0, sp, #12

    @ Craft the return value.
    add r1, sp, #16

    @ Craft the ArtMethod* pointer.
    ldr r2, [sp, #24]

    @ Craft the receiver pointer.
    ldr r3, [sp, #28]

    bl ArtQuickInstrument

    @ Store the return value in the r0/r1 register pair.
    ldrd r0, r1, [sp, #16]

    @ Restore the stack layout.
    add sp, sp, #40
    pop {lr}

    bx  lr


.global ArtQuickDeliverExceptionTrampoline
ArtQuickDeliverExceptionTrampoline:
    push {lr}
    bl ArtQuickDeliverException
    pop {lr}
    bx  lr
