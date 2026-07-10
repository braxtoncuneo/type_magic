#include "../../include/include.h"

#include <fstream>



template<typename TRAIT>
struct TraitImplMutex {

    struct Candidate{};
    struct Checked{};
    struct Unchecked{};


    // Used to find implementations of TRAIT that are not CANDIDATE
    template<typename TRAIT_MAP, typename CANDIDATE>
    struct NotCandidateAndSelected {
        template <typename IMPL>
        struct Filter {
            static constexpr bool implements_trait = TRAIT_MAP::template ItemAt<TRAIT>::type::template has_item<IMPL>();
            static constexpr bool value = implements_trait && ! std::is_same<IMPL,CANDIDATE>::value;
        };
    };

    // Checks each candidate implementation to see if it results in a coontext with all requirements satisfied
    template<typename STATE,typename ENABLE=void>
    struct SolutionSearch;
   
   
    template<typename STATE>
    struct SolutionSearch <
        STATE,
        typename std::enable_if<STATE::template ItemAt<Unchecked>::type::ITEM_COUNT != 0>::type
    > {
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;
        typedef typename STATE::template ItemAt<Candidate>::type CandidateImpl;
        typedef typename STATE::template ItemAt<Checked>::type   CheckedImplSet;
        typedef typename STATE::template ItemAt<Unchecked>::type UncheckedImplSet;
        
        typedef typename ImplMap::KeySet
                ::template Filter<NotCandidateAndSelected<TraitMap,CandidateImpl>::template Filter>::type
                ImplsToRemove;

        typedef typename TraitMap
                ::template MapItems<container::util::type_set::Exclude<ImplsToRemove>::template Template>::type
                UpdatedTraitMap;

        typedef typename ImplMap
                ::template FilterKeys<container::util::Negate<ImplsToRemove::template HasItem>::template Template>::type
                UpdatedImplMap;

        typedef typename STATE
                ::template UpdateItem<context::key::TraitMap,UpdatedTraitMap>::type
                ::template UpdateItem<context::key::ImplMap,UpdatedImplMap>::type
                CandidateState;

        typedef typename context::EvalTransform<CandidateState>::type
                ::template ItemAt<context::key::CheckInfo>::type
                CandidateInfo;
        
        static constexpr bool SATISFIED = CandidateInfo::ALL_REQS_SATISFIED;

        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;
        typedef typename TformQueue::template PushFront<Meta<SolutionSearch>>::type ExtendedTformQueue;
    
        typedef UncheckedImplSet::MapType::HeadKeyType      FallbackCandidate;
        typedef UncheckedImplSet::MapType::TailType::KeySet FallbackUnchecked;
        typedef CheckedImplSet::template Union<container::TypeSet<CandidateImpl>>::type FallbackChecked;

        typedef typename STATE
                ::template SetItem<Candidate,FallbackCandidate>::type
                ::template SetItem<Unchecked,FallbackUnchecked>::type
                ::template SetItem<Checked,FallbackChecked>::type
                ::template SetItem<context::key::TransformQueue,ExtendedTformQueue>::type
                FallbackState;

        typedef typename container::TypeArray<FallbackState,CandidateState>
                ::template ItemAt<SATISFIED>::type type;
    
    };
    
    
    template<typename STATE>
    struct SolutionSearch <
        STATE,
        typename std::enable_if<STATE::template ItemAt<Unchecked>::type::ITEM_COUNT == 0>::type
    > {
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;
        typedef typename STATE::template ItemAt<Candidate>::type CandidateImpl;
        typedef typename STATE::template ItemAt<Checked>::type   CheckedImplSet;
        typedef typename STATE::template ItemAt<Unchecked>::type UncheckedImplSet;
        
        typedef typename ImplMap::KeySet
                ::template Filter<NotCandidateAndSelected<TraitMap,CandidateImpl>::template Filter>::type
                ImplsToRemove;

        typedef typename TraitMap
                ::template MapItems<container::util::type_set::Exclude<ImplsToRemove>::template Template>::type
                UpdatedTraitMap;

        typedef typename ImplMap
                ::template FilterKeys<container::util::Negate<ImplsToRemove::template HasItem>::template Template>::type
                UpdatedImplMap;

        typedef typename STATE
                ::template UpdateItem<context::key::TraitMap,UpdatedTraitMap>::type
                ::template UpdateItem<context::key::ImplMap,UpdatedImplMap>::type
                type;
    };
    

    // Sets up and evaluates a SolutionSearch
    template<typename STATE>
    struct Transform {
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;

        typedef typename TraitMap::template ItemAt<TRAIT>::type MutexSet;

        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;
        typedef typename TformQueue::template PushFront<Meta<SolutionSearch>>::type UpdatedTformQueue;
        
        typedef typename STATE
                ::template SetItem<Candidate,typename MutexSet::MapType::HeadKeyType>::type 
                ::template SetItem<Checked,container::TypeSet<>>::type
                ::template SetItem<Unchecked,typename MutexSet::MapType::TailType::KeySet>::type
                ::template SetItem<context::key::TransformQueue,UpdatedTformQueue>::type
                type;
        
        typedef typename std::enable_if<type::template ItemAt<Unchecked>::type::MapType::ITEM_COUNT != 0>::type X;
    };

    typedef context::SimpleModule<
        TraitImplMutex<TRAIT>,
        context::ImplementationSet<TraitImplMutex<TRAIT>>
    > Module;
};



using MutexModule = context::MetaModule <TraitImplMutex,TraitImplMutex>;




struct TraitX{};
struct TraitY{};
struct CPUOnly{};
struct GPUOnly{};
struct Platform{};
struct GPU{};

struct CircularTrait{};

using CircularModule = context::SimpleModule <
    CircularTrait,
    context::RequirementSet<CircularTrait>,
    context::ImplementationSet<CircularTrait>
>;

using GPUModule = context::SimpleModule <
    GPU,
    context::RequirementSet<TraitImplMutex<Platform>,CircularTrait>,
    context::ImplementationSet<Platform>
>;

struct CPU{};

using CPUModule = context::SimpleModule <
    CPU,
    context::RequirementSet<TraitImplMutex<Platform>>,
    context::ImplementationSet<Platform,CPUOnly>
>;



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
    context::RequirementSet<TraitY,Platform>,
    context::ImplementationSet<TraitX>
>;

using ModuleB = context::SimpleModule <
    Meta<ComponentB>,
    context::RequirementSet<TraitX,Platform,CPUOnly>,
    context::ImplementationSet<TraitY>
>;

using RootModule = context::ModuleBundle<
    CPUModule,
    GPUModule,
    ModuleA,
    ModuleB,
    MutexModule,
    CircularModule
>;



template<typename CTX>
void run() {
 
    if constexpr (CTX::Info::SATISFIED) {
        
        CTX ctx(
            As<TraitX,CTX>{1234},
            As<TraitY,CTX>{56.78}
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

    typedef typename context::CreateContextType<InputState> Creator;
    typedef typename Creator::type Ctx;

    run<Ctx>();

    return 0;
}




