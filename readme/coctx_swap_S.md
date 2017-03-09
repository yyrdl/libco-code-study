# coctx_swap.S

在这个汇编文件定义了函数coctx_swap 函数（co_routine context swap）。该函数的作用是
保存当前co_routine的执行环境到结构体coctx_t ，然后将CPU上下文设置为目标co_routine的上下文.

# 相关知识

## 两个寄存器

* esp 栈顶指针寄存器，指向调用栈的栈顶（始终指向，意味着栈分配到哪里了，从当前栈往高地址是已被分配了的）
* ebp 基址指针寄存器，指向当前活动栈帧的基址

>一个function 调用会在栈上生成一个record ,称之为栈帧

## function 调用与栈活动(正常的情况下)

* 1.将传给被调用函数的参数从右至左压栈
* 2.将返回地址压栈，返回地址即函数调用结束后要执行的下一条指令的地址
* 3.将当前EBP 寄存器里的值压栈
* 4.将EBP寄存器的值设为ESP寄存器保存的值
* 5.在栈里为当前函数局部变量分配所需的空间，表现为修改ESP寄存器的值（一般是减4的整数倍,栈分配是从高地址向低地址分配）

## 其他

* [X86 REGISTERS](https://en.wikibooks.org/wiki/X86_Assembly/X86_Architecture)

* [FUNCTIONS and STACK FRAME](https://en.wikibooks.org/wiki/X86_Disassembly/Functions_and_Stack_Frames)

* [计算机组成原理](http://product.dangdang.com/23793048.html)


# 源码注解

只注解了32位部分，64位部分内容一致

```s
.globl coctx_swap
#if !defined( __APPLE__ )
.type  coctx_swap, @function
#endif
coctx_swap:

#if defined(__i386__)
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

```

# 问题

* 1.随意改变ESP的值不会导致栈分配冲突么？是怎么解决的？to be continue...

答： 运行时栈区不再由系统分配，而是由函数co_create_env 分配，co_create_env会分配最小 128kb最大8MB的内存空间作为一个co_routine的运行时栈空间。
每次co_routine调度时，系统（汇编部分coctx_swap方法）会保存当前co_routine的栈区到内存，并切换栈区基地址和栈顶地址指针到目标co_routine的基地址和栈顶地址（用过设置对应寄存器的值）。这样就不可能导致栈空间分配冲突。
