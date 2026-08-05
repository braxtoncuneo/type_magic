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
        #ifdef HARMONIZE_TRACK_SEQUENCE
        typedef typename STATE
                ::template UpdateItem<key::TransformQueue,UpdatedTransformQueue>::type
                PrimedState;
        
        typedef typename STATE::template SetItem<key::Sequence,container::TypeArray<>>::type
                CleanedState;

        typedef typename PrimedState
                ::template ItemAt<key::Sequence>::type
                ::template PushFront<CleanedState>::type
                UpdatedSequence;

        typedef typename PrimedState::template UpdateItem<key::Sequence,UpdatedSequence>::type
                InputState;
        #else
        typedef typename STATE::template UpdateItem<key::TransformQueue,UpdatedTransformQueue>::type InputState;
        #endif
        // The result of the current transform
        typedef typename CurrentTransform::template Template<InputState>::Type::type NextState;
        // The result of the rest of the transformations
        typedef typename EvalTransform<NextState>::type type;  
    };



}

#endif
