#ifndef HARMONIZE_CONTEXT_CONTEXT
#define HARMONIZE_CONTEXT_CONTEXT

#include "../container/mod.h"
#include "module.h"
#include "transform.h"

#include <iostream>

namespace context {


    template<typename STATE,typename ENABLE=void>
    struct DepMapBuild {
        static_assert(
            container::IsTypeMap<STATE>::value,
            ASSERT_TEXT("ERROR: STATE parameter must be a TypeMap.")
        );
        static_assert(
            !container::IsTypeMap<STATE>::value,
            ASSERT_TEXT("INTERNAL ERROR: This specialization should never recieve a TypeMap as a parameter.")
        );
    };
    
    template<typename...ARGS>
    struct DepMapBuild <
        container::TypeMap<ARGS...>,
        typename std::enable_if<container::TypeMap<ARGS...>::template ItemAt<tform::RequirementSet>::type::MapType::ITEM_COUNT!=0>::type
    > {

        typedef container::TypeMap<ARGS...> STATE;
        typedef typename STATE::template ItemAt<tform::RootModule>::type     ROOT;
        typedef typename STATE::template ItemAt<tform::RequirementSet>::type REQ_SET;
        typedef typename STATE::template ItemAt<tform::TraitMap>::type       TRAIT_MAP;
        typedef typename STATE::template ItemAt<tform::ImplMap>::type        IMPL_MAP;
        typedef typename STATE::template ItemAt<tform::TransformQueue>::type TFORM_QUEUE;

        static_assert(
            container::IsTypeSet<REQ_SET>::value,
            ASSERT_TEXT("INTERNAL ERROR: REQ_SET parameter must be a TypeSet.")
        );
        static_assert(
            container::IsTypeMap<TRAIT_MAP>::value,
            ASSERT_TEXT("INTERNAL ERROR: TRAIT_MAP parameter must be a TypeMap.")
        );
        static_assert(
            container::IsTypeMap<IMPL_MAP>::value,
            ASSERT_TEXT("INTERNAL ERROR: TRAIT_MAP parameter must be a TypeMap.")
        );
        
        typedef DepMapBuild<STATE> SelfType;

        template <typename TYPE>
        struct NotInOldTraits {
            static constexpr bool value = ! TRAIT_MAP::template has_key<TYPE>();
        };

        // Get the set of required traits that have not yet been added to the trait map
        typedef typename REQ_SET::template Filter<NotInOldTraits>::type UnQueriedTraits;

        // Get a mapping of unqueried traits to the implementation maps returned for those traits
        typedef typename UnQueriedTraits::MapType::template MapItems<ROOT::template ImplFor>::type NewDepMap;

        // Extract a mapping from all queried traits to their implementations
        typedef typename NewDepMap::template MapItems<container::util::type_map::KeySet>::type NewTraitMap;
        // Combine this mapping with the current trait map
        typedef typename TRAIT_MAP::template Combine<NewTraitMap>::type UpdatedTraitMap;

        // Extract a combination of all implementation maps returned from the queries
        typedef typename NewDepMap::template FoldItems<container::TypeMap<>,container::util::type_map::BinaryCombine>::type NewImplMap;
        // Combine these mappings with the current implementation map
        typedef typename IMPL_MAP::template LossyCombine<NewImplMap>::type UpdatedImplMap;

        // Extract the union of all trait requirements listed by the newly-found impl maps
        typedef typename NewImplMap::template FoldItems<container::TypeSet<>,container::util::type_set::BinaryUnion>::type NewImplMapReqs;
        // Filter down set of traits to those that are not already listed in the updated trait map
        typedef typename NewImplMapReqs::template Difference<typename UpdatedTraitMap::KeySet>::type NewReqSet;
   
        typedef TFORM_QUEUE::template PushFront<Meta<DepMapBuild>>::type UpdatedTformQueue;

        typedef STATE::template UpdateItem<tform::RequirementSet,NewReqSet>::type
                     ::template UpdateItem<tform::TraitMap,UpdatedTraitMap>::type
                     ::template UpdateItem<tform::ImplMap,UpdatedImplMap>::type
                     ::template UpdateItem<tform::TransformQueue,UpdatedTformQueue>::type
                     type;

        // Recursively define the fully-resolved mappings of traits to implementations and vice-versa
        //typedef DepMapBuild<ROOT,NewReqSet,UpdatedTraitMap,UpdatedImplMap> NextIteration;
        //typedef typename NextIteration::FinalIteration FinalIteration;
        //typedef typename NextIteration::Result Result;
        //typedef typename Result::TraitMap TraitMap;
        //typedef typename Result::ImplMap  ImplMap;
    };


