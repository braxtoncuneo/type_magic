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

        template<typename REQ_SET, typename DEP_MAP=container::TypeMap<>>
        struct DepMapBuild {

            static_assert(
                container::IsTypeSet<REQ_SET>::value,
                ASSERT_TEXT("INTERNAL ERROR: REQ_SET parameter must be a TypeSet.")
            );
            static_assert(
                container::IsTypeMap<DEP_MAP>::value,
                ASSERT_TEXT("INTERNAL ERROR: DEP_MAP parameter must be a TypeMap.")
            );

            template <typename TYPE>
            struct NotInOldDeps {
                static constexpr bool value = ! DEP_MAP::template has_key<TYPE>();
            };

            typedef typename REQ_SET::template Filter<NotInOldDeps>::type UnQueriedReqs;
            typedef typename UnQueriedReqs::MapType::template MapItems<SPEC::template ImplFor>::type NewDepsMap;
            typedef typename DEP_MAP::template Combine<NewDepsMap>::type UpdatedDepMap;

            template <typename TYPE>
            struct NotInNewDeps {
                static constexpr bool value = ! UpdatedDepMap::template has_key<TYPE>();
            };


            template <typename REQ_ACC, typename BINDING>
            struct ReqCombiningFold {
                typedef typename BINDING::ItemType NextImplMap;
                static_assert(
                    container::IsTypeMap<NextImplMap>::value,
                    ASSERT_TEXT("ERROR: Dependency map entry does not provide implementation candidates as a TypeMap.")
                );
                typedef typename NextImplMap::template FoldItems<container::TypeSet<>,container::fold::BinarySetUnionFold>::type LocalUnion;
                typedef typename REQ_ACC::template Union<LocalUnion>::type type;
            };

            typedef typename UpdatedDepMap::Fold<container::TypeSet<>,ReqCombiningFold>::type FullReqSet;
            typedef typename FullReqSet::Filter<NotInNewDeps>::type UpdatedReqSet;

            typedef typename DepMapBuild<UpdatedReqSet,UpdatedDepMap>::type type;

        };

        template <typename DEP_MAP>
        struct DepMapBuild <container::TypeSet<>,DEP_MAP>
        {
            typedef DEP_MAP type;
        };

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

