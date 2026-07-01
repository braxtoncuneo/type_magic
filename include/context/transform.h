#ifndef HARMONIZE_CONTEXT_TRANSFORM
#define HARMONIZE_CONTEXT_TRANSFORM

#include "context.h"


namespace _util {

    template<typename TYPE, typename KEY, typename ENABLE=void>
    struct IsTypeMapWithKey {
        static constexpr bool value = false;
    };

    template<typename... ARGS, typename KEY, typename ENABLE=void>
    struct IsTypeMapWithKey <
        TypeMap<ARGS...>,
        KEY,
        typename std::enable_if<TypeMap<ARGS...>::has_key<KEY>()>::type
    > {
        static constexpr bool value = true;
    };


    template<typename TYPE, typename ENABLE=void>
    struct HasRootModule {
        static constexpr bool value = false;
    };
    
    template<typename TYPE>
    struct HasRootModule <
        TYPE,
        typename std::enable_if<IsTypeMapWithKey<TYPE,RootModule>>
    > {
        static constexpr bool value = IsModule<typename TYPE::template ItemAt<RootModule>::type>::value;
    };

    template<typename TYPE>
    struct IsMapOverSets {
        static constexpr bool value = false;
    };

    template<typename... BINDINGS>
    struct IsMapOverSets<TypeMap<BINDINGS...>> {
        static constexpr bool value = TypeMap<BINDINGS...>
            ::template FilterItems<Negate<Meta<container::TypeSet>::template Generalizes>::template Template>::type
            ::ITEM_COUNT != 0;
    };

    template<typename TYPE, typename ENABLE=void>
    struct HasTraitMap {
        static constexpr bool value = false;
    };
    
    template<typename TYPE>
    struct HasTraitMap <
        TYPE,
        typename AlwaysVoid<typename TYPE::TraitMap>::type
    > {
        static constexpr bool value = IsMapOverSets<typename TYPE::TraitMap>::value;
    };

    template<typename TYPE, typename ENABLE=void>
    struct HasImplMap {
        static constexpr bool value = false;
    };
    
    template<typename TYPE>
    struct HasImplMap <TYPE,typename AlwaysVoid<typename TYPE::ImplMap>::type> {
        static constexpr bool value = IsMapOverSets<typename TYPE::TraitMap>::value;
    };

    template<typename TYPE, typename ENABLE=void>
    struct HasDepMap {
        static constexpr bool value = false;
    };
    
    template<typename TYPE>
    struct HasDepMap <TYPE,typename AlwaysVoid<typename TYPE::DepMap>::type> {
        static constexpr bool value = HasTraitMap<TYPE>::value && HasImplMap<TYPE>::value;
    };

    template<typename TYPE, typename ENABLE=void>
    struct IsTransform {
        static constexpr bool value = false;
    };

    template<typename TYPE>
    struct IsTransform <TYPE,typename std::enable_if<IsTemplate<Type::Transform>>::type> {
        static constexpr bool value = true;
    };

    template<typename TYPE, typename ENABLE=void>
    struct HasTransformQueue {
        static constexpr bool value = false;
    };

    template<typename TYPE>
    struct HasTransformQueue <
        TYPE,
        std::enable_if<
            Meta<container::TypeArray>::template Generalizes<TYPE>::value,
            typename AlwaysVoid<typename TYPE::TransformQueue>::type
        >
    > {
        static constexpr bool value = TYPE::MapType
            ::template FilterItems<
                Negate<Meta<container::TypeSet>::template Generalizes>::template Template
            >::type
            ::ITEM_COUNT != 0;
    };

    template<typename TYPE, typename ENABLE=void>
    struct IsTerminal {
        static constexpr bool value = false;
    };
    
    template<typename TYPE>
    struct IsTerminal <TYPE,typename std::enable_if<HasTransformQueue<TYPE>::value>::type> {
        static constexpr bool value = TYPE::TransformQueue::MapType::ITEM_COUNT == 0;
    };

    template<typename TYPE>
    struct TransformIterInfo {
        static_assert(
            HasRootModule<TYPE>::value,
            ASSERT_TEXT("Transform iterations must have a module named RootModule.")
        );
        static_assert(
            HasDepMap<TYPE>::value,
            ASSERT_TEXT("Transform iterations must have a type named DepMap which contains a TraitMap type and an ImplMap type.")
        );
        static_assert(
            HasTransformQueue<TYPE>::value,
            ASSERT_TEXT("Transform iterations must have a TypeArray named TransformQueue which contains only valid Transform types (defining a template<typename...> typename called Transform).")
        );
        static constexpr bool IS_TERMINAL         = IsTerminal<TYPE>::value;
    };

}

template<typename ITERATION, typename ENABLE=void>
struct EvaluateTransform;


template<typename ITERATION>
struct TransformChain <
    ITERATION,
    ITERATION::TransformArray
> {
    typedef ITERATION::TransformArray::template ItemAt<0>::type 
};



namespace transform {

    template <typename... ARGS>
    struct ComponentMutex;
  
    // Mutually excludes the existance of all selected components
    template <typename template <typename...> SELECTOR>
    struct ComponentMutex <Meta<SELECTOR>>  {

        template <typename DEP_MAP>
        struct Transform {

            typedef  DEP_MAP::ImplMap::template FilterKeys<SELECTOR>::type::KeySet ExclusionSet;

            template <typename TYPE>
            struct ExcludeOthers {

                typedef typename ExclusionSet::template Difference<ItemSet<TYPE>>::type LocalExclusionSet;
                typedef typename DEP_MAP::ImplMap::template FilterKeys<Negate<LocalExclusionSet::template HasItem>>::type UpdatedImplMap;

                struct DepMap {
                    typedef typename DEP_MAP::TraitMap TraitMap;
                    typedef UpdatedImplMap ImplMap;
                };
                typedef typename Prune<DepMap>::Result type;
            };

            typedef typename ExclusionSet::template Map<ExcludeOthers>::type Result;
            
        };

    };


    template <typename... ARGS>
    struct ImplMutex;
  
    // Mutually excludes the implementation of all selected traits by more than one component
    template <typename template <typename...> SELECTOR>
    struct ImplMutex <Meta<SELECTOR>>  {

        template <typename DEP_MAP>
        struct Transform {

            typedef  DEP_MAP::TraitMap::template FilterKeys<SELECTOR>::type::KeySet TraitSet;
            typedef  DEP_MAP::ImplMap ::template FilterItems<
                item_set::SharesItemWith<TraitSet>::template Template
            >::type ExclusionSet;

            template <typename TYPE>
            struct ExcludeOtherImpls {

                typedef typename ExclusionSet::template Difference<ItemSet<TYPE>>::type LocalExclusionSet;
                typedef typename DEP_MAP::ImplMap::template FilterKeys<
                    LocalExclusionSet::template HasItem
                >::type PreExclusionSubMap;

                typedef typename PreExclusionMap::template MapItems<
                    type_set::Exclude<TraitSet>::Template
                >::type ExcludedSubMap;
                typedef typename ExcludedSubMap::template LossyCombine<typename DEP_MAP::ImplMap>  ImplMap;

                struct DepMap {
                    typedef typename DEP_MAP::TraitMap TraitMap;
                    typedef ImplMap ImplMap;
                };
                typedef typename Prune<DepMap>::Result type;
            };

            typedef ExclusionSet::template Map<ExcludeOthers>::type Result;
            
        };

    };

}

#endif