    template<typename... ARGS>
    struct DepMapBuild <
        container::TypeMap<ARGS...>,
        typename std::enable_if<container::TypeMap<ARGS...>::template ItemAt<tform::RequirementSet>::type::MapType::ITEM_COUNT==0>::type
    > {
        typedef container::TypeMap<ARGS...> STATE;
        typename STATE::template ItemAt<tform::RootModule>::type     ROOT;
        typename STATE::template ItemAt<tform::TraitMap>::type       TRAIT_MAP;
        typename STATE::template ItemAt<tform::ImplMap>::type        IMPL_MAP;

        typedef DepMapBuild<STATE> SelfType;
        typedef STATE type;
        //typedef TRAIT_MAP TraitMap;
        //typedef IMPL_MAP  ImplMap;
        //typedef SelfType FinalIteration;
        //typedef DepMap<TraitMap,ImplMap> Result;

    };


    template <typename IMPL_MAP>
    struct ImplementsTrait {
        template <typename TRAIT>
        struct Selector {
            static constexpr bool value = IMPL_MAP::template has_key<TRAIT>();
        };
    };

    template <typename IMPL_MAP>
    struct TraitHasImplementations {
        template <typename IMPL_SET>
        struct Selector {
            static constexpr bool value = IMPL_SET::template Filter<ImplementsTrait<IMPL_MAP>::template Selector>::type::MapType::ITEM_COUNT > 0;
        };
    };


    template<typename TRAIT_MAP>
    struct TraitMissing {
        template <typename TYPE>
        struct Selector {
            static constexpr bool value = ! TRAIT_MAP::template has_key<TYPE>();
        };

    };

    template <typename TRAIT_MAP>
    struct ImplementationHasTraits {
        template <typename REQ_SET>
        struct Selector {
            static constexpr bool value = REQ_SET::template Filter<TraitMissing<TRAIT_MAP>::template Selector>::type::MapType::ITEM_COUNT == 0;
        };
    };

    template <typename STATE>
    struct Prune {

        typedef STATE::template ItemAt<tform::TraitMap>::type OriginalTraitMap;
        typedef STATE::template ItemAt<tform::ImplMap>::type OriginalImplMap;
        typedef typename OriginalTraitMap::template FilterItems<TraitHasImplementations<OriginalImplMap>::template Selector>::type UpdatedTraitMap;
        typedef typename OriginalImplMap::template FilterItems<ImplementationHasTraits<OriginalTraitMap>::template Selector>::type UpdatedImplMap;

        static constexpr bool NO_TRAIT_CHANGE = std::is_same<OriginalTraitMap,UpdatedTraitMap>::value;
        static constexpr bool NO_IMPL_CHANGE  = std::is_same<OriginalImplMap,UpdatedImplMap>::value;
        static constexpr bool NO_CHANGE = NO_TRAIT_CHANGE && NO_IMPL_CHANGE;


        typedef STATE::template ItemAt<tform::TransformQueue>::type TformQueue;
        typedef container::TypeArray <
                TformQueue::template PushFront<Meta<Prune>>::type,
                TformQueue
            >::ItemAt<NO_CHANGE>::type
            UpdatedTformQueue;

        typedef typename STATE
                ::template UpdateItem<tform::TraitMap,UpdatedTraitMap>::type
                ::template UpdateItem<tform::ImplMap,UpdatedImplMap>::type
                ::template UpdateItem<tform::TransformQueue,UpdatedTformQueue>::type
                type;
    };



    namespace _key {
        struct UnsatTraits{};
        struct UnsatImpls{};
        struct FrontierTraitSet{};
        struct FrontierImplSet{};
    }


    template<typename STATE>
    struct UnsatRecurse {

        typedef STATE::template ItemAt<tform::TraitMap>::type         TraitMap;
        typedef STATE::template ItemAt<tform::ImplMap>::type          ImplMap;
        typedef STATE::template ItemAt<_key::UnsatTraits>::type       UnsatTraits;
        typedef STATE::template ItemAt<_key::UnsatImpls>::type        UnsatImpls;
        typedef STATE::template ItemAt<_key::FrontierTraitSet>::type  FrontierTraitSet;
        typedef STATE::template ItemAt<_key::FrontierImplSet>::type   FrontierImplSet;
        typedef STATE::template ItemAt<tform::TraitSet>::type         TraitSet;
        typedef STATE::template ItemAt<tform::ImplSet>::type          ImplSet;
        typedef STATE::template ItemAt<tform::TransformQueue>::type   TformQueue;


