#ifndef HARMONIZE_CORE_ARRAY
#define HARMONIZE_CORE_ARRAY

#include "../context/mod.h"
#include "platform.h"






template<typename TYPE>
struct ArrayAlloc
{
    template<typename COMPONENT>
    struct Check {

        //static constexpr bool has_alloc = HAS_MEMBER_FOR_TRAIT( ArrayAlloc<TYPE>, COMPONENT, alloc, TYPE*(size_t) );
        //static constexpr bool has_free  = HAS_MEMBER_FOR_TRAIT( ArrayAlloc<TYPE>, COMPONENT, free,  void(TYPE*) );
    
    };
};


template<typename TYPE>
struct DynArray
{
};


template<typename TYPE>
struct ArrayStack
{
};


namespace impl {

template<typename TYPE>
struct CPUArrayAlloc {

    template<typename CONTEXT>
    struct Impl {
        static TYPE *alloc(size_t size) {
            return new TYPE[size];
        }
        static void free(TYPE* ptr) {
            delete ptr;
        }
    };

};

template<typename TYPE>
struct GPUArrayAlloc {

    template<typename CONTEXT>
    struct Impl {
        static TYPE *alloc(size_t size) {
            return new TYPE[size];
        }
        static void free(TYPE* ptr) {
            delete ptr;
        }
    };

};



template<typename TYPE>
struct CPUDynArray {

    struct Impl {
        int   size;
        TYPE *ptr;
    };

};


template<typename TYPE>
struct CPUArrayStack {

    struct Impl {
        int   size;
        TYPE *ptr;
    };

};

}




template<typename TYPE>
using GPUArrayAlloc = context::SimpleModule <
    Meta<impl::GPUArrayAlloc<TYPE>::template Impl>,
    context::RequirementSet<platform::GPU>,
    context::ImplementationSet<impl::GPUArrayAlloc<TYPE>>
>;



template<typename TYPE>
using CPUArrayAlloc = context::SimpleModule <
    Meta<impl::CPUArrayAlloc<TYPE>::template Impl>,
    context::RequirementSet<platform::CPU>,
    context::ImplementationSet<ArrayAlloc<TYPE>>
>;


template<typename TYPE>
using CPUDynArray = context::SimpleModule<
    typename impl::CPUDynArray<TYPE>::Impl,
    context::RequirementSet<ArrayAlloc<TYPE>>,
    context::ImplementationSet<DynArray<TYPE>>
>;


template<typename TYPE>
using CPUArrayStack = context::SimpleModule<
    typename impl::CPUArrayStack<TYPE>::Impl,
    context::RequirementSet<DynArray<TYPE>>,
    context::ImplementationSet<ArrayStack<TYPE>>
>;




#endif // HARMONIZE_CORE_ARRAY
