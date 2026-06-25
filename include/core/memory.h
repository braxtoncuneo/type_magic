#ifndef HARMONIZE_CORE_MEMORY
#define HARMONIZE_CORE_MEMORY

#include "platform.h"

namespace storage {

    namespace cpu {
        struct Global : context::TagTrait<Global> {};
    }

    namespace gpu {
        struct Global : context::TagTrait<Global> {};
        struct Shared : context::TagTrait<Shared> {};
        struct Private : context::TagTrait<Private> {};
    }

    namespace hetero {
        struct Managed : context::TagTrait<Managed> {};
        struct Unified : context::TagTrait<Unified> {};
    }


    struct Default : context::TagTrait<Default> {};
    
    template<typename... ARGS>
    struct Path{};

    template<typename... ARGS>
    struct ValueType{};

    template<typename...>
    struct StorageType{};

    template<typename... ARGS>
    struct StaticMember;


    template<typename... PATH_ARGS, typename STORAGE_TYPE, typename VALUE_TYPE>
    struct StaticMember <Path<PATH_ARGS...>,STORAGE_TYPE,VALUE_TYPE> {
        typedef VALUE_TYPE   ValueType;
        typedef STORAGE_TYPE StorageType;
    };


    template<typename STORAGE_TYPE>
    struct HasStorageType {

        template<typename... ARGS>
        struct Selector;

        template<typename... PATH_ARGS, typename VALUE_TYPE>
        struct Selector <Path<PATH_ARGS...>,STORAGE_TYPE,VALUE_TYPE> {
            typedef VALUE_TYPE   ValueType;
            typedef STORAGE_TYPE StorageType;
        };
    };


    namespace impl {
        
        namespace cpu {
            template<typename PATH, typename VALUE_TYPE>
            struct GlobalStaticMemberImpl {
                VALUE_TYPE value;
            };
        }

        namespace gpu {
            template<typename PATH, typename VALUE_TYPE>
            struct GlobalStaticMemberImpl {
                VALUE_TYPE &value;
            };
            template<typename PATH, typename VALUE_TYPE>
            struct SharedStaticMemberImpl {
                VALUE_TYPE &value;
            };
            template<typename PATH, typename VALUE_TYPE>
            struct PrivateStaticMemberImpl {
                VALUE_TYPE value;
            };
        }

    }

    struct StorageModule {
        template<typename TRAIT> struct ImplFor;

        template<typename... PATH_ARGS, typename VALUE_TYPE>
        struct ImplFor<StaticMember<Path<PATH_ARGS...>,cpu::Global,VALUE_TYPE>> {
            typedef container::TypeMap<container::Binding<
                impl::cpu::GlobalStaticMemberImpl<Path<PATH_ARGS...>,VALUE_TYPE>,
                container::TypeSet<platform::CPU>
            >> type;
        };

        template<typename... PATH_ARGS, typename VALUE_TYPE>
        struct ImplFor<StaticMember<Path<PATH_ARGS...>,gpu::Global,VALUE_TYPE>> {
            typedef container::TypeMap<container::Binding<
                impl::gpu::GlobalStaticMemberImpl<Path<PATH_ARGS...>,VALUE_TYPE>,
                container::TypeSet<platform::GPU>
            >> type;
        };

        template<typename... PATH_ARGS, typename VALUE_TYPE>
        struct ImplFor<StaticMember<Path<PATH_ARGS...>,gpu::Shared,VALUE_TYPE>> {
            typedef container::TypeMap<container::Binding<
                impl::gpu::SharedStaticMemberImpl<Path<PATH_ARGS...>,VALUE_TYPE>,
                container::TypeSet<platform::GPU>
            >> type;
        };

        template<typename... PATH_ARGS, typename VALUE_TYPE>
        struct ImplFor<StaticMember<Path<PATH_ARGS...>,gpu::Private,VALUE_TYPE>> {
            typedef container::TypeMap<container::Binding<
                impl::gpu::PrivateStaticMemberImpl<Path<PATH_ARGS...>,VALUE_TYPE>,
                container::TypeSet<platform::GPU>
            >> type;
        };
    };

}

namespace scope {
    struct Global : context::TagTrait<Global> {};
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
        context::RequirementSet<platform::CPU>,
        context::ImplementationSet<AllocBytes>
    >;

}




#endif
