#ifndef HARMONIZE_CORE_PLATFORM
#define HARMONIZE_CORE_PLATFORM

namespace platform {

struct CPU : context::TagTrait<CPU> {};
struct GPU : context::TagTrait<GPU> {};

struct CUDA : context::TagTrait<CUDA> {};
struct ROCM : context::TagTrait<ROCM> {};

}

#endif
