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





struct isA{};
struct isB{};




template<typename CONTEXT>
struct A {
    int x;
    double &get_y() {
        return via<isB>(this).y;
    }
};

using aModule = context::SimpleModule <
    Meta<A>,
    context::RequirementSet<>,
    context::ImplementationSet<isA>
>;




template<typename CONTEXT>
struct B {
    double y;
    int &get_x() {
        return via<isA>(this).x;
    }
};


using bModule = context::SimpleModule <
    Meta<B>,
    context::RequirementSet<>,
    context::ImplementationSet<isB>
>;

typedef context::ModuleBundle<
    aModule,
    bModule,
    alloc::StdAllocBytes,
    platform::CPU
> rootModule;



int main() {

    using alloc::AllocBytes;

    typedef context::CreateContextType<
        rootModule,
        container::TypeSet<
            isA,
            isB,
            AllocBytes
         >,
        context::EagerSolve
    >::type Ctx;
   
    Ctx ctx(
        B<Ctx>{5.67},
        A<Ctx>{1234}
    );

    as<isA>(ctx).get_y() *= 2;
    as<isB>(ctx).get_x() *= 4;

    std::cout << as<isA>(ctx).x << std::endl;
    std::cout << as<isB>(ctx).y << std::endl;

    void *ptr = as<AllocBytes>(ctx).alloc_bytes(10);
    as<AllocBytes>(ctx).free_bytes(ptr);
    
    return 0;
}




