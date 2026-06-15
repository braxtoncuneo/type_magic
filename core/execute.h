#ifndef HARMONIZE_CORE_EXECUTE
#define HARMONIZE_CORE_EXECUTE

#include <concepts>
#include "memory.h"




namespace exec {


template<auto FN>
struct Fn {};

template<typename TASK_TYPE>
struct Task {};


template<typename REQ_SET,typename TASK>
struct Launch {};

template<typename TASK>
struct Execute {};

template<typename TASK>
struct Schedule {};

struct Loop{};

struct For{};

struct UnorderedFor{};




namespace impl {

namespace cpu {

    template<typename CONDITION>
    struct Loop{

        template<typename CONTEXT>
        requires std::predicate<CONDITION>
        static void loop(CONDITION c) {
            while (c()){}
        }

    };

    template<typename TASK>
    struct Launch {

        template <typename ROOT_MODULE>
        using SubCtx = typename context::CreateContextType <
            ROOT_MODULE,
            container::TypeSet<TASK>,
            typename CONTEXT::Info::Solver
        >::type;
    
         
        template <typename ROOT_MODULE>
        struct Recurse {
            template<typename CONTEXT> 
            static constexpr bool SATISFIED = SubCtx<CONTEXT>
                ::template ComponentLookup<context::ContextInfo>
                ::ALL_REQS_SATISFIED;
        };

        template<typename CONTEXT>
        struct LaunchImpl {

            template<typename... ARGS>
            void launch(ARGS... args) {

                typename SubCtx<typename CONTEXT::Info::Root>
                       ::template ComponentLookup<storage::StorageType<storage::cpu::Global>>
                       ::type global_fields;

                SubCtx<CONTEXT> ctx(global_fields);
                as<TASK>(ctx)(args...);

            }

        };
    
    };


}


namespace gpu {
  
    /* 
    template<typename LAUNCH_CTX, typename TASK, typename GLOBAL, typename SHARED, typename PRIVATE, typename... ARGS>
    __global__ void launch_helper(GLOBAL * global_fields, ARGS... args) {
            __shared__ SHARED shared_fields;
            PRIVATE private_fields;
            SubCtx ctx(*global_fields,shared_fields,private_fields);
            as<TASK>(ctx)(args);
    }


    template<typename TASK>
    struct KernelLaunch {

        template<typename CONTEXT>
        struct KernelLaunchImpl {
                
            typedef typename context::CreateContextType <
                typename CONTEXT::Info::Root,
                container::TypeSet<TASK>,
                typename CONTEXT::Info::Solver
            >::type SubCtx;

            static constexpr bool SATISFIED = SubCtx::template ComponentLookup<context::ContextInfo>::ALL_REQS_SATISFIED;

            template<typename... ARGS>
            void kernel_launch(dim3 grid_dims, dim3 block_dims, ARGS... args) {
                typedef typename SubCtx::template ComponentLookup<storage::StorageType<storage::gpu::Global>>::type  GlobalFields;
                typedef typename SubCtx::template ComponentLookup<storage::StorageType<storage::gpu::Shared>>::type  SharedFields;
                typedef typename SubCtx::template ComponentLookup<storage::StorageType<storage::gpu::Private>>::type PrivateFields;
                launch_helper<<<grid_dims,block_dims>>>(args);
            }

        };

    };


    template<typename TASK>
    struct Launch {

        template<typename CONTEXT>
        struct KernelLaunchImpl {
                
            typedef typename context::CreateContextType <
                typename CONTEXT::Info::Root,
                container::TypeSet<TASK>,
                typename CONTEXT::Info::Solver
            >::type SubCtx;

            static constexpr bool SATISFIED = SubCtx::template ComponentLookup<context::ContextInfo>::ALL_REQS_SATISFIED;

            template<typename... ARGS>
            void kernel_launch(ARGS... args) {
                typedef typename SubCtx::template ComponentLookup<storage::StorageType<storage::gpu::Shared>>::type  SharedFields;
                typedef typename SubCtx::template ComponentLookup<storage::StorageType<storage::gpu::Private>>::type PrivateFields;
                SubCtx ctx (SharedFields,PrivateFields);
                ctx(args...);
            }

        };

    };
    */

}




}


using CPULoop = context::SimpleModule <
    Meta<impl::cpu::Loop>,
    context::RequirementSet<platform::CPU>,
    context::ImplementationSet<Loop>
>;




}

#endif

