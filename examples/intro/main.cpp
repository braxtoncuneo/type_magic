#include "../../include/include.h"

#include <fstream>

// Traits

struct TraitX{};
struct TraitY{};



// Components

template<typename CONTEXT>
struct ComponentA {
    int x;
    float &get_y() {
        return via<TraitY>(this).y;
    }
};

template<typename CONTEXT>
struct ComponentB {
    float y;
    int &get_x() {
        return via<TraitX>(this).x;
    }
};


// Modules

using ModuleA = context::SimpleModule <
    Meta<ComponentA>,
    context::RequirementSet<TraitY>,
    context::ImplementationSet<TraitX>
>;

using ModuleB = context::SimpleModule <
    Meta<ComponentB>,
    context::RequirementSet<TraitX>,
    context::ImplementationSet<TraitY>
>;

using RootModule = context::ModuleBundle<
    ModuleA,
    ModuleB
>;



template<typename CTX>
void run() {
 
    if constexpr (CTX::Info::SATISFIED) {
        
        CTX ctx(
            init<TraitX>(1234),
            init<TraitY>(56.78)
        );

        std::cout << "X is: " << as<TraitX>(ctx).x << std::endl;
        std::cout << "Y is: " << as<TraitY>(ctx).y << std::endl;

        as<TraitY>(ctx).get_x() = 4321;
        as<TraitX>(ctx).get_y() = 87.65;

        std::cout << "X is: " << as<TraitX>(ctx).x << std::endl;
        std::cout << "Y is: " << as<TraitY>(ctx).y << std::endl;

    } else {
        CTX ctx;
        std::cout << as<context::ContextInfo>(ctx).error_string();
    }

}

int main() {
    
    typedef typename context::CreateContextType<
        RootModule,
        container::TypeSet<TraitX,TraitY>,
        Meta<context::EagerSolve>
    >::type Ctx;

    run<Ctx>();

    return 0;
}




