#ifndef HARMONIZE_CORE_ARRAY
#define HARMONIZE_CORE_ARRAY

#include "../context/mod.h"




template<typename TYPE>
struct ArrayAlloc
{
    template<typename COMPONENT>
    struct Check {
        ASSERT_HAS_MEMBER_FOR_TRAIT( ArrayAlloc<TYPE>, COMPONENT, alloc, TYPE*(size_t) )
        ASSERT_HAS_MEMBER_FOR_TRAIT( ArrayAlloc<TYPE>, COMPONENT, free,  void(TYPE*) )
    };
};


template<typename TYPE>
struct DynArray{

};

namespace impl {

template<typename TYPE>
struct ArrayAllocImpl {
    static TYPE *alloc(size_t size) {
        return new TYPE[size];
    }
    static void free(TYPE* ptr) {
        delete ptr;
    }
};


ArrayAlloc<float>::Check<ArrayAllocImpl<float>> __check;

template<typename TYPE>
struct DynArrayImpl {
    template<typename CONTEXT>
    struct DynArray {
        int   size;
        TYPE *ptr;
    };


};

}



template<typename TYPE>
using ArrayAllocComponent = context::SimpleComponent <
    impl::ArrayAllocImpl<TYPE>,
    context::RequirementSet<>,
    context::ImplementationSet<ArrayAlloc<TYPE>>
>;


template<typename TYPE>
using DynArrayComponent = context::SimpleComponent<
    impl::DynArrayImpl<TYPE>,
    context::RequirementSet<ArrayAlloc<TYPE>>,
    context::ImplementationSet<DynArray<TYPE>>
>;


typedef container::TypeSet<DynArray<float>> Requirements;
typedef context::ComponentBundle<DynArrayComponent<float>,ArrayAllocComponent<float>> Bundle;
typedef context::DependencyGraph<Bundle> Graph;
typedef Graph::template DepMapBuild<Requirements>::Result DepMap;
static_assert(AlwaysFalse<DepMap>::values,"NOPE");


#endif // HARMONIZE_CORE_ARRAY
