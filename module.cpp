#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cxxabi.h>

#include "preamble.h"


#include "sanity_check.cpp"


#include "config.h"


#include "container/mod.h"
#include "context/mod.h"
#include "core/mod.h"


#include "postamble.h"




typedef container::TypeSet<
	ArrayStack<float>,
    exec::Loop
> Requirements;

typedef context::ModuleBundle<
	CPUArrayStack<float>,
	CPUDynArray<float>,
	GPUArrayAlloc<float>,
    CPUArrayAlloc<float>,
    exec::CPULoop,
    CPU
> RootModule;


typedef typename context::CreateContextType<
    RootModule,
    Requirements,
    context::EagerSolve
>::type Context;


struct isA{};
struct isB{};

template<typename CONTEXT>
struct A {
    int x;
    double &get_y() {
        return via<isB>(this).y;
    }
};

template<typename CONTEXT>
struct B {
    double y;
    int &get_x() {
        return via<isA>(this).x;
    }
};

using aModule = context::SimpleModule <
    Meta<A>,
    context::RequirementSet<>,
    context::ImplementationSet<isA>
>;

using bModule = context::SimpleModule <
    Meta<B>,
    context::RequirementSet<>,
    context::ImplementationSet<isB>,
    alloc::StdAllocBytes
>;

typedef context::ModuleBundle<aModule,bModule> rootModule;



int main() {

    typedef context::CreateContextType<
        rootModule,
        container::TypeSet<
            isA,
            isB,
            alloc::AllocBytes
         >,
        context::EagerSolve
    >::type MyContext;
   
    MyContext context(
        A<MyContext>{1234},
        B<MyContext>{5.67}
    );

    as<isA>(context).get_y() *= 2;
    as<isB>(context).get_x() *= 4;

    std::cout << as<isA>(context).x << std::endl;
    std::cout << as<isB>(context).y << std::endl;


    return 0;
}




