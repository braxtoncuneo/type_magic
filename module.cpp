#include <type_traits>
#include <cassert>
#include <stdexcept>
#include "sanity_check.cpp"


#ifdef HARMONIZE_RUNTIME_CORRECTNESS_CHECK
constexpr bool RUNTIME_CORRECTNESS_CHECK = HARMONIZE_RUNTIME_CORRECTNESS_CHECK;
#else
constexpr bool RUNTIME_CORRECTNESS_CHECK = false;
#endif


#ifdef HARMONIZE_REORDER_MEMBERS
constexpr bool REORDER_MEMBERS = HARMONIZE_REORDER_MEMBERS;
#else
constexpr bool REORDER_MEMBERS = true;
#endif

///////////////////////////////////////////////////////////////////////////////
// Forward-declare templates
///////////////////////////////////////////////////////////////////////////////


// Like a tuple, but indexes members by a type template parameter
template<typename... BINDINGS>
struct TypeMap;

// Used to help typemaps determine the held type for a given type key
template<typename KEY, typename MAP>
struct TypeMapLookup;

// Used to bind a certain key type to a certain held type for a TypeMap
template<typename KEY, typename TYPE>
struct Binding;




///////////////////////////////////////////////////////////////////////////////
// Define templates
///////////////////////////////////////////////////////////////////////////////


// Holds a template for later application
template<template <typename> typename TEMPLATE>
struct Meta
{
    template <typename ARG>
    struct Template {
        typedef TEMPLATE<ARG> Type;
    };
};

// Specializes the template provided
template<typename TYPE, typename ARG>
struct UnMeta
{
    typedef TYPE Type;
};

template<template <typename> typename TEMPLATE, typename ARG>
struct UnMeta <Meta<TEMPLATE>,ARG>
{
    typedef TEMPLATE<ARG> Type;
};


template<typename KEY, typename TYPE>
struct Binding
{
    typedef KEY  Key;
    typedef TYPE Type;
};


template<typename T>
struct AlwaysFalse
{
    static constexpr bool VALUE = false;
};


// Base case
template<typename... BINDINGS>
struct TypeMap
{
    template<typename KEY>
    static constexpr bool has_key()
    {
        return false;
    }

    static constexpr bool has_duplicate_key()
    {
        return false;
    }

    template <typename OTHER>
    struct Union;

    template <typename... ITEMS>
    struct Union <TypeMap<ITEMS...>> {
        typedef TypeMap<ITEMS...> type;
    };

    template <template<typename> typename SELECTOR>
    struct Filter {
        typedef TypeMap<> type;
    };

    template <typename OTHER>
    struct Intersection;

    template <typename... ITEMS>
    struct Intersection <TypeMap<ITEMS...>> {
        typedef TypeMap<> type;
    };

    template <typename OTHER>
    struct Difference;

    template <typename... ITEMS>
    struct Difference <TypeMap<ITEMS...>> {
        typedef TypeMap<> type;
    };

    struct Invert {
        typedef TypeMap<> type;
    };

    struct Sort {
        typedef TypeMap<> type;
    };

};

// Recursive case
template<typename HEAD, typename... TAIL>
struct TypeMap <HEAD,TAIL...>
{

    typedef TypeMap<HEAD,TAIL...> Self;
    typedef TypeMap<TAIL...> Tail;
    typedef typename HEAD::Key  Key;
    typedef typename HEAD::Type Type;

    template<typename KEY>
    static constexpr bool has_key()
    {
        if constexpr (std::is_same<KEY,Key>::value) {
            return true;
        } else {
            return Tail::template has_key<KEY>();
        }
    }

    static constexpr bool has_duplicate_key()
    {
        if constexpr (Tail::template has_key<Key>()) {
            return true;
        } else {
            return Tail::has_duplicate_key();
        }
    }

    static_assert(
        !TypeMap<HEAD,TAIL...>::has_duplicate_key(),
        "TypeMap cannot contain duplicate key types."
    );


    template <typename OTHER,typename ENABLE=void>
    struct Union;

    template <typename... ITEMS>
    struct Union <
        TypeMap<ITEMS...>,
        typename std::enable_if<TypeMap<ITEMS...>::template has_key<typename HEAD::Key>()>::type
    > {
        typedef typename Tail::template Union<TypeMap<ITEMS...>>::type type;
    };