        typedef typename FrontierTraitSet::Map<TraitMap::template ItemAt>::type TraitFrontierImplSets;
        typedef typename Meta<container::TypeSet<>::template Union>::SpecializeFromTypeSet<TraitFrontierImplSets>::type::type UnfilteredImplFrontier;
        typedef typename UnfilteredImplFrontier::template Intersection<UnsatImpls>::type::template Difference<ImplSet>::type UpdatedImplFrontier;
        
        typedef typename FrontierImplSet::Map<ImplMap::template ItemAt>::type ImplFrontierTraitSets;
        typedef typename Meta<container::TypeSet<>::template Union>::template SpecializeFromTypeSet<ImplFrontierTraitSets>::type::type UnfilteredTraitFrontier;
        typedef typename UnfilteredTraitFrontier::template Intersection<UnsatTraits>::type::template Difference<TraitSet>::type UpdatedTraitFrontier; 

        typedef typename TraitSet::template Union<FrontierTraitSet>::type UpdatedTraitSet;
        typedef typename ImplSet::template Union<FrontierImplSet> ::type UpdatedImplSet;

        static constexpr bool EMPTY_TRAIT_FRONTIER = std::is_same<container::TypeSet<>,UpdatedTraitFrontier>::value;
        static constexpr bool EMPTY_IMPL_FRONTIER  = std::is_same<container::TypeSet<>,UpdatedImplFrontier>::value;
        static constexpr bool EMPTY_FRONTIER = EMPTY_TRAIT_FRONTIER && EMPTY_IMPL_FRONTIER;

        typedef typename TypeArray<
                TformQueue::template PushFront<Meta<UnsatRecurse>>::type,
                TformQueue
            >::template ItemAt<EMPTY_FRONTIER>::type UpdatedTformQueue;

        typedef typename STATE
                ::template UpdateItem<_key::FrontierTraitSet,UpdatedTraitFrontier>::type
                ::template UpdateItem<_key::FrontierImplSet, UpdatedImplFrontier>::type
                ::template UpdateItem<_key::TraitSet,        UpdatedTraitSet>::type
                ::template UpdateItem<_key::ImplSet,         UpdatedImplSet>::type
                ::template UpdateItem<_key::TransformQueue,  UpdatedTransformQueue>::type
                type;
    };


    template<typename REQ_SET, typename FULL_MAP, typename PRUNED_MAP>
    struct DepMapCheck {

        typedef typename container::util::Negate<PRUNED_MAP::TraitMap::template HasKey> TraitIsNotSat; 
        typedef typename container::util::Negate<PRUNED_MAP::ImplMap ::template HasKey> CompIsNotSat; 
        
        typedef typename FULL_MAP::TraitMap::template FilterKeys<TraitIsNotSat::template Template>::type UnsatTraitMap;
        typedef typename FULL_MAP::ImplMap::template  FilterKeys<CompIsNotSat ::template Template>::type  UnsatImplMap;

        typedef typename REQ_SET::template Intersection<typename UnsatTraitMap::KeySet>::type TopLevelUnsatReqs;

        typedef UnsatRecurse<
            FULL_MAP,
            typename UnsatTraitMap::KeySet,
            typename UnsatImplMap::KeySet,
            TopLevelUnsatReqs,
            container::TypeSet<>,
            container::TypeSet<>,
            container::TypeSet<>
        > UnsatSearch;
       
       typedef typename UnsatSearch::Result ReqUnsat;
    
        static constexpr bool SOME_TRAITS_UNSATISFIED = UnsatTraitMap::ITEM_COUNT > 0;
        static constexpr bool SOME_IMPLS_UNSATISFIED  = UnsatImplMap::ITEM_COUNT > 0;
        
        static constexpr bool ALL_REQS_SATISFIED   = (ReqUnsat::TraitSet::MapType::ITEM_COUNT + ReqUnsat::ImplSet::MapType::ITEM_COUNT) == 0;
   

        template<typename TYPE_ARRAY>
        static std::string type_list_string() {
            if constexpr (TYPE_ARRAY::MapType::ITEM_COUNT == 0) {
                return "[Nothing]";
            } else {
                typedef typename TYPE_ARRAY::template PopFront<>::type Tail;
                std::string result = container::repr::type_name<typename TYPE_ARRAY::template Front<>::type>();
                if constexpr (Tail::MapType::ITEM_COUNT != 0) {
                    result = result + ", " + type_list_string<Tail>();
                }
                return result; 
            }
        }

