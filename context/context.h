#ifndef HARMONIZE_CONTEXT_CONTEXT
#define HARMONIZE_CONTEXT_CONTEXT

#include "../container/mod.h"


namespace context {

    namespace _util {
        template<typename TYPE>
        using GetImplFor = Meta<TYPE::template ImplFor>;
    };

    template<typename TYPE>
    struct IsComponent
    {
        static constexpr bool value = !SpecializeOrFallBack<DoesNotExist,void,_util::GetImplFor,TYPE>::fell_back;
    };

    template<typename SPACE,typename TYPE,typename MARKER>
    struct StaticMember {};

    template<typename COMPONENT, typename TRAIT>
    struct Private {};

    template<typename BUILTIN>
    struct Builtin{};

    template<typename COMPONENT>
    struct Mixin {};

    template <typename... REQUIREMENTS>
    struct RequirementSet
    {
        typedef container::TypeSet<REQUIREMENTS...> SetType;
    };

    template <typename... IMPLEMENTATIONS>
    struct ImplementationSet
    {
        typedef container::TypeSet<IMPLEMENTATIONS...> SetType;
    };

    template<typename IMPL, typename... SUBSPECS>
    struct SimpleComponent
    {

        typedef typename container::template TypeSet<SUBSPECS...>::template CollapseAll<RequirementSet>::type ReqSet;
        typedef typename container::template TypeSet<SUBSPECS...>::template CollapseAll<ImplementationSet>::type::SetType ImplSet;

        template<typename TRAIT>
        struct ImplFor {
            static constexpr bool TRAIT_VALID = ImplSet::template has_item<TRAIT>();
            typedef typename container::TypeArray<
                container::TypeMap<>,container::TypeMap<container::Binding<IMPL,typename ReqSet::SetType>>
            >::template ItemAt<(size_t)TRAIT_VALID>::type type;

        };

    };

    template <typename... COMPONENTS>
    struct ComponentBundle {

        typedef container::TypeSet<COMPONENTS...> SubComponentSet;

        template<typename TRAIT>
        struct ImplFor {

            template<typename A, typename B>
            struct CombineImpl {
                static_assert(
                    container::IsTypeMap<A>::value,
                    ASSERT_TEXT("INTERNAL ERROR: Folding combine operation of ComponentBundle of ImplFor should fold into a TypeMap.")
                );
                static_assert(
                    IsComponent<B>::value,
                    ASSERT_TEXT("ERROR: A constituent of a ComponentBundle does not have the members required of a component.")
                );
                typedef typename B::template ImplFor<TRAIT>::type ImplMap;
                static_assert(
                    container::IsTypeMap<ImplMap>::value,
                    ASSERT_TEXT("ERROR: A constituent of a ComponentBundle did not return a TypeMap from its ImplFor template.")
                );
                typedef typename A::LossyCombine<ImplMap> LossyCombo;
                typedef typename LossyCombo::type type;
                static_assert(
                    !LossyCombo::duplicate_key,
                    ASSERT_TEXT( "ERROR: The same type is listed as an implementation multiple times! Each implementation should be "
                    "generated only once across all components.")
                );
            };

            typedef typename SubComponentSet::template Fold<container::TypeMap<>,CombineImpl>::type type;
        };

    };


    template<typename TRAIT_MAP, typename IMPL_MAP>
    struct DepMap {
        typedef TRAIT_MAP TraitMap;
        typedef IMPL_MAP  ImplMap;
    };


    template<
        typename SPEC,
        typename REQ_SET,
        typename TRAIT_MAP=container::TypeMap<>,
        typename IMPL_MAP=container::TypeMap<>
    >
    struct DepMapBuild {

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
        
        typedef DepMapBuild<SPEC,REQ_SET,TRAIT_MAP,IMPL_MAP> SelfType;

        template <typename TYPE>
        struct NotInOldTraits {
            static constexpr bool value = ! TRAIT_MAP::template has_key<TYPE>();
        };

        // Get the set of required traits that have not yet been added to the trait map
        typedef typename REQ_SET::template Filter<NotInOldTraits>::type UnQueriedTraits;

        // Get a mapping of unqueried traits to the implementation maps returned for those traits
        typedef typename UnQueriedTraits::MapType::template MapItems<SPEC::template ImplFor>::type NewDepMap;

        // Extract a mapping from all queried traits to their implementations
        typedef typename NewDepMap::template MapItems<container::util::type_map::KeySet>::type NewTraitMap;
        // Combine this mapping with the current trait map
        typedef typename TRAIT_MAP::template Combine<NewTraitMap>::type UpdatedTraitMap;

        // Extract a combination of all implementation maps returned from the queries
        typedef typename NewDepMap::template FoldItems<container::TypeMap<>,container::util::type_map::BinaryCombine>::type NewImplMap;
        // Combine these mappings with the current implementation map
        typedef typename IMPL_MAP::template Combine<NewImplMap>::type UpdatedImplMap;

