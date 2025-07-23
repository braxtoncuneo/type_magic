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
                container::TypeMap<>,container::TypeMap<container::Binding<IMPL,container::TypeSet<ReqSet>>>
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


    template<typename SPEC>
    struct DependencyGraph {

        template<typename REQ_SET, typename TRAIT_MAP=container::TypeMap<>, typename IMPL_MAP=container::TypeMap<>>
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
            typedef typename DepMapBuild<NewReqSet,UpdatedTraitMap,UpdatedImplMap>::Result Result;
            typedef typename Result::TraitMap TraitMap;
            typedef typename Result::ImplMap  ImplMap;

        };


        template <typename TRAIT_MAP,typename IMPL_MAP>
        struct DepMapBuild <container::TypeSet<>,TRAIT_MAP,IMPL_MAP>
        {
            typedef DepMapBuild<container::TypeSet<>,TRAIT_MAP,IMPL_MAP> Result;
            typedef TRAIT_MAP TraitMap;
            typedef IMPL_MAP  ImplMap;
        };

        /*/
        typedef typename DepMapBuild<typename SPEC::RequirementSet>::type DependencyMap;

        template <typename CONNECTED_SET, typename OLD_DEP_SET=TypeSet<>, >
        */

    };


    template<typename SPEC>
    struct EagerSpecSolver {

        template<typename REQ_SET, typename SOLN_MAP = container::TypeMap<>>
        struct Solve {

        };

    };

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
};

#endif // HARMONIZE_CONTEXT_CONTEXT

