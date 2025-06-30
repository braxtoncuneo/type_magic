#include <type_traits>
#include <cassert>
#include <stdexcept>

#include "preamble.h"


#include "sanity_check.cpp"


#include "config.h"


#include "container/mod.h"


template<typename MOD,typename RT>
MOD get(RT& rt) {
    return MOD(rt.set.template get<MOD>(),rt);
}




struct ExampleComponent
{



};

#include "postamble.h"





















// Type Containers:
// - Meta<template<typename>> : Represents a template taking one type parameter
// - TypeSet<typename...>     : Represents a set of types
// - TypeList<typename...>    : Represents a sequence of types. Provides `Set` as a conversion to TypeSet.
// - TypeMap<typename...>     : Represents a mapping of types to other types. Provides TypeList of keys and their corresponding items as `Key` and `Item`

// Type Processors:
// - TryInstantiate<Input,Arg>  : Provides Temp<Arg> as Type if Input is Meta<Temp>. If not, Type is just Input.
// - Filter<Meta<Temp>,TypeSet> : Filters for types in TypeSet where Temp<type>::VALUE == true, provides it as Type
// - Flatten<TypeList<>>        : Expands all TypeList types directly cont
// - FlattenRec<TypeList<>>     :
// - For<TypeList<>,TypeList<>>




// Component Descriptors:
// - Req<type>                 : Requires the `type` trait to be implemented to be used.
// - Impl<type>                : Directly implements the `type` trait.
// -

// - All<template<typename>>   : Corresponds to all specializations of the template.
// - Mixin<template<typename>> : Indicates that the underlying template should have the runtime type plugged in
// -



// Impl<For<Each<Meta<SlabAllocator<RT>>>::Type,TypedAllocator<RT> >::Type>



// Bridge type :
//    Used as a handle to represent a set of functionality and as a guard to
//    check whether or not a type implements that functionality. Concepts may
//    be used by bridges if compiled for C++20 and above.
//
// ComponentType :
//     Literally any type.
// ComponentMixin :
//     A template which produces components based off of an input runtime
//     type. This allows one to write code that can adapt based upon the
//     underlying runtime, and its other components.
// ComponentTemplate :
//     A template which can supply types implementing
// ComponentTemplateMixin :
//     A template which produces a component template based off of an input
//     runtime


// Questions to answer :
//  - Could we compile something that accomplishes this trait? a.k.a. Is it implemented?
//  - Is this trait compiled and available to use in some context? a.k.a. Is it required?
//  - Is this trait actually available to use in this specific context? a.k.a. Is it provided?



// Component member types :
//
// nonlocal :
//     A TypeSet of Member<X,Y> types, with X being a memory space
//     type, and Y being a type that should be held within that memory space.
//     This serves as a way of defining members that are "static" within
//     the lifetime of the owning runtime, while also specifying their
//     memory space. In cases where memory spaces limit mutual accesibility
//     between different execution contexts (e.g. private memory, thread-local
//     memory, and shared memory), exactly one instance is created for each
//     independent region of mutual accessibility.
//
// requires_traits :
//     A TypeSet of traits required by the component. If no such type is
//     provided, it is assumed that the component may always be included in
//     any runtime.
//
// implements_traits :
//     A TypeSet of traits directly implemented by the component. If no such
//     type is provided, it is assumed that the component does not
//     automatically implement any trait simply by being included in a runtime
//     that meets all of its requirements.
//
// reifies_traits :
//     A TypeSet of traits directly reified by the component. If no such type
//     is provided, it is assumed that the component does not reify any trait.
//
// uses_traits :
//     A TypeSet of traits directly used by the component, and hence must be
//     reified directly or by an outter runtime. If no such type is provided,
//     it is assumed that the component does
//
// components :
//     A template that maps trait types to components that implement the
//     given interface. Traits implemented within the "Components" template
//     of a runtime's component also count as implementations for the parent
//     runtime. This allows arbitrary sets of implementations to be provided
//     to runtimes in response to specific conditions.
//


#include "module_check.cpp"

int main() {

    return 0;
}






