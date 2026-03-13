#ifndef HARMONIZE_CORE_EXECUTE
#define HARMONIZE_CORE_EXECUTE

#include <concepts>
#include "memory.h"




namespace exec {

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


    template<typename FN_TRAIT>
    struct SubContextExecute  {

        

    };


}







}


using CPULoop = context::SimpleModule <
    Meta<impl::cpu::Loop>,
    context::RequirementSet<CPU>,
    context::ImplementationSet<Loop>
>;





}

#endif

