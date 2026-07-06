#ifndef HARMONIZE_CONTEXT_TRANSFORM
#define HARMONIZE_CONTEXT_TRANSFORM

#include "module.h"
#include "key.h"


namespace context {

    namespace check {

        template<typename TYPE, typename KEY, typename ENABLE=void>
        struct IsTypeMapWithKey {
            static constexpr bool value = false;
        };

        template<typename... ARGS, typename KEY>
        struct IsTypeMapWithKey <
            container::TypeMap<ARGS...>,
            KEY,
            typename std::enable_if<container::TypeMap<ARGS...>::template has_key<KEY>()>::type
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
            typename std::enable_if<IsTypeMapWithKey<TYPE,key::RootModule>::value>::type
        > {
            static constexpr bool value = context::IsModule<typename TYPE::template ItemAt<key::RootModule>::type>::value;
        };

        template<typename TYPE>
        struct IsMapOverSets {
            static constexpr bool value = false;
        };

        template<typename... BINDINGS>
        struct IsMapOverSets<container::TypeMap<BINDINGS...>> {
            static constexpr bool value = container::TypeMap<BINDINGS...>
                ::template FilterItems<container::util::Negate<Meta<container::TypeSet>::template Generalizes>::template Template>::type
                ::ITEM_COUNT != 0;
        };

        template<typename TYPE, typename ENABLE=void>
        struct HasTraitMap {
            static constexpr bool value = false;
        };
        
        template<typename TYPE>
        struct HasTraitMap <
            TYPE,
            typename std::enable_if<IsTypeMapWithKey<TYPE,key::TraitMap>::value>::type
        > {
            static constexpr bool value = IsMapOverSets<typename TYPE::template ItemAt<key::TraitMap>::type>::value;
        };

        template<typename TYPE, typename ENABLE=void>
        struct HasImplMap {
            static constexpr bool value = false;
        };
        
        template<typename TYPE>
        struct HasImplMap <
            TYPE,
            typename std::enable_if<IsTypeMapWithKey<TYPE,key::ImplMap>::value>::type
        > {
            static constexpr bool value = IsMapOverSets<typename TYPE::template ItemAt<key::ImplMap>::type>::value;
        };

        template<typename TYPE, typename ENABLE=void>
        struct HasTransformQueue {
            static constexpr bool value = false;
        };

        template<typename TYPE>
        struct HasTransformQueue <
            TYPE,
            typename std::enable_if<IsTypeMapWithKey<TYPE,key::TransformQueue>::value>::type
        > {
            static constexpr bool value = TYPE
                ::template FilterItems<
                    container::util::Negate<IsMeta>::template Template
                >::type
                ::ITEM_COUNT != 0;
        };

        template<typename TYPE, typename ENABLE=void>
        struct IsTerminal {
            static constexpr bool value = false;
        };
        
        template<typename TYPE>
        struct IsTerminal <
            TYPE,
            typename std::enable_if<HasTransformQueue<TYPE>::value>::type
        > {
            static constexpr bool value = TYPE::template ItemAt<key::TransformQueue>::type::MapType::ITEM_COUNT == 0;
        };

        template<typename TYPE>
        struct TransformStateInfo {
            static_assert(
                Meta<container::TypeMap>::template Generalizes<TYPE>::value,
                ASSERT_TEXT("Transform states must be TypeMap specializations.")
            );
            static_assert(
                HasRootModule<TYPE>::value,
                ASSERT_TEXT("Transform states must have context::key::RootModule as a key, and the corresponding item must be a module.")
            );
            static_assert(
                HasTraitMap<TYPE>::value,
                ASSERT_TEXT("Transform states must have context::key::TraitMap as a key, and the corresponding item must be a TypeMap with only TypeSet specializations as items.")
            );
            static_assert(
                HasTraitMap<TYPE>::value,
                ASSERT_TEXT("Transform states must have context::key::ImplMap as a key, and the corresponding item must be a TypeMap with only TypeSet specializations as items.")
            );
            static_assert(
                HasTransformQueue<TYPE>::value,
                ASSERT_TEXT("Transform states must have context::key::TransformQueue as a key, and the corresponding item must be a TypeArray with only Meta specializations as items.")
            );
            static constexpr bool IS_TERMINAL = IsTerminal<TYPE>::value;
        };

    }



    template<typename STATE, typename ENABLE=void>
    struct EvalTransform;

    template<typename STATE>
    struct EvalTransform <
        STATE,
        typename std::enable_if<check::IsTerminal<STATE>::value>::type
    > {
        static_assert(
                container::IsTypeMap<STATE>::value,
                ASSERT_TEXT("EvalTransform's parameter must be a TypeMap specialization.")
        );
        typedef check::TransformStateInfo<STATE> Info;
        typedef STATE type; 
        typedef container::TypeArray<STATE> Sequence;
    };

    template<typename STATE>
    struct EvalTransform <
        STATE,
        typename std::enable_if<!check::IsTerminal<STATE>::value>::type
    > {
        static_assert(
                container::IsTypeMap<STATE>::value,
                ASSERT_TEXT("EvalTransform's parameter must be a TypeMap specialization.")
        );
        // Diagnostic information
        typedef check::TransformStateInfo<STATE> Info;
        // The current transform queue, still containing the transform to be evaluated
        typedef typename STATE::template ItemAt<key::TransformQueue>::type TformQueue;
        // The transform that will be evaluated in this iteration
        typedef typename TformQueue::template ItemAt<0>::type CurrentTransform;
        // The updated transform queue, with the current transform removed
        typedef typename TformQueue::template PopFront<>::type UpdatedTransformQueue;
        // The state that will be provided to the current transform
        typedef STATE::template UpdateItem<key::TransformQueue,UpdatedTransformQueue>::type InputState;
        // The result of the current transform
        typedef typename CurrentTransform::template Template<InputState>::Type::type NextState;
        // The result of the rest of the transformations
        typedef typename EvalTransform<NextState>::type type;  
        // The sequence of states that led to the result
        typedef EvalTransform<NextState>::Sequence::template PushFront<STATE>::type Sequence;
    };


    /*
    template <typename... ARGS>
    struct ComponentMutex;
  
    // Mutually excludes the existance of all selected components
    template <template <typename...> SELECTOR>
    struct ComponentMutex <Meta<SELECTOR>>  {

        template <typename DEP_MAP>
        struct Transform {

            typedef  DEP_MAP::ImplMap::template FilterKeys<SELECTOR>::type::KeySet ExclusionSet;

            template <typename TYPE>
            struct ExcludeOthers {

                typedef typename ExclusionSet::template Difference<ItemSet<TYPE>>::type LocalExclusionSet;
                typedef typename DEP_MAP::ImplMap::template FilterKeys<container::util::Negate<LocalExclusionSet::template HasItem>>::type UpdatedImplMap;

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
    */

}

#endif
