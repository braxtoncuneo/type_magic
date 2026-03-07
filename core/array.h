#ifndef HARMONIZE_CORE_ARRAY
#define HARMONIZE_CORE_ARRAY

#include "../context/mod.h"




template<typename TYPE>
struct ArrayAlloc
{

    template<typename COMPONENT>
    struct Check {
        static constexpr bool x = ASSERT_HAS_MEMBER_FOR_TRAIT( ArrayAlloc<TYPE>, COMPONENT, alloc, TYPE*(size_t) );
        static constexpr bool y = ASSERT_HAS_MEMBER_FOR_TRAIT( ArrayAlloc<TYPE>, COMPONENT, free,  void(TYPE*) );
    
        static_assert(y,"YIPPY");
    };
};


template<typename TYPE>
struct DynArray
{
};


template<typename Type>
struct ArrayStack
{
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

template<typename TYPE>
struct ArrayStackImpl {
    template<typename CONTEXT>
    struct ArrayStack {
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


template<typename TYPE>
using ArrayStackComponent = context::SimpleComponent<
    impl::ArrayStackImpl<TYPE>,
    context::RequirementSet<DynArray<TYPE>>,
    context::ImplementationSet<ArrayStack<TYPE>>
>;




#endif // HARMONIZE_CORE_ARRAY
