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

#ifndef __CO_CLOSURE_H__
#define __CO_CLOSURE_H__
struct stCoClosure_t
{
public:
	virtual void exec() = 0;
};

//1.base
//-- 1.1 comac_argc
// __VA_ARGS__ C99 新增的，是一个可变参数宏，

#define comac_get_args_cnt( ... ) comac_arg_n( __VA_ARGS__ )

#define comac_arg_n( _1,_2,_3,_4,_5,_6,_7,N,...) N
#define comac_args_seqs() 7,6,5,4,3,2,1,0
#define comac_join_1( x,y ) x##y  //连接 xy 成为一个整体

#define comac_argc( ... ) comac_get_args_cnt( __VA_ARGS__,comac_args_seqs() ) //计算可变参数的个数
//: comac_get_args_cnt(__VA_ARGS__,7,6,5,4,3,2,1,0)  : comac_arg_n(__VA_ARGS__) //保证末尾是 7,6,5,4,3,2,1,0

#define comac_join( x,y) comac_join_1( x,y )
//: x##y

//-- 1.2 repeat
#define repeat_0( fun,a,... )
#define repeat_1( fun,a,... ) fun( 1,a,__VA_ARGS__ ) repeat_0( fun,__VA_ARGS__ )
#define repeat_2( fun,a,... ) fun( 2,a,__VA_ARGS__ ) repeat_1( fun,__VA_ARGS__ )
#define repeat_3( fun,a,... ) fun( 3,a,__VA_ARGS__ ) repeat_2( fun,__VA_ARGS__ )
#define repeat_4( fun,a,... ) fun( 4,a,__VA_ARGS__ ) repeat_3( fun,__VA_ARGS__ )
#define repeat_5( fun,a,... ) fun( 5,a,__VA_ARGS__ ) repeat_4( fun,__VA_ARGS__ )
#define repeat_6( fun,a,... ) fun( 6,a,__VA_ARGS__ ) repeat_5( fun,__VA_ARGS__ )

#define repeat( n,fun,... ) comac_join( repeat_,n )( fun,__VA_ARGS__)
//comac_join1(repeat_,n)(fun,__VA_ARGS__)
// 假如n 是1 则

//:repeat_1(fun,__VA_ARGS__)
//:fun(1,a,...) repeat_0(fun,__VA_ARGS__)
//:fun(1,a,...)

//假如n 是2，则展开后如下
//:repeat_2(fun,a,...)
//:fun(2,a,b,...)repeat_1(fun,__VA_ARGS__)
//:fun(2,a,b,...)fun(1,a,...)repeat_0(fun,__VA_ARGS__)
//:fun(2,a,b,...)fun(1,a,...)

//2.implement
#define decl_typeof( i,a,... ) typedef typeof( a ) typeof_##a;//推断类型定义
#define impl_typeof( i,a,... ) typeof_##a & a;
#define impl_typeof_cpy( i,a,... ) typeof_##a a;
#define con_param_typeof( i,a,... ) typeof_##a & a##r,
#define param_init_typeof( i,a,... ) a(a##r),

/*
   a piece of code

   #include <iostream>
   #define N( x,y ) (at_##x+y)
   #define B(x,y) N(x,y)
   using namespace std;

    int main(){
       int at_a=10,a=1,b=2;
        cout <<B(a,b)<< endl;//输出12
      return 0;
   }
   //宏定义是编译时解释成纯C或C++源码

*/
//2.1 reference

#define co_ref( name,... )\
repeat( comac_argc(__VA_ARGS__) ,decl_typeof,__VA_ARGS__ )\//批量定义类型列表
class type_##name\
{\
public:\
	repeat( comac_argc(__VA_ARGS__) ,impl_typeof,__VA_ARGS__ )\
	int _member_cnt;\
	type_##name( \
		repeat( comac_argc(__VA_ARGS__),con_param_typeof,__VA_ARGS__ ) ... ): \
		repeat( comac_argc(__VA_ARGS__),param_init_typeof,__VA_ARGS__ ) _member_cnt(comac_argc(__VA_ARGS__)) \
	{}\
} name( __VA_ARGS__ ) ;




//2.2 function

#define co_func(name,...)\
repeat( comac_argc(__VA_ARGS__) ,decl_typeof,__VA_ARGS__ )\
class name:public stCoClosure_t\
{\
public:\
	repeat( comac_argc(__VA_ARGS__) ,impl_typeof_cpy,__VA_ARGS__ )\
	int _member_cnt;\
public:\
	name( repeat( comac_argc(__VA_ARGS__),con_param_typeof,__VA_ARGS__ ) ... ): \
		repeat( comac_argc(__VA_ARGS__),param_init_typeof,__VA_ARGS__ ) _member_cnt(comac_argc(__VA_ARGS__))\
	{}\
	void exec()

#define co_func_end }


#endif

