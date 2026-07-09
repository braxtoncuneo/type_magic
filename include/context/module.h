#ifndef HARMONIZE_CONTEXT_MODULE
#define HARMONIZE_CONTEXT_MODULE

namespace context {

    namespace _util {
        template<typename TYPE>
        using GetImplFor = Meta<TYPE::template ImplFor>;
    };

    template<typename TYPE>
    struct IsModule
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


    struct EmptyModule {
        template <typename TRAIT>
        struct ImplFor {
            typedef container::TypeMap<> type;
        };
    };

    template<typename IMPL, typename... ARGS>
    struct SimpleModule
    {

        typedef typename container::template TypeSet<ARGS...>::template CollapseAll<RequirementSet>::type ReqSet;
        typedef typename container::template TypeSet<ARGS...>::template CollapseAll<ImplementationSet>::type::SetType ImplSet;

        template<typename TRAIT>
        struct ImplFor {
            static constexpr bool TRAIT_VALID = ImplSet::template has_item<TRAIT>();
            typedef typename container::TypeArray<
                container::TypeMap<>,container::TypeMap<container::Binding<IMPL,typename ReqSet::SetType>>
            >::template ItemAt<(size_t)TRAIT_VALID>::type type;
        };
    };

    template<typename TAG_TRAIT>
    struct TagTrait {
        template<typename TRAIT>
        struct ImplFor {
            static constexpr bool TRAIT_VALID = std::is_same<TAG_TRAIT,TRAIT>::value;
            typedef typename container::TypeArray<
                container::TypeMap<>,container::TypeMap<container::Binding<TAG_TRAIT,container::TypeSet<TAG_TRAIT>>>
            >::template ItemAt<(size_t)TRAIT_VALID>::type type;
        };
    };
   

    template<typename T, typename W=void>
    struct IsModuleWrapper {
        static constexpr bool value = false;
    };

    template<typename T>
    struct IsModuleWrapper <T,typename AlwaysVoid<typename T::Module>::type> {
        static constexpr bool value = true;
    };


    template<
        template <typename...> typename TRAIT_TEMPLATE,
        template <typename...> typename MODULE_TEMPLATE
    >
    struct MetaModule {
        
        template<
            typename TRAIT,
            typename ENABLE=void
        >
        struct ModuleFor {
            typedef EmptyModule type;
        };
        
        template<typename... ARGS>
        struct ModuleFor <
            TRAIT_TEMPLATE<ARGS...>,
            typename std::enable_if<IsModuleWrapper<MODULE_TEMPLATE<ARGS...>>::value>::type
        > {
            typedef typename MODULE_TEMPLATE<ARGS...>::Module type;
        };

        template<typename TRAIT>
        struct ImplFor {
            typedef typename ModuleFor<TRAIT>::type::template ImplFor<TRAIT>::type type;
        };

    };


    template <typename... MODULES>
    struct ModuleBundle {

        typedef container::TypeSet<MODULES...> SubModuleSet;

        template<typename TRAIT>
        struct ImplFor {

            template<typename A, typename B>
            struct CombineImpl {
                static_assert(
                    container::IsTypeMap<A>::value,
                    ASSERT_TEXT("INTERNAL ERROR: Folding combine operation of ModuleBundle of ImplFor should fold into a TypeMap.")
                );
                static_assert(
                    IsModule<B>::value,
                    ASSERT_TEXT("ERROR: A constituent of a ModuleBundle does not have the members required of a module.")
                );
                typedef typename B::template ImplFor<TRAIT>::type ImplMap;
                static_assert(
                    container::IsTypeMap<ImplMap>::value,
                    ASSERT_TEXT("ERROR: A constituent of a ModuleBundle did not return a TypeMap from its ImplFor template.")
                );
                typedef typename A::template LossyCombine<ImplMap> LossyCombo;
                typedef typename LossyCombo::type type;
                static_assert(
                    !LossyCombo::duplicate_key,
                    ASSERT_TEXT( "ERROR: The same type is listed as an implementation multiple times! Each implementation should be "
                    "generated only once across all modules.")
                );
            };

            typedef typename SubModuleSet::template Fold<container::TypeMap<>,CombineImpl>::type type;
        };

    };

}

#endif
