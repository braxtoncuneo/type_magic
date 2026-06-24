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


namespace impl {

namespace cpu {

    template<typename CONTEXT>
    struct Loop{

        template<typename CONDITION>
        requires std::predicate<CONDITION>
        static void loop(CONDITION c) {
            while (c()){}
        }

    };

    template<typename TASK>
    struct Launch {

        template<typename CONTEXT>
        struct LaunchImpl {

            void operator()() {
                typedef typename context::CreateContextType <
                    typename CONTEXT::Info::Root,
                    container::TypeSet<TASK>,
                    typename CONTEXT::Info::Solver
                >::type Ctx;

                Ctx ctx;

            }

        };

    };



}







}


using CPULoop = context::SimpleModule <
    Meta<impl::cpu::Loop>,
    context::RequirementSet<platform::CPU>,
    context::ImplementationSet<Loop>
>;




}

#endif

