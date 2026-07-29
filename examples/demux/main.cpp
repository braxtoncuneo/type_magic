#include "../../include/include.h"

#include <cstdint>



// Traits

struct State{};

template<template<typename>typename FN_MIXIN>
struct AsyncCall {};

template<template<typename>typename FN_MIXIN>
struct Async {};



// Helper Templates
template<typename TYPE>
struct GetFirstArgument {
    typedef typename GetTemplateArgs<TYPE>::template ItemAt<0>::type type;
}; 

template<typename TYPE>
struct IsAsync {
    static constexpr bool value = false;
};

template<template<typename>typename FN_MIXIN>
struct IsAsync <Async<FN_MIXIN>> {
    static constexpr bool value = true;
};



// Components
template<typename CONTEXT>
struct SimpleCPUAsyncImpl;

template<typename TRAIT_MAP, typename... COMPONENTS>
struct SimpleCPUAsyncImpl<context::Context<TRAIT_MAP,COMPONENTS...>> {

    typedef context::Context<TRAIT_MAP,COMPONENTS...> Context;
    typedef TRAIT_MAP TraitMap;

    template<typename TYPE>
    struct ComponentTypeFromMixin;

    template<template<typename>typename MIXIN>
    struct ComponentTypeFromMixin <Async<MIXIN>> {
        typedef container::Fn<&MIXIN<Context>::operator()> type;
    };

    typedef typename TraitMap::KeySet::template Filter<IsAsync>::type AsyncSet;
    typedef typename AsyncSet::template Map<ComponentTypeFromMixin>::type FunctionSet;
    typedef container::ArrayVariant<typename FunctionSet::ItemArray> VariantType;
    typedef typename VariantType::MapType::Invert::type FnMap;


    std::vector<VariantType> tasks;
    
    template<typename FN,typename... ARGS>
    void call(ARGS... args) {
        //VariantType fn_variant;
        //fn_varient.get<FnMap::template ItemAt<FN>::type::value>()
        //tasks.push_back(typename {}.bind(FN::function(args...)));
        FN::function(args...);
    }


    void exec() {
        while(tasks.size() != 0) {
            auto task = tasks.pop_back();
            task();
        }
    }

};

template<template<typename>typename MIXIN>
struct AsyncTrampoline {

    template<typename CONTEXT>
    struct Impl {
        
        template<typename... ARGS>
        void operator()(ARGS... args) {
            typedef container::Fn<&MIXIN<CONTEXT>::operator()> FnTrait;
            via<AsyncCall<MIXIN>>(this).template call<FnTrait>(&via<container::Method<MIXIN>>(this),args...);
        }

    };

};


struct StepCounts {
    std::vector<uint64_t> steps;
};


struct CollazIter {
    uint64_t value;
    uint64_t step_count;
    uint64_t start_value;
};

template<typename CONTEXT>
struct Odd;


template<typename CONTEXT>
struct Even {

    void operator()(CollazIter iter) {
        using namespace container;
        if (iter.value <= 1) {
            via<State>(this).steps[iter.start_value] = iter.step_count;
            std::cout << iter.start_value << " : " << iter.step_count << std::endl;
        } else {
            iter.value /= 2;
            iter.step_count ++;
            if ( (iter.value%2) == 0 ) {
                via<Async<Even>>(this)(iter);
            } else {
                via<Async<Odd>>(this)(iter);
            }
        }
    }

};

template<typename CONTEXT>
struct Odd {

    void operator()(CollazIter iter) {
        using namespace container;
        if (iter.value <= 1) {
            via<State>(this).steps[iter.start_value] = iter.step_count;
            std::cout << iter.start_value << " : " << iter.step_count << std::endl;
        } else {
            iter.value *= 3;
            iter.value += 1;
            iter.step_count ++;
            via<Async<Even>>(this)(iter);
        }
    }

};
 


template<typename CONTEXT>
struct StartUp {

    void operator()(uint64_t limit) {
        using namespace container;
        via<State>(this).steps.resize(limit);
        for (uint64_t i=0; i<limit; i++) {
            via<State>(this).steps[i] = 0;
            if ( (i%2) == 0 ) {
                via<Async<Even>>(this)(CollazIter{i,0,i});
            } else {
                via<Async<Odd>>(this)(CollazIter{i,0,i});
            }
        }
    }

};



// Modules

struct CPUAsyncModule {

    template <typename TRAIT>
    struct ImplFor {
        typedef container::TypeMap<> type;
    };

    template<template<typename>typename FN_MIXIN>
    struct ImplFor <AsyncCall<FN_MIXIN>> {
        typedef container::TypeMap<container::Binding<
            DeMux<Meta<SimpleCPUAsyncImpl>,AsyncCall<FN_MIXIN>>,
            container::TypeSet<container::Method<FN_MIXIN>>
        >> type;
    };

};

struct AsyncTrampolineModule {

    template <typename TRAIT>
    struct ImplFor {
        typedef container::TypeMap<> type;
    };

    template<template<typename>typename FN_MIXIN>
    struct ImplFor <Async<FN_MIXIN>> {
        typedef container::TypeMap<container::Binding<
            Meta<AsyncTrampoline<FN_MIXIN>::template Impl>,
            container::TypeSet<AsyncCall<FN_MIXIN>>
        >> type;
        static_assert(std::is_same<Async<FN_MIXIN>,void>::value);
    };

};


using EvenModule = context::SimpleModule <
    Meta<Even>,
    context::RequirementSet<
        State,
        Async<Even>,
        Async<Odd>
    >,
    context::ImplementationSet<container::Method<Even>>
>;

using OddModule = context::SimpleModule <
    Meta<Odd>,
    context::RequirementSet<
        State,
        Async<Even>
    >,
    context::ImplementationSet<container::Method<Odd>>
>;

using StartUpModule = context::SimpleModule <
    Meta<StartUp>,
    context::RequirementSet<
        State,
        Async<Even>,
        Async<Odd>
    >,
    context::ImplementationSet<container::Method<StartUp>>
>;

using StateModule = context::SimpleModule <
    StepCounts,
    context::ImplementationSet<State>
>;


template<typename CTX>
void run() {
 
    CTX ctx;
    if constexpr (CTX::Info::SATISFIED) {
        as<container::Method<StartUp>>(ctx)(10);
        std::cout << container::repr::type_name<CTX>();
    } else {
        std::cout << as<context::ContextInfo>(ctx).error_string();
        std::cout << as<context::ContextInfo>(ctx).solve_sequence_string();
    }

}


struct RootModule : context::ModuleBundle<
    CPUAsyncModule,
    AsyncTrampolineModule,
    EvenModule,
    OddModule,
    StartUpModule,
    StateModule
> {};


int main() {

    using namespace container;   
    using namespace context;   

    typedef TypeMap<
        Binding<context::key::RootModule,RootModule>,
        Binding<context::key::RequirementSet,TypeSet<Method<StartUp>>>
    > InputState;

    typedef typename context::CreateContextType<InputState>::type Ctx;

    run<Ctx>();

    return 0;
}




