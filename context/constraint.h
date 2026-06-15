#ifndef HARMONIZE_CONTEXT_TRANSFORM
#define HARMONIZE_CONTEXT_TRANSFORM

#include "context.h"

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