    template <typename... ITEMS>
    struct Union <
        TypeMap<ITEMS...>,
        typename std::enable_if<!(TypeMap<ITEMS...>::template has_key<typename HEAD::Key>())>::type
    > {
        typedef typename Tail::template Union<TypeMap<HEAD,ITEMS...>>::type type;
    };

    template <template<typename> typename SELECTOR,typename ENABLE=void>
    struct Filter;

    template <template<typename> typename SELECTOR>
    struct Filter <
        SELECTOR,
        typename std::enable_if<SELECTOR<HEAD>::value>::type
    > {
        typedef typename TypeMap<HEAD>::Union<typename Tail::Filter<SELECTOR>::type>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct Filter <
        SELECTOR,
        typename std::enable_if<!(SELECTOR<HEAD>::value)>::type
    > {
        typedef typename Tail::Filter<SELECTOR>::type type;
    };

    template <typename OTHER>
    struct Intersection;

    template <typename... ITEMS>
    struct Intersection <TypeMap<ITEMS...>> {

        typedef TypeMap<ITEMS...> Other;

        template<typename OTHER_ITEM>
        struct OtherHasKey {
            static constexpr bool value = Other::template has_key<typename OTHER_ITEM::Key>();
        };

        typedef typename Self::template Filter<OtherHasKey>::type type;
    };

    template <typename OTHER>
    struct Difference;

    template <typename... ITEMS>
    struct Difference <TypeMap<ITEMS...>> {

        typedef TypeMap<ITEMS...> Other;

        template<typename OTHER_ITEM>
        struct OtherDoesNotHaveKey {
            static constexpr bool value = !Other::template has_key<typename OTHER_ITEM::Key>();
        };

        typedef typename Self::template Filter<OtherDoesNotHaveKey>::type type;
    };

    struct Invert {
        typedef typename TypeMap<Type,Key>::template Union<typename Tail::Invert>::type type;
    };

};


template<typename... ELEMENTS>
struct TypeSet
{
    typedef TypeMap<Binding<ELEMENTS,ELEMENTS>...> MapType;

    template<typename ITEM>
    static constexpr bool has_item()
    {
        return MapType::template has_key<ITEM>();
    }

};




template <size_t INDEX>
struct TypeIndex {
    static constexpr size_t value = INDEX;
};

template<typename... ELEMENTS>
struct TypeArray
{
    typedef TypeMap<Binding<ELEMENTS,ELEMENTS>...> MapType;

    template<size_t INDEX>
    static constexpr bool has_index()
    {
        return MapType::template has_key<TypeIndex<INDEX>> ();
    }

};





struct UndefinedType {
    private:
    UndefinedType(){};
    public:
    static UndefinedType value;

    template <typename TYPE>
    operator TYPE() {
        throw std::runtime_error("Attempted to cast value of undefined type.");
    }
};
UndefinedType UndefinedType::value = UndefinedType();


template<typename TYPE>
struct PartialAssign {
    TYPE &value;
};



// A struct with members defined by a type map

template <typename MAP> struct MapStruct;

template <typename... ITEMS>
struct MapStruct <TypeMap<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type map.");
    }
};


template <typename HEAD, typename... TAIL>
struct MapStruct <TypeMap<HEAD,TAIL...>> {

    typedef TypeMap<HEAD,TAIL...> MapType;
    typedef typename MapType::Type Type;
    typedef MapStruct<TypeMap<TAIL...>> Tail;

    Type data;
    Tail tail;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            if constexpr (std::is_same<KEY,typename MapType::Key>::value) {
                return data;
            } else {
                return tail.template get<KEY>();
            }
        } else {
            static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type map.");
            return UndefinedType::value;
        }
    }
};


// A struct with members defined by a type set

template <typename SET> struct SetStruct;

template <typename... ITEMS>
struct SetStruct <TypeSet<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type set.");
    }
};


template <typename HEAD, typename... TAIL>
struct SetStruct <TypeSet<HEAD,TAIL...>> {

    typedef typename TypeSet<HEAD,TAIL...>::MapType MapType;
    typedef typename MapType::Type Type;
    typedef SetStruct<TypeMap<TAIL...>> Tail;

