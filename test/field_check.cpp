#include "../include/include.h"

#include <cassert>

// ---------------------------------------------------------------------------
// Field labels
// ---------------------------------------------------------------------------
// Labels are empty tag types whose only job is to give a field a unique name.
// Note that `CountA` and `CountB` will be paired with the *same* value type
// (`int`) yet remain two independent fields.
struct SpeedOfLight {};
struct Gravity {};
struct CountA {};
struct CountB {};


// ---------------------------------------------------------------------------
// A component that consumes a field
// ---------------------------------------------------------------------------
// `EnergyTrait` is implemented by `Kinetic`, which does *not* store the speed
// of light itself: it requires the `Field<SpeedOfLight, float>` and reads the
// value out of the surrounding context. This demonstrates a value that exists
// independently of the component consuming it.
struct EnergyTrait {};

template<typename CONTEXT>
struct Kinetic {
    float mass;

    explicit Kinetic(float mass) : mass(mass) {}

    // E = m * c^2, with c pulled from the context field.
    float energy() {
        float c = via<context::Field<SpeedOfLight, float>>(this).value;
        return mass * c * c;
    }
};

using KineticModule = context::SimpleModule<
    Meta<Kinetic>,
    context::RequirementSet<context::Field<SpeedOfLight, float>>,
    context::ImplementationSet<EnergyTrait>
>;


// ---------------------------------------------------------------------------
// Assemble the root module
// ---------------------------------------------------------------------------
// `context::FieldModule` is the meta-module that manufactures a storage
// component for every `Field<...>` the context ends up requiring.
using RootModule = context::ModuleBundle<
    KineticModule,
    context::FieldModule
>;


// ---------------------------------------------------------------------------
// Build the context type
// ---------------------------------------------------------------------------
// The required traits are the consumer trait plus every field we want to exist
// in the context. Two fields share the value type `int` (CountA / CountB), and
// one field uses the explicit storage form.
using Ctx = typename context::CreateContextType<
    RootModule,
    container::TypeSet<
        EnergyTrait,
        context::Field<SpeedOfLight, float>,
        context::Field<CountA, int>,
        context::Field<CountB, int>,
        context::Field<Gravity, double, storage::StorageType<storage::cpu::Global>>
    >,
    Meta<context::EagerSolve>
>::type;

// The explicit-storage field is long to spell out; alias it for readability.
using GravityField =
    context::Field<Gravity, double, storage::StorageType<storage::cpu::Global>>;


int main() {
    static_assert(Ctx::Info::SATISFIED, "Context requirements could not be satisfied.");

    Ctx ctx(
        // A component that consumes a field:
        init<EnergyTrait>(2.0f),
        // A field seeded with the `init<>` helper (tuple-forwarded constructor):
        init<context::Field<SpeedOfLight, float>>(3.0e8f),
        // Fields seeded with the aggregate-style `As<>` syntax:
        As<context::Field<CountA, int>, Ctx>{7},
        As<context::Field<CountB, int>, Ctx>{42},
        // A field carrying an explicit storage marker:
        init<GravityField>(9.81)
    );

    // Fields hold the values they were initialised with. (The extra parentheses
    // shield the template commas from the `assert` macro's argument parsing.)
    assert((as<context::Field<SpeedOfLight, float>>(ctx).value == 3.0e8f));
    assert((field<CountA, int>(ctx) == 7));   // convenience accessor
    assert((field<CountB, int>(ctx) == 42));

    // Two fields of identical type are genuinely independent slots.
    field<CountA, int>(ctx) += 1;
    assert((field<CountA, int>(ctx) == 8));
    assert((field<CountB, int>(ctx) == 42));

    // A consumer component reads a field out of the context.
    // E = m * c^2 = 2 * (3e8)^2.
    assert((as<EnergyTrait>(ctx).energy() == 2.0f * 3.0e8f * 3.0e8f));

    // The explicit-storage field works and is mutable.
    assert((as<GravityField>(ctx).value == 9.81));
    as<GravityField>(ctx).value = 1.62;
    assert((as<GravityField>(ctx).value == 1.62));

    return 0;
}