        // Extract the union of all trait requirements listed by the newly-found impl maps
        typedef typename NewImplMap::template FoldItems<container::TypeSet<>,container::util::type_set::BinaryUnion>::type NewImplMapReqs;
        // Filter down set of traits to those that are not already listed in the updated trait map
        typedef typename NewImplMapReqs::template Difference<typename UpdatedTraitMap::KeySet>::type NewReqSet;
    
        // Recursively define the fully-resolved mappings of traits to implementations and vice-versa
        typedef DepMapBuild<SPEC,NewReqSet,UpdatedTraitMap,UpdatedImplMap> NextIteration;
        typedef typename NextIteration::FinalIteration FinalIteration;
        typedef typename NextIteration::Result Result;
        typedef typename Result::TraitMap TraitMap;
        typedef typename Result::ImplMap  ImplMap;
    };


    template <
        typename SPEC,
        typename TRAIT_MAP,
        typename IMPL_MAP
    >
    struct DepMapBuild <SPEC,container::TypeSet<>,TRAIT_MAP,IMPL_MAP>
    {

        typedef DepMapBuild<SPEC,container::TypeSet<>,TRAIT_MAP,IMPL_MAP> SelfType;
        typedef TRAIT_MAP TraitMap;
        typedef IMPL_MAP  ImplMap;
        typedef SelfType FinalIteration;
        typedef DepMap<TraitMap,ImplMap> Result;

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

    template <typename DEP_MAP, typename PREV_DEP_MAP=void>
    struct Prune {
        typedef typename DEP_MAP::TraitMap::template FilterKeys<TraitHasImplementations<typename DEP_MAP::ImplMap>::template Selector>::type UpdatedTraitMap;
        typedef typename DEP_MAP::ImplMap::template FilterItems<ImplementationHasTraits<typename DEP_MAP::TraitMap>::template Selector>::type UpdatedImplMap;

        typedef Prune<DepMap<UpdatedTraitMap,UpdatedImplMap>,DEP_MAP> NextIteration;
        typedef typename NextIteration::FinalIteration FinalIteration;
        typedef typename NextIteration::Result Result;
    };

    template <typename DEP_MAP>
    struct Prune <DEP_MAP,DEP_MAP>{
        typedef Prune<DEP_MAP,DEP_MAP> FinalIteration;
        typedef DEP_MAP Result;
    };


    template<typename TYPE,typename SOLN_SEQ=container::TypeArray<>,typename ENABLE=void >
    struct SolutionSequence {
        typedef typename SOLN_SEQ::template PushBack<TYPE>::type UpdatedSolnSeq;
        typedef SolutionSequence<typename TYPE::NextIteration,UpdatedSolnSeq> NextIteration;
        typedef typename NextIteration::Result Result;
    };

    template<typename TYPE,typename SOLN_SEQ>
    struct SolutionSequence <TYPE,SOLN_SEQ,typename std::enable_if<std::is_same<TYPE,typename TYPE::FinalIteration>::value>::type> {
        typedef typename SOLN_SEQ::template PushBack<TYPE>::type Result;
    };

    
    template<
        typename FULL_MAP,
        typename UNSAT_TRAITS,
        typename UNSAT_IMPLS,
        typename FRONTIER_TRAIT_SET,
        typename FRONTIER_IMPL_SET,
        typename TRAIT_SET,
        typename IMPL_SET
    > struct UnsatRecurse {
        
        typedef typename FRONTIER_TRAIT_SET::Map<FULL_MAP::TraitMap::template ItemAt>::type TraitFrontierImplSets;
        typedef typename Meta<container::TypeSet<>::template Union>::SpecializeFromTypeSet<TraitFrontierImplSets>::type::type UnfilteredImplFrontier;
        typedef typename UnfilteredImplFrontier::template Intersection<UNSAT_IMPLS>::type::template Difference<IMPL_SET>::type UpdatedImplFrontier;
        
        typedef typename FRONTIER_IMPL_SET::Map<FULL_MAP::ImplMap::template ItemAt>::type ImplFrontierTraitSets;
        typedef typename Meta<container::TypeSet<>::template Union>::template SpecializeFromTypeSet<ImplFrontierTraitSets>::type::type UnfilteredTraitFrontier;
        typedef typename UnfilteredTraitFrontier::template Intersection<UNSAT_TRAITS>::type::template Difference<TRAIT_SET>::type UpdatedTraitFrontier;
        

        typedef typename TRAIT_SET::template Union<FRONTIER_TRAIT_SET>::type UpdatedTraitSet;
        typedef typename IMPL_SET ::template Union<FRONTIER_IMPL_SET> ::type UpdatedImplSet;


        typedef UnsatRecurse<
            FULL_MAP,
            UNSAT_TRAITS,
            UNSAT_IMPLS,
            UpdatedTraitFrontier,
            UpdatedImplFrontier,
            UpdatedTraitSet,
            UpdatedImplSet
        > NextIteration;

        typedef typename NextIteration::Result Result;

    };

