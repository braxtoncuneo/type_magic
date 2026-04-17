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




// Traits
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
    bModule
> rootModule;


struct Bad {};
struct Worse {};


template<typename CTX>
void run() {
 
    if constexpr (CTX::Info::SATISFIED) {
        
        CTX ctx(
            As<isA,CTX>{1234},
            As<isB,CTX>{5.67}
        );

        as<isA>(ctx).get_y() *= 2;
        as<isB>(ctx).get_x() *= 4;

        std::cout << as<isA>(ctx).x << std::endl;
        std::cout << as<isB>(ctx).y << std::endl;

    } else {
        CTX ctx;
        std::cout << as<context::ContextInfo>(ctx).error_string();
    }

}

int main() {
    
    typedef typename context::CreateContextType<
        rootModule,
        container::TypeSet<
            isA,
            isB
         >,
        Meta<context::EagerSolve>
    >::type Ctx;

    run<Ctx>();

    return 0;
}