        template<typename UNSAT_TRAIT>
        static std::string unsat_trait_diagnostic() {
            std::string trait_name = container::repr::type_name<UNSAT_TRAIT>();
            std::string result = std::string("Trait '")+trait_name+"' is not implemented. Implementation candidates include: ";
            
            typedef typename FULL_MAP::TraitMap::template ItemAt<UNSAT_TRAIT>::type ImplSet;
            typedef typename ImplSet::template Intersection<typename UnsatImplMap::KeySet>::type::MapType::KeyArray UnsatCompArray;
            
            result += type_list_string<UnsatCompArray>();
            return result;
        }

        template<typename UNSAT_COMP>
        static std::string unsat_comp_diagnostic() {
            std::string comp_name = container::repr::type_name<UNSAT_COMP>();
            std::string result = std::string("Component '") + comp_name + "' requires traits that have not been implemented. The traits: ";

            typedef typename FULL_MAP::ImplMap::template ItemAt<UNSAT_COMP>::type TraitSet;
            typedef typename TraitSet::template Intersection<typename UnsatTraitMap::KeySet>::type::MapType::KeyArray UnsatTraitArray;
            result += type_list_string<UnsatTraitArray>();
            return result;
        }


        template<typename TYPE_ARRAY>
        static std::string unsat_trait_list_string() {
            if constexpr (TYPE_ARRAY::MapType::ITEM_COUNT == 0) {
                return "";
            } else {
                typedef typename TYPE_ARRAY::template Front   <>::type Front;
                typedef typename TYPE_ARRAY::template PopFront<>::type Tail;
                std::string result = unsat_trait_diagnostic<Front>()+ "\n";
                if constexpr (Tail::MapType::ITEM_COUNT != 0) {
                    result = result  + unsat_trait_list_string<Tail>();
                }
                return result; 
            }
        }

        template<typename TYPE_ARRAY>
        static std::string unsat_comp_list_string() {
            if constexpr (TYPE_ARRAY::MapType::ITEM_COUNT == 0) {
                return "";
            } else {
                typedef typename TYPE_ARRAY::template Front   <>::type Front;
                typedef typename TYPE_ARRAY::template PopFront<>::type Tail;
                std::string result = unsat_comp_diagnostic<Front>()+ "\n";
                if constexpr (Tail::MapType::ITEM_COUNT != 0) {
                    result = result  + unsat_comp_list_string<Tail>();
                }
                return result; 
            }
        }

        static std::string unsat_diagnostic_string() {
            /*
            std::cout << "Pruned traits:"<<std::endl;
            std::cout<< container::repr::StringRepr<typename PRUNED_MAP::TraitMap>::repr_node().to_string() << std::endl;
            std::cout << "Pruned impls :"<<std::endl;
            std::cout<< container::repr::StringRepr<typename PRUNED_MAP::ImplMap>::repr_node().to_string() << std::endl;
            std::cout << "Original traits:"<<std::endl;
            std::cout<< container::repr::StringRepr<typename FULL_MAP::TraitMap>::repr_node().to_string() << std::endl;
            std::cout << "Original impls :"<<std::endl;
            std::cout<< container::repr::StringRepr<typename FULL_MAP::ImplMap>::repr_node().to_string() << std::endl;
            std::cout << "Unsat traits:"<<std::endl;
            std::cout<< container::repr::StringRepr<UnsatTraitMap>::repr_node().to_string() << std::endl;
            std::cout << "Unsat impls s:"<<std::endl;
            std::cout<< container::repr::StringRepr<UnsatImplMap>::repr_node().to_string() << std::endl;
            std::cout << "ReqUnsat traits:"<<std::endl;
            std::cout<< container::repr::StringRepr<typename ReqUnsat::TraitSet>::repr_node().to_string() << std::endl;
            std::cout << "ReqUnsat impls :"<<std::endl;
            std::cout<< container::repr::StringRepr<typename ReqUnsat::ImplSet>::repr_node().to_string() << std::endl;
            
            std::cout << "ReqUnsat impls :"<<std::endl;
            std::cout<< container::repr::StringRepr<typename SolutionSequence<UnsatSearch>::Result>::repr_node().to_string() << std::endl;
            */
            typedef typename ReqUnsat::TraitSet::MapType::KeyArray TraitArray;
            typedef typename ReqUnsat::ImplSet ::MapType::KeyArray ImplArray;
            return unsat_trait_list_string<TraitArray>() + unsat_comp_list_string<ImplArray>();
        }
        

    
    };



