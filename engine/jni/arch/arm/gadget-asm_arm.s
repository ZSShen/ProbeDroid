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
GetJniEnv:
    bx  lr


.global GetFuncDeliverException
GetFuncDeliverException:
    bx  lr


.global SetFuncDeliverException
SetFuncDeliverException:
    bx  lr


.global AddIndirectReference
AddIndirectReference:
    bx  lr


.global RemoveIndirectReference
RemoveIndirectReference:
    bx  lr


.global DecodeJObject
DecodeJObject:
    bx  lr


.global ComposeInstrumentGadgetTrampoline
ComposeInstrumentGadgetTrampoline:
    bx  lr


.global ArtQuickInstrumentTrampoline
ArtQuickInstrumentTrampoline:
    bx  lr


.global ArtQuickDeliverExceptionTrampoline
ArtQuickDeliverExceptionTrampoline:
    bx  lr
