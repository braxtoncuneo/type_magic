#ifndef HARMONIZE_CORE_PLATFORM
#define HARMONIZE_CORE_PLATFORM

struct CPU : context::SimpleModule <
    CPU,
    context::RequirementSet<>,
    context::ImplementationSet<CPU>
> {};


struct GPU : context::SimpleModule <
    GPU,
    context::RequirementSet<>,
    context::ImplementationSet<GPU>
> {};

#endif
