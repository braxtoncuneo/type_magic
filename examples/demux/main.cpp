#include "../../include/include.h"

#include <fstream>



// Traits

template<auto FN>
struct Fn {
    static constexpr bool IS_FUNCTION = false;
};

template<typename RETURN, typename CLASS, typename... ARGS>
struct Fn <RETURN (CLASS::*FN)(ARGS...)> {
    static constexpr bool IS_FUNCTION = true;
    static constexpr bool IS_NON_STATIC = true;
    typedef RETURN Return;
    typedef CLASS Class;
    typedef container::TypeArray<ARGS...> Args;
    typedef Return (Class::* NonStaticSignature)(ARGS...);
    typedef Return FunctionSignature (CLASS*,ARGS...);
    static constexpr auto non_static_function = FN;

    static Return function(Class *self, ARGS... args) {
        return (self->*non_static_function)(args...);
    }
};

template<typename RETURN, typename... ARGS>
struct Fn {
    static constexpr bool IS_FUNCTION = true;
    static constexpr bool IS_NON_STATIC = false;
    typedef RETURN Return;
    typedef CLASS Class;
    typedef container::TypeArray<ARGS...> Args;
    typedef Return FunctionSignature (ARGS...);
    static constexpr auto function = FN;
};


struct Schedule<typename FN_TRAIT>{};


// Helper Templates
template<typename TYPE>
struct GetFirstArgument {
    typedef GetTemplateArgs<TYPE>::template ItemAt<0>::type type;
}; 


// Components
template<typename CONTEXT>
struct ScheduleImpl {

    typedef typename CONTEXT::TraitMap::KeySet::template Filter<Meta<Schedule>::template Generalizes>::type ScheduleSet;
    typedef typename ScheduleSet::template Map<GetFirstArgument>::type FunctionSet;

    std::vector<FunctionTaggedUnion> tasks;



    void exec() {
        while(tasks.size() != 0) {
            auto task = tasks.pop_back();
            task();
        }
    }

};


template<typename CONTEXT>
struct Even {
    () {
        return via<TraitY>(this).y;
    }
};

template<typename CONTEXT>
struct Odd {
    int &get_x() {
        return via<TraitX>(this).x;
    }
};


// Modules


template<typename>
struct ScheduleMetaModule;

template<typename FN_TRAIT>
struct ScheduleMetaModule <Schedule<FN_TRAIT>> {
    typedef SimpleModule <
        Meta<ScheduleImpl>,
        DeMux<Meta<ScheduleImpl>,FN_TRAIT>
    >
};


typedef MetaModule<Schedule,ScheduleMetaModule> ScheduleModule;

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

    using namespace container;   
    using namespace context;   

    typedef TypeMap<
        Binding<key::RootModule,RootModule>,
        Binding<key::RequirementSet,TypeSet<TraitX,TraitY>>
    > InputState;

    typedef typename context::CreateContextType<InputState>::type Ctx;

    run<Ctx>();

    return 0;
}




