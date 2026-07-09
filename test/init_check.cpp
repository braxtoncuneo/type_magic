#include "../include/include.h"

#include <cassert>
#include <memory>
#include <utility>

struct DirectTrait {};
struct OldStyleTrait {};
struct RefTrait {};
struct MoveOnlyArgTrait {};

template<typename CONTEXT>
struct DirectComponent {
    int value;

    explicit DirectComponent(int value) : value(value) {}
    DirectComponent() = delete;
    DirectComponent(DirectComponent const&) = delete;
    DirectComponent(DirectComponent&&) = delete;
};

template<typename CONTEXT>
struct OldStyleComponent {
    int value;

    explicit OldStyleComponent(int value) : value(value) {}
};

template<typename CONTEXT>
struct RefComponent {
    int& value;

    explicit RefComponent(int& value) : value(value) {}
};

template<typename CONTEXT>
struct MoveOnlyArgComponent {
    std::unique_ptr<int> value;

    explicit MoveOnlyArgComponent(std::unique_ptr<int> value)
        : value(std::move(value))
    {}

    MoveOnlyArgComponent() = delete;
    MoveOnlyArgComponent(MoveOnlyArgComponent const&) = delete;
    MoveOnlyArgComponent(MoveOnlyArgComponent&&) = delete;
};

using DirectModule = context::SimpleModule<
    Meta<DirectComponent>,
    context::RequirementSet<>,
    context::ImplementationSet<DirectTrait>
>;

using OldStyleModule = context::SimpleModule<
    Meta<OldStyleComponent>,
    context::RequirementSet<>,
    context::ImplementationSet<OldStyleTrait>
>;

using RefModule = context::SimpleModule<
    Meta<RefComponent>,
    context::RequirementSet<>,
    context::ImplementationSet<RefTrait>
>;

using MoveOnlyArgModule = context::SimpleModule<
    Meta<MoveOnlyArgComponent>,
    context::RequirementSet<>,
    context::ImplementationSet<MoveOnlyArgTrait>
>;

using RootModule = context::ModuleBundle<
    DirectModule,
    OldStyleModule,
    RefModule,
    MoveOnlyArgModule
>;

using Ctx = typename context::CreateContextType<
    RootModule,
    container::TypeSet<
        DirectTrait,
        OldStyleTrait,
        RefTrait,
        MoveOnlyArgTrait
    >,
    Meta<context::EagerSolve>
>::type;

int main() {
    static_assert(Ctx::Info::SATISFIED);

    int referenced = 10;

    Ctx ctx(
        init<DirectTrait>(123),
        As<OldStyleTrait,Ctx>{456},
        init<RefTrait>(referenced),
        init<MoveOnlyArgTrait>(std::make_unique<int>(789))
    );

    assert(as<DirectTrait>(ctx).value == 123);
    assert(as<OldStyleTrait>(ctx).value == 456);
    assert(as<MoveOnlyArgTrait>(ctx).value != nullptr);
    assert(*as<MoveOnlyArgTrait>(ctx).value == 789);

    as<RefTrait>(ctx).value = 11;
    assert(referenced == 11);

    return 0;
}