    Type data;
    Tail tail;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            if constexpr (std::is_same<KEY,typename MapType::Key>::value) {
                return data;
            } else {
                return tail.template get<KEY>();
            }
        } else {
            static_assert(AlwaysFalse<KEY>::VALUE,"Item does not exist in type set.");
            return UndefinedType::value;
        }
    }
};


// A union with members defined by a type set

template <typename SET> struct ArrayStruct;

template <typename... ITEMS>
struct ArrayStruct <TypeArray<ITEMS...>> {
    template<size_t INDEX>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<TypeIndex<INDEX>>::VALUE,"Index does not exist in type array.");
    }
};


template <typename HEAD, typename... TAIL>
struct ArrayStruct <TypeArray<HEAD,TAIL...>> {

    typedef typename TypeArray<HEAD,TAIL...>::MapType MapType;
    typedef typename MapType::Type Type;
    typedef ArrayStruct<TypeArray<TAIL...>> Tail;

    Type data;
    Tail tail;

    template<size_t INDEX>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<TypeIndex<INDEX>>()) {
            if constexpr (std::is_same<TypeIndex<INDEX>,typename MapType::Key>::value) {
                return data;
            } else {
                return tail.template get<INDEX>();
            }
        } else {
            static_assert(AlwaysFalse<TypeIndex<INDEX>>::VALUE,"Index does not exist in type array.");
            return UndefinedType::value;
        }
    }
};




// A union with members defined by a type map

template <typename MAP> union MapUnion;

template <typename... ITEMS>
union MapUnion <TypeMap<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type map.");
    }
};


template <typename HEAD, typename... TAIL>
union MapUnion <TypeMap<HEAD,TAIL...>> {

    typedef TypeMap<HEAD,TAIL...> MapType;
    typedef typename MapType::Type Type;
    typedef MapUnion<TypeMap<TAIL...>> Tail;

    Type data;
    Tail tail;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            if constexpr (std::is_same<KEY,typename MapType::Key>::value) {
                return data;
            } else {
                return tail.template get<KEY>();
            }
        } else {
            static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type map.");
            return UndefinedType::value;
        }
    }
};


// A union with members defined by a type set

template <typename SET> union SetUnion;

template <typename... ITEMS>
union SetUnion <TypeSet<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type set.");
    }
};


template <typename HEAD, typename... TAIL>
union SetUnion <TypeSet<HEAD,TAIL...>> {

    typedef typename TypeSet<HEAD,TAIL...>::MapType MapType;
    typedef typename MapType::Type Type;
    typedef SetUnion<TypeSet<TAIL...>> Tail;

    Type data;
    Tail tail;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            if constexpr (std::is_same<KEY,typename MapType::Key>::value) {
                return data;
            } else {
                return tail.template get<KEY>();
            }
        } else {
            static_assert(AlwaysFalse<KEY>::VALUE,"Item does not exist in type set.");
            return UndefinedType::value;
        }
    }
};



// A union with members defined by a type set

template <typename SET> union ArrayUnion;

template <typename... ITEMS>
union ArrayUnion <TypeArray<ITEMS...>> {
    template<size_t INDEX>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<TypeIndex<INDEX>>::VALUE,"Index does not exist in type array.");
    }
};


template <typename HEAD, typename... TAIL>
union ArrayUnion <TypeArray<HEAD,TAIL...>> {

    typedef typename TypeArray<HEAD,TAIL...>::MapType MapType;
    typedef typename MapType::Type Type;
    typedef ArrayUnion<TypeSet<TAIL...>> Tail;

    Type data;
    Tail tail;

    template<size_t INDEX>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<TypeIndex<INDEX>>()) {
            if constexpr (std::is_same<TypeIndex<INDEX>,typename MapType::Key>::value) {
                return data;
            } else {
                return tail.template get<INDEX>();
            }
        } else {
            static_assert(AlwaysFalse<TypeIndex<INDEX>>::VALUE,"Index does not exist in type array.");
            return UndefinedType::value;
        }
    }
};




template<typename MOD,typename RT>
MOD get(RT& rt) {
    return MOD(rt.set.template get<MOD>(),rt);
}




struct ExampleComponent
{



};






















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






