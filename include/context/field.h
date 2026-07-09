#ifndef HARMONIZE_CONTEXT_FIELD
#define HARMONIZE_CONTEXT_FIELD

#include "context.h"

#include <utility>

namespace context {

    // A `Field` is a labelled value that lives inside a context for the entire
    // lifetime of that context. Where an ordinary component both *stores* state
    // and *implements* behaviour, a field only stores a value. Fields are useful
    // when:
    //
    //   - a value must exist independently of whatever component consumes it,
    //   - the value and its consumer live in different memory spaces, or
    //   - several independent values of the *same* type are needed at once.
    //
    // A field is identified by up to three template arguments, in this order:
    //
    //     Field<LABEL, TYPE, STORAGE>
    //
    //   LABEL   - a tag type naming the field (e.g. `struct SpeedOfLight {};`).
    //             Two fields with different labels are always distinct, even when
    //             they share the same TYPE and STORAGE. This is what lets several
    //             values of identical type coexist in one context.
    //   TYPE    - the type of the value the field holds (e.g. `float`).
    //   STORAGE - (optional) a marker describing where the value is stored, such
    //             as `storage::StorageType<storage::cpu::Global>`. When omitted,
    //             the field defaults to inline, context-local storage.
    //
    // The trait is variadic for forward compatibility: additional storage kinds
    // can introduce trailing arguments without changing the trait's shape. See
    // issue #1 ("Context Fields") for the design rationale.
    template<typename... ARGS>
    struct Field;


    // `FieldComponent` maps the arguments of a `Field<...>` trait to the module
    // that implements it. It follows the same "component wrapper" convention used
    // by `MetaModule` elsewhere in the library: a wrapper exposes an inner
    // `Component<CONTEXT>` type together with a `Module` typedef, and a
    // `MetaModule` binds the trait template to the wrapper template.
    //
    // The primary template is intentionally left with no `Module` member. As a
    // result `IsModuleWrapper` reports `false` for any argument list that does
    // not match one of the specializations below, which lets a `MetaModule` fall
    // back to an `EmptyModule` for unsupported `Field<...>` shapes instead of
    // producing a hard compilation error.
    template<typename... ARGS>
    struct FieldComponent {};


    // Explicit-storage form: `Field<LABEL, TYPE, STORAGE>`.
    template<typename LABEL, typename TYPE, typename STORAGE>
    struct FieldComponent<LABEL, TYPE, STORAGE> {

        // The storage cell. It is nested inside this specialization so that every
        // distinct (LABEL, TYPE, STORAGE) triple yields a distinct component type;
        // if two different fields shared a component type, the module system would
        // (correctly) reject them as a duplicate implementation.
        template<typename CONTEXT>
        struct Component {
            TYPE value;

            Component() = default;
            // Allows both `init<Field<...>>(v)` and `As<Field<...>, Ctx>{v}` to
            // seed the field. `TYPE` is taken by value and moved, so move-only
            // value types are supported.
            explicit Component(TYPE value) : value(std::move(value)) {}
        };

        // A field has no requirements of its own; it simply provides its value.
        using Module = SimpleModule<
            Meta<Component>,
            ImplementationSet<Field<LABEL, TYPE, STORAGE>>,
            RequirementSet<>
        >;

    };


    // Storage-omitted form: `Field<LABEL, TYPE>`. Behaves like the explicit form
    // above but records no storage marker (inline, context-local storage). Its
    // `Component` is a separate nested type, so `Field<L, T>` and
    // `Field<L, T, StorageType<...>>` remain distinct fields.
    template<typename LABEL, typename TYPE>
    struct FieldComponent<LABEL, TYPE> {

        template<typename CONTEXT>
        struct Component {
            TYPE value;

            Component() = default;
            explicit Component(TYPE value) : value(std::move(value)) {}
        };

        using Module = SimpleModule<
            Meta<Component>,
            ImplementationSet<Field<LABEL, TYPE>>,
            RequirementSet<>
        >;

    };


    // The meta-module that manufactures a storage component for every `Field<...>`
    // trait a context requires. Add it to a `ModuleBundle` to enable fields:
    //
    //     using RootModule = context::ModuleBundle<MyModule, context::FieldModule>;
    //
    // Any `Field<...>` listed in a context's requirement set (or required by a
    // component) is then satisfied automatically.
    using FieldModule = MetaModule<Field, FieldComponent>;

}


// Convenience accessor for the common, storage-defaulted field form. Returns a
// reference to the stored value, so it can be read or assigned in place:
//
//     field<SpeedOfLight, float>(ctx) = 3.0e8f;
//     float c = field<SpeedOfLight, float>(ctx);
//
// For the explicit-storage form, use `as<Field<L, T, S>>(ctx).value` directly.
template<typename LABEL, typename TYPE, typename CTX>
TYPE& field(CTX& ctx) {
    return as<context::Field<LABEL, TYPE>>(ctx).value;
}

#endif // HARMONIZE_CONTEXT_FIELD