    template<typename... ARGS>
    struct Context;

    template<typename... ARGS>
    struct Super;

    template<typename TRAIT_MAP>
    struct TraitMapAsModuleBundle {
        //typedef typename TraitMap::template FilterKeys<Meta<ParentContext>::Generalizes>::type ParentTraitMap;
        //typedef TraitMap::template FilterKeys<container::util::Negate<Meta<ParentContext>::Generalizes>> LocalTraitMap; 
    };


    struct ContextInfo {};

    template<typename ROOT, typename CHECK, typename SOLVER>
    struct ContextInfoImpl {
        template<typename CONTEXT>
        struct Impl {
            typedef ROOT   Root;
            typedef SOLVER Solver;
            static constexpr bool SATISFIED = CHECK::ALL_REQS_SATISFIED;
            static std::string error_string() {
                return CHECK::unsat_diagnostic_string();
            }
        };
    };


    template <typename...ARGS>
    struct BaseContext;

    template <typename...ARGS>
    struct BaseContext <Context<ARGS...>> {
        typedef Context<ARGS...> Type;
        Type& ref;
    };
    


    template <typename TRAIT_MAP, typename... COMPONENTS>
    struct Context <TRAIT_MAP,COMPONENTS...> : UnMeta<COMPONENTS,Context<TRAIT_MAP,COMPONENTS...>>::Type... {
        
        typedef Context<TRAIT_MAP,COMPONENTS...> Self;  
        typedef TRAIT_MAP TraitMap;


        template <typename TRAIT>
        static constexpr bool implements_trait () {
            return TRAIT_MAP::template has_key<TRAIT>();
        }

        template<typename TRAIT>
        using ComponentLookup = typename UnMeta<typename TRAIT_MAP::template ItemAt<TRAIT>::type,Self>::Type;
      
        typedef  ComponentLookup<ContextInfo> Info;

        template<typename TRAIT>
        ComponentLookup<TRAIT>& as() {
            if constexpr (TRAIT_MAP::template has_key<TRAIT>()) {
                typedef ComponentLookup<TRAIT> COMPONENT;
                if constexpr (Meta<BaseContext>::Generalizes<COMPONENT>::value) {
                    return (*static_cast<ComponentLookup<TRAIT>*>(this)).ref.template as<TRAIT>();
                } else {
                    return *static_cast<ComponentLookup<TRAIT>*>(this);
                }
            } else {
                throw 1;
            }
        }

        template<typename... ARGS>
        Context(ARGS&&... args)
            : ARGS(std::forward<ARGS>(args))...
        {}
        
    };



    template<typename TRAIT, typename IMPL_ARRAY>
    struct BindPriority {
        typedef TRAIT Trait;
        typedef IMPL_ARRAY ImplArray;
    };


    template<typename PRUNED>
    struct EagerSolve {
        typedef typename PRUNED::TraitMap::template MapItems<container::util::type_set::GetFirst>::type TraitMap; 
        typedef typename TraitMap::Invert::type::KeySet ComponentSet;
    };

    template<typename TRAIT_MAP, typename COMPONENT_SET>
    struct ContextFromComponents;

    template<typename TRAIT_MAP, typename... COMPONENTS>
    struct ContextFromComponents <TRAIT_MAP,container::TypeSet<COMPONENTS...>> {
        typedef Context<TRAIT_MAP,COMPONENTS...> type;
    };
    

    template<bool VALID, typename ROOT, typename PRUNED_DEP_MAP, typename CHECK, typename SOLVER>
    struct ContextSolveGuard;
   
    template<typename ROOT, typename PRUNED_DEP_MAP, typename CHECK, typename SOLVER>
    struct ContextSolveGuard <true,ROOT,PRUNED_DEP_MAP,CHECK,SOLVER>
    {
        typedef typename SOLVER::template Template<PRUNED_DEP_MAP>::Type Solution;

        typedef typename container::TypeMap<container::Binding<ContextInfo,Meta<ContextInfoImpl<ROOT,CHECK,SOLVER>::template Impl>>>
                                  ::template LossyCombine<typename Solution::TraitMap>::type TraitMap;
        
        typedef typename container::TypeSet<Meta<ContextInfoImpl<ROOT,CHECK,SOLVER>::template Impl>>
                                  ::template Union<typename Solution::ComponentSet>::type ComponentSet;


