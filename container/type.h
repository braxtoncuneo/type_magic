#ifndef HARMONIZE_CONTAINER_TYPE
#define HARMONIZE_CONTAINER_TYPE

#include "undef.h"

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
    static constexpr bool value = false;
};


// Base case
template<typename... BINDINGS>
struct TypeMap
{

    static constexpr size_t ITEM_COUNT = 0;

    template<typename KEY>
    static constexpr bool has_key()
    {
        return false;
    }

    static constexpr bool has_duplicate_key()
    {
        return false;
    }

    template <typename KEY>
    struct ItemAt {
        static_assert(
            AlwaysFalse<KEY>::value,
            ASSERT_TEXT("Key does not exist in type map.")
        );
        typedef UndefinedType type;
    };

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

    static constexpr size_t ITEM_COUNT = Tail::ITEM_COUNT + 1;

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
        (!TypeMap<HEAD,TAIL...>::has_duplicate_key()),
        "TypeMap cannot contain duplicate key types."
    );


    template <typename KEY_QUERY,typename ENABLE=void>
    struct ItemAt;

    template <typename KEY_QUERY>
    struct ItemAt <
        KEY_QUERY,
        typename std::enable_if<!(std::is_same<Key,KEY_QUERY>::value)>::type
    > {
        typedef typename Tail::template ItemAt<KEY_QUERY>::type type;
    };

    template <typename KEY_QUERY>
    struct ItemAt <
        KEY_QUERY,
        typename std::enable_if<std::is_same<Key,KEY_QUERY>::value>::type
    > {
        typedef Type type;
    };


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

    template<size_t INDEX, typename... ELEMS>
    struct TypeMapGenerator;

    template<size_t INDEX>
    struct TypeMapGenerator <INDEX> {
        typedef TypeMap<> type;
    };

    template<size_t INDEX, typename HEAD, typename... TAIL>
    struct TypeMapGenerator <INDEX,HEAD,TAIL...> {
        typedef typename TypeMapGenerator<INDEX+1,TAIL...>::type TailMap;
        typedef typename TypeMap<Binding<TypeIndex<INDEX>,HEAD>>::template Union<TailMap>::type type;
    };

    typedef typename TypeMapGenerator<0,ELEMENTS...>::type MapType;

    template<size_t INDEX>
    static constexpr bool has_index()
    {
        return MapType::template has_key<TypeIndex<INDEX>> ();
    }

};


#endif // HARMONIZE_CONTAINER_TYPE