    template<
        typename FULL_MAP,
        typename UNSAT_TRAITS,
        typename UNSAT_IMPL,
        typename TRAIT_SET,
        typename IMPL_SET
    > struct UnsatRecurse <
        FULL_MAP,
        UNSAT_TRAITS,
        UNSAT_IMPL,
        container::TypeSet<>,
        container::TypeSet<>,
        TRAIT_SET,
        IMPL_SET
    >{
        typedef UnsatRecurse <
            FULL_MAP,
            UNSAT_TRAITS,
            UNSAT_IMPL,
            typename container::TypeSet<>,
            typename container::TypeSet<>,
            TRAIT_SET,
            IMPL_SET
        > FinalIteration;

        struct Result {
            typedef TRAIT_SET TraitSet;
            typedef IMPL_SET  ImplSet;
        }; 
    };


    template<typename REQ_SET, typename FULL_MAP, typename PRUNED_MAP>
    struct DepMapCheck {

        typedef typename container::util::Negate<PRUNED_MAP::TraitMap::KeySet::template HasItem> TraitIsNotSat; 
        typedef typename container::util::Negate<PRUNED_MAP::ImplMap ::KeySet::template HasItem> CompIsNotSat; 
        
        typedef typename FULL_MAP::TraitMap::template FilterKeys<TraitIsNotSat::template Template>::type UnsatTraitMap;
        typedef typename FULL_MAP::ImplMap::template  FilterKeys<CompIsNotSat ::template Template>::type  UnsatImplMap;


        typedef typename UnsatRecurse<
            FULL_MAP,
            typename UnsatTraitMap::KeySet,
            typename UnsatImplMap::KeySet,
            REQ_SET,
            container::TypeSet<>,
            container::TypeSet<>,
            container::TypeSet<>
        >::Result ReqUnsat;
    
        static constexpr bool SOME_TRAITS_UNSATISFIED = UnsatTraitMap::ITEM_COUNT > 0;
        static constexpr bool SOME_IMPLS_UNSATISFIED  = UnsatImplMap::ITEM_COUNT > 0;
        
        static constexpr bool SOME_REQS_UNSATISFIED   = (ReqUnsat::TraitSet::ITEM_COUNT + ReqUnsat::ImplSet::ITEM_COUNT) > 0;
    

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
            result += type_list_string<typename UnsatTraitMap::template ItemAt<UNSAT_TRAIT>::type::MapType::KeyArray>();
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
            typedef typename ReqUnsat::TraitSet::MapType::KeyArray TraitArray;
            typedef typename ReqUnsat::ImplSet ::MapType::KeyArray ImplArray;

            return unsat_trait_list_string<TraitArray>() + unsat_comp_list_string<ImplArray>();
        }
        

    
    };
    


    template<typename UNSAT_REQ_SET>
    struct DepMapUnsatSearch {
        
    };
    
    template<>
    struct DepMapUnsatSearch <container::TypeSet<>> {
        
    };


    /*

    template<typename REQ_SET, typename DEP_MAP>
    struct SolutionGraph {

        struct SolutionNode {
            std::string kind;
            std::string name;
            std::string status;
            std::string reason;
            std::vector<SolutionNode> dependencies;
        };

        std::unordered_map<std::type_index,std::shared_ptr<SolutionNode>> registry;
        std::vector<SolutionNode> requirements;

        template<typename TYPE>
        std::shared_ptr<SolutionNode> trait_node() {
            
        }

        template<typename TYPE>
        std::shared_ptr<SolutionNode> impl_node() {

        }



    };
    */


    template<typename SPEC>
    struct Reify {
        
    };




    /*

    template <typename... COMPONENTS>
    struct Specification {

        template <typename REQ>
        struct Implements {
            static constexpr bool value = (COMPONENTS::template Implements<REQ>::value || ...);
        };

        constexpr bool valid () {

        }
    };


    template <typename COMPONENT>
    struct Context {
    };

    */
}

namespace container {
namespace repr {

    template<
        typename SPEC,
        typename REQ_SET,
        typename TRAIT_MAP,
        typename IMPL_MAP
    > struct StringRepr <
        context::DepMapBuild<
            SPEC,
            REQ_SET,
            TRAIT_MAP,
            IMPL_MAP
        >
    > {

        typedef context::DepMapBuild<
            REQ_SET,
            TRAIT_MAP,
            IMPL_MAP
        > Type; 
        
        static StringReprNode repr_node() {
            return StringReprNode {
                "DepMapBuild {",
                StringContentRepr<TypeArray<REQ_SET,TRAIT_MAP,IMPL_MAP>>::repr(),
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
    
    
    template<typename TRAIT_MAP, typename IMPL_MAP>
    struct StringRepr<context::DepMap<TRAIT_MAP,IMPL_MAP>> {
        typedef context::DepMap<TRAIT_MAP,IMPL_MAP> Type;
        static StringReprNode repr_node() {
            return StringReprNode {
                "DepMap {",
                StringContentRepr<TypeArray<typename Type::TraitMap, typename Type::ImplMap>>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }
    };
    
    
    template<typename DEP_MAP, typename PREV_DEP_MAP>
    struct StringRepr<context::Prune<DEP_MAP,PREV_DEP_MAP>> {
        typedef context::DepMap<DEP_MAP,PREV_DEP_MAP> Type;
        static StringReprNode repr_node() {
            return StringReprNode {
                "Prune {",
                StringContentRepr<TypeArray<DEP_MAP>>::repr(),
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

