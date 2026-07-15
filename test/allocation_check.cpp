#include "../include/include.h"

#include <cassert>
#include <concepts>
#include <cstddef>
#include <cstdlib>
#include <type_traits>

namespace allocation_check {

using CpuBytes = alloc::AllocBytes<storage::cpu::Global>;
using GpuBytes = alloc::AllocBytes<storage::gpu::Global>;

using CpuIntObjects = alloc::AllocObject<int, storage::cpu::Global>;
using CpuFloatObjects = alloc::AllocObject<float, storage::cpu::Global>;

using CpuGeneralAlloc = alloc::Alloc<storage::cpu::Global>;
using CpuMinCapacity = alloc::AllocMinCapacity<CpuBytes, 1024>;
using LargerCpuMinCapacity = alloc::AllocMinCapacity<CpuBytes, 4096>;
using CpuUtilization = alloc::AllocUtilization<CpuBytes>;

static_assert(!std::is_same_v<CpuBytes, GpuBytes>);
static_assert(!std::is_same_v<CpuIntObjects, CpuFloatObjects>);
static_assert(!std::is_same_v<CpuMinCapacity, LargerCpuMinCapacity>);
static_assert(std::is_class_v<CpuGeneralAlloc>);
static_assert(std::is_class_v<CpuUtilization>);

template<typename TRAIT, typename COMPONENT>
concept SatisfiesTraitCheck = requires {
    typename TRAIT::template Check<COMPONENT>;
};

struct InstanceByteAllocator {
    void* alloc_bytes(std::size_t size) {
        return std::malloc(size);
    }

    void free_bytes(void* ptr) {
        std::free(ptr);
    }
};

struct StaticByteAllocator {
    static void* alloc_bytes(std::size_t size) {
        return std::malloc(size);
    }

    static void free_bytes(void* ptr) {
        std::free(ptr);
    }
};

struct MissingFreeByteAllocator {
    void* alloc_bytes(std::size_t size) {
        return std::malloc(size);
    }
};

struct WrongReturnByteAllocator {
    int* alloc_bytes(std::size_t) {
        return nullptr;
    }

    void free_bytes(void*) {}
};

static_assert(SatisfiesTraitCheck<CpuBytes, InstanceByteAllocator>);
static_assert(SatisfiesTraitCheck<CpuBytes, StaticByteAllocator>);
static_assert(!SatisfiesTraitCheck<CpuBytes, MissingFreeByteAllocator>);
static_assert(!SatisfiesTraitCheck<CpuBytes, WrongReturnByteAllocator>);

using RootModule = context::ModuleBundle<
    platform::CPU,
    alloc::StdAllocBytes
>;

using CpuContext = typename context::CreateContextType<
    RootModule,
    container::TypeSet<CpuBytes>,
    Meta<context::EagerSolve>
>::type;

using MissingGpuContext = typename context::CreateContextType<
    RootModule,
    container::TypeSet<GpuBytes>,
    Meta<context::EagerSolve>
>::type;

static_assert(CpuContext::Info::SATISFIED);
static_assert(CpuContext::template implements_trait<CpuBytes>());
static_assert(!MissingGpuContext::Info::SATISFIED);

void run() {
    CpuContext ctx;
    auto& allocator = as<CpuBytes>(ctx);

    void* memory = allocator.alloc_bytes(2 * sizeof(int));
    assert(memory != nullptr);

    int* values = static_cast<int*>(memory);
    values[0] = 12;
    values[1] = 34;

    assert(values[0] == 12);
    assert(values[1] == 34);

    allocator.free_bytes(memory);
}

} // namespace allocation_check

int main() {
    allocation_check::run();
    return 0;
}
