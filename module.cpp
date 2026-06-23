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


#include <fstream>




// Traits
struct XFloatField{};
struct YIntField{};






struct A {
    float x;
};

using AModule = context::SimpleModule <
    A,
    context::RequirementSet<>,
    context::ImplementationSet<XFloatField>
>;

struct B {
    int y;
};


using BModule = context::SimpleModule <
    B,
    context::RequirementSet<>,
    context::ImplementationSet<YIntField>
>;




typedef context::ModuleBundle<
    AModule,
    BModule
> RootModule;



template<typename CTX>
void run() {
 
    if constexpr (CTX::Info::SATISFIED) {
        
        CTX ctx(
            As<XFloatField,CTX>{45.67},
            As<YIntField,CTX>{9876}
        );

        std::cout << as<XFloatField>(ctx).x << std::endl;
        std::cout << as<YIntField>(ctx).y << std::endl;

    } else {
        CTX ctx;
        std::cout << as<context::ContextInfo>(ctx).error_string();
    }

}

int main() {
    
    typedef typename context::CreateContextType<
        RootModule,
        container::TypeSet<XFloatField,YIntField>,
        Meta<context::EagerSolve>
    >::type Ctx;

    run<Ctx>();

    return 0;
}




