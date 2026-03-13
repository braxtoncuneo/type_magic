#ifndef HARMONIZE_CORE_MEMORY
#define HARMONIZE_CORE_MEMORY

#include "platform.h"

namespace storage {


template<typename STORAGE_TYPE>
struct Storage{}; 

namespace cpu {
    struct Global {};
}

namespace gpu {
    struct Global {};
    struct Shared {};
}

struct Default{};

template<typename SCOPE, typename TYPE, typename STORAGE_TYPE=Default>
struct StaticMember {
    TYPE value;  
};


}

namespace scope {
    struct Global{};
}


namespace alloc {

    template<typename T>
    concept HasAllocBytes = requires() {
        { &T::alloc_bytes } -> std::same_as<void*(T::*)(size_t)>;
    };

    template<typename T>
    concept HasFreeBytes = requires() {
        { &T::free_bytes } -> std::same_as<void(T::*)(void*)>;
    };

    struct AllocBytes {
        template<typename COMPONENT>
        requires HasAllocBytes<COMPONENT> && HasFreeBytes<COMPONENT>
        struct Check {};
    };


    struct StdAllocBytesImpl {
        void *alloc_bytes(size_t size) {
            return malloc(size);
        }
        void  free_bytes(void *ptr) {
            free(ptr);
        }
    };

    using StdAllocBytes = context::SimpleModule<
        StdAllocBytesImpl,
        context::RequirementSet<CPU>,
        context::ImplementationSet<AllocBytes>
    >;

}

#endif