        typedef typename context::ContextFromComponents<TraitMap,ComponentSet>::type type;
    };
   
    template<typename ROOT, typename PRUNED_DEP_MAP, typename CHECK, typename SOLVER>
    struct ContextSolveGuard <false,ROOT,PRUNED_DEP_MAP,CHECK,SOLVER>
    {
        typedef container::TypeMap<container::Binding<ContextInfo,Meta<ContextInfoImpl<ROOT,CHECK,SOLVER>::template Impl>>> TraitMap;
        typedef container::TypeSet<Meta<ContextInfoImpl<ROOT,CHECK,SOLVER>::template Impl>> ComponentSet;
        typedef typename context::ContextFromComponents<TraitMap,ComponentSet>::type type;
    };


    template<typename ROOT, typename REQS, typename SOLVER>
    struct CreateContextType {

        typedef container::TypeArray<
            Meta<context::DepMapBuild>,
            Meta<context::Prune>
            Meta<context::DepMapCheck>
        > TformQueue;

        typedef typename container::TypeMap<>
                         ::template SetItem<tform::RootModule,ROOT>::type
                         ::template SetItem<tform::RequirementSet,REQS>::type
                         ::template SetItem<tform::TraitMap,container::TypeMap<>>::type
                         ::template SetItem<tform::ImplMap,container::TypeMap<>>::type
                         ::template SetItem<tform::TransformQueue,TformQueue>::type
                         State;

        typedef typename tform::EvalTransform<State>::type              State;
        typedef typename ContextSolveGuard<Check::ALL_REQS_SATISFIED,ROOT,PrunedDepMap,Check,SOLVER>::type type;

    };

}



template<
    typename TRAIT,
    typename TRAIT_MAP,
    typename...COMPONENTS
>
auto& as(context::Context<TRAIT_MAP,COMPONENTS...>& context) {
    return context.template as<TRAIT>();
}




template<
    typename TRAIT,
    template<typename> typename START_COMP,
    typename TRAIT_MAP,
    typename...COMPONENTS
>
auto& via(START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>>* comp) {
    return as<TRAIT>(*static_cast<context::Context<TRAIT_MAP,COMPONENTS...>*>(comp));
}

template<typename TRAIT,typename CONTEXT>
constexpr bool implements_trait() {
    return CONTEXT::template implements_trait<TRAIT>();
}




template<typename TRAIT,typename CTX>
using As = CTX::template ComponentLookup<TRAIT>;




namespace container {
namespace repr {

    template<typename STATE>
    struct StringRepr <context::DepMapBuild<STATE>> {

        typedef context::DepMapBuild<STATE> Type; 
        
        static StringReprNode repr_node() {
            return StringReprNode {
                "DepMapBuild {",
                StringContentRepr<typename STATE::BindingArray>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }

    };

    
    template <typename... TYPES>
    struct StringRepr<context::RequirementSet<TYPES...>>
    {
        typedef context::RequirementSet<TYPES...> Type;
        static StringReprNode repr_node() {
            return StringReprNode {
                "RequirementSet {",
                StringContentRepr<typename Type::SetType::MapType::KeyArray>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }
    };
    
    
    template<
        typename FULL_MAP,
        typename UNSAT_TRAITS,
        typename UNSAT_IMPLS,
        typename FRONTIER_TRAIT_SET,
        typename FRONTIER_IMPL_SET,
        typename TRAIT_SET,
        typename IMPL_SET
    > struct StringRepr < context::UnsatRecurse <
        FULL_MAP,
        UNSAT_TRAITS,
        UNSAT_IMPLS,
        FRONTIER_TRAIT_SET,
        FRONTIER_IMPL_SET,
        TRAIT_SET,
        IMPL_SET
    > > {
        
        context::UnsatRecurse <
            FULL_MAP,
            UNSAT_TRAITS,
            UNSAT_IMPLS,
            FRONTIER_TRAIT_SET,
            FRONTIER_IMPL_SET,
            TRAIT_SET,
            IMPL_SET
        > Type;
        
        static StringReprNode repr_node() {
            return StringReprNode {
                "UnsatRecurse {",
                StringContentRepr<TypeArray<
                    FRONTIER_TRAIT_SET,
                    FRONTIER_IMPL_SET,
                    TRAIT_SET,
                    IMPL_SET
                >>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }

    };

}
}

#endif // HARMONIZE_CONTEXT_CONTEXT

