/*
* Tencent is pleased to support the open source community by making Libco available.

* Copyright (C) 2014 THL A29 Limited, a Tencent company. All rights reserved.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*	http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing,
* software distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
/*
   https://en.wikibooks.org/wiki/X86_Assembly/GAS_Syntax
   esp ESP为栈指针，用于指向栈的栈顶（下一个压入栈的活动记录的顶部） 当一个函数内出现函数调用的时候，会在这个函数的汇编代码中更新esp的值，因为存在函数调用意味着有新的活动记录会压栈

   eax EAX 是"累加器"(accumulator), 它是很多加法乘法指令的缺省寄存器。
   ebx EBX 是"基地址"(base)寄存器, 在内存寻址时存放基地址。
   ecx ECX 是计数器(counter), 是重复(REP)前缀指令和LOOP指令的内定计数器。
   edx EDX 则总是被用来放整数除法产生的余数。
   esi/edi ESI/EDI分别叫做"源/目标索引寄存器"(source/destination index),因为在很多字符串操作指令中, DS:ESI指向源串,而ES:EDI指向目标串.
   ebp EBP是"基址指针"(BASE POINTER), 它最经常被用作高级语言函数调用的"框架指针"(frame pointer). 在破解的时候,经常可以看见一个标准的函数起始代码:


   coctx_swap的作用是保存当前routine的执行环境到结构体coctx_t ，然后将CPU上下文设置为目标routine的上下文

*/

.globl coctx_swap
#if !defined( __APPLE__ )
.type  coctx_swap, @function
#endif
coctx_swap:

#if defined(__i386__)//在执行下面的汇编代码之前已经执行了将参数压栈，将返回地址压栈的操作，当前esp指向的内存的值应该为下一条指令的地址，即调用coctx_swap之后的指令（见co_routine.cpp co_swap函数）
	leal 4(%esp), %eax //sp   R[eax]=R[esp]+4 R[eax]的值应该为coctx_swap的第一个参数在栈中的地址
	movl 4(%esp), %esp  //    R[esp]=Mem[R[esp]+4] 将esp指向 &(curr->ctx) 当前routine 上下文的内存地址，ctx在堆区，现在esp应指向reg[0]
	leal 32(%esp), %esp //parm a : &regs[7] + sizeof(void*)   push 操作是以esp的值为基准，push一个值,则esp的值减一个单位（因为是按栈区的操作逻辑，从高位往低位分配地址），但ctx是在堆区，所以应将esp指向reg[7]，然后从eax到-4(%eax)push
    //保存寄存器值到栈中，实际对应coctx_t->regs 数组在栈中的位置（参见coctx.h 中coctx_t的定义）
	pushl %eax //esp ->parm a

	pushl %ebp
	pushl %esi
	pushl %edi
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl -4(%eax) //将函数返回地址压栈，即coctx_swap 之后的指令地址，保存返回地址,保存到coctx_t->regs[0]

    //恢复运行目标routine时的环境（各个寄存器的值和栈状态）
	movl 4(%eax), %esp //parm b -> &regs[0] //切换esp到目标 routine  ctx在栈中的起始地址,这个地址正好对应regs[0],pop一次 esp会加一个单位的值

	popl %eax  //ret func addr regs[0] 暂存返回地址到 EAX
	//恢复当时的寄存器状态
	popl %ebx  // regs[1]
	popl %ecx  // regs[2]
	popl %edx  // regs[3]
	popl %edi  // regs[4]
	popl %esi  // regs[5]
	popl %ebp  // regs[6]
	popl %esp  // regs[7]
	//将返回地址压栈
	pushl %eax //set ret func addr
    //将 eax清零
	xorl %eax, %eax
	//返回，这里返回之后就切换到目标routine了，C++代码中调用coctx_swap的地方之后的代码将得不到立即执行
	ret

#elif defined(__x86_64__)
	leaq 8(%rsp),%rax
	leaq 112(%rdi),%rsp
	pushq %rax
	pushq %rbx
	pushq %rcx
	pushq %rdx

	pushq -8(%rax) //ret func addr

	pushq %rsi
	pushq %rdi
	pushq %rbp
	pushq %r8
	pushq %r9
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15

	movq %rsi, %rsp
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %r9
	popq %r8
	popq %rbp
	popq %rdi
	popq %rsi
	popq %rax //ret func addr
	popq %rdx
	popq %rcx
	popq %rbx
	popq %rsp
	pushq %rax

	xorl %eax, %eax
	ret
#endif
