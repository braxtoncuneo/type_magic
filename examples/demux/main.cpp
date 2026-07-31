#include "../../include/include.h"

#include <cstdint>



// Traits
struct State{};

struct AsyncStartup {};

template<typename Executor>
struct AsyncExecutor {};

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
        typedef container::FnObj<container::Fn<&MIXIN<Context>::operator()>> type;
    };

    typedef typename TraitMap::KeySet::template Filter<IsAsync>::type AsyncSet;
    typedef typename AsyncSet::template Map<ComponentTypeFromMixin>::type::ItemArray FnObjTypeArray;
    typedef container::ArrayVariant<FnObjTypeArray> VariantType;
    typedef typename VariantType::MapType::Invert::type FnMap;


    std::vector<VariantType> tasks;
    
    template<typename FN,typename... ARGS>
    inline constexpr void call(ARGS... args) {
        using namespace container;
        constexpr size_t INDEX = FnMap::template ItemAt<FnObj<FN>>::type::value;
        if constexpr (FN::IS_STATIC) {
            VariantType fn_variant(init::init<TypeIndex<INDEX>>(FnObj<FN>(args...)));
            tasks.push_back(fn_variant);
        } else {
            typedef typename FN::Class Class;
            auto self = static_cast<Class*>(static_cast<Context*>(this));
            VariantType fn_variant(init::init<TypeIndex<INDEX>>(FnObj<FN>(self,args...)));
            tasks.push_back(fn_variant);
        }
    }


    inline constexpr void execute() {
        while(tasks.size() != 0) {
            auto executor = [](auto fn){fn();};
            VariantType next_task = tasks.back();
            tasks.pop_back();
            next_task.visit(executor);
        }
    }

};


template<template<typename>typename MIXIN>
struct AsyncTrampoline {

    template<typename CONTEXT>
    struct Impl {
        
        template<typename... ARGS>
        inline constexpr void operator()(ARGS... args) {
            typedef container::Fn<&MIXIN<CONTEXT>::operator()> FnTrait;
            as<AsyncCall<MIXIN>>(this).template call<FnTrait>(args...);
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
struct Even;
template<typename CONTEXT>
struct Odd;


template<typename CONTEXT>
struct Even {

    inline void operator()(CollazIter iter) {
        using namespace container;
        if (iter.value <= 1) {
            as<State>(this).steps[iter.start_value] = iter.step_count;
        } else {
            iter.value /= 2;
            iter.step_count ++;
            if ( (iter.value%2) == 0 ) {
                as<Async<Even>>(this)(iter);
            } else {
                as<Async<Odd>>(this)(iter);
            }
        }
    }

};

template<typename CONTEXT>
struct Odd {

    inline void operator()(CollazIter iter) {
        using namespace container;
        if (iter.value <= 1) {
            as<State>(this).steps[iter.start_value] = iter.step_count;
        } else {
            iter.value *= 3;
            iter.value += 1;
            iter.step_count ++;
            as<Async<Even>>(this)(iter);
        }
    }

};
 


template<typename CONTEXT>
struct StartUp {

    inline void operator()(uint64_t limit) {
        using namespace container;
        as<State>(this).steps.resize(limit);
        for (uint64_t i=0; i<limit; i++) {
            as<State>(this).steps[i] = 0;
            if ( (i%2) == 0 ) {
                as<Async<Even>>(this)(CollazIter{i,0,i});
            } else {
                as<Async<Odd>>(this)(CollazIter{i,0,i});
            }
        }
    }

};



template<typename CONTEXT>
struct StartAsync;

template<typename TRAIT_MAP, typename... COMPONENTS>
struct StartAsync<context::Context<TRAIT_MAP,COMPONENTS...>> {

    private:
    template <typename EXEC_SET>
    void startup() {
        if constexpr ( EXEC_SET::ITEM_COUNT == 0 ) {
            return;
        } else {
            as<typename EXEC_SET::MapType::HeadItemType>(this).execute();
            startup<typename EXEC_SET::MapType::TailType::KeySet>();
        }
    }
    public:
    void operator()() {
        typedef typename TRAIT_MAP::KeySet::template Filter<
            Meta<AsyncExecutor>::template Generalizes
        >::type ExecSet;
        startup<ExecSet>();
    }
};


// Modules

struct CPUAsyncModule {

    template <typename TRAIT, typename ENABLE=void>
    struct ImplFor {
        typedef container::TypeMap<> type;
    };

    template <typename TRAIT>
    struct ImplFor <
        TRAIT,
        typename std::enable_if<std::is_same<TRAIT,AsyncExecutor<Meta<SimpleCPUAsyncImpl>>>::value>::type
    > {
        typedef container::TypeMap<container::Binding<
            DeMux<Meta<SimpleCPUAsyncImpl>,AsyncExecutor<Meta<SimpleCPUAsyncImpl>>>,
            container::TypeSet<
                AsyncExecutor<Meta<SimpleCPUAsyncImpl>>
            >
        >> type;
    };

    template <template<typename>typename FN_MIXIN>
    struct ImplFor <AsyncCall<FN_MIXIN>,void> {
        typedef container::TypeMap<container::Binding<
            DeMux<Meta<SimpleCPUAsyncImpl>,AsyncCall<FN_MIXIN>>,
            container::TypeSet<
                container::Method<FN_MIXIN>,
                AsyncExecutor<Meta<SimpleCPUAsyncImpl>>
            >
        >> type;
    };

};

struct AsyncTrampolineModule {

    template <typename TRAIT>
    struct ImplFor {
        typedef container::TypeMap<> type;
    };

    template <template<typename>typename FN_MIXIN>
    struct ImplFor <Async<FN_MIXIN>> {
        typedef container::TypeMap<container::Binding<
            Meta<AsyncTrampoline<FN_MIXIN>::template Impl>,
            container::TypeSet<AsyncCall<FN_MIXIN>>
        >> type;
    };

};

using StartAsyncModule = context::SimpleModule <
    Meta<StartAsync>,
    context::ImplementationSet<AsyncStartup>
>;

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
int run() {
 
    CTX ctx;
    if constexpr (CTX::Info::SATISFIED) {
        as<container::Method<StartUp>>(ctx)(10000);
        as<AsyncStartup>(ctx)();
        auto counts = as<State>(ctx).steps;
        size_t result = 0;
        for(size_t i=0; i<counts.size(); i++) {
            result += counts[i];
        }
        return result;
    } else {
        std::cout << as<context::ContextInfo>(ctx).error_string();
        std::cout << as<context::ContextInfo>(ctx).solve_sequence_string();
        return 0;
    }

}


struct RootModule : context::ModuleBundle<
    StartAsyncModule,
    CPUAsyncModule,
    AsyncTrampolineModule,
    EvenModule,
    OddModule,
    StartUpModule,
    StateModule
> {};


int add(int x) {
    return x;
}

int main() {

    using namespace container;   
    using namespace context;   

    typedef TypeMap<
        Binding<context::key::RootModule,RootModule>,
        Binding<context::key::RequirementSet,TypeSet<Method<StartUp>,AsyncStartup>>
    > InputState;

    typedef typename context::CreateContextType<InputState>::type Ctx;

    return run<Ctx>();
}




