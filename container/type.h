#ifndef HARMONIZE_CONTAINER_TYPE
#define HARMONIZE_CONTAINER_TYPE

#include "undef.h"

namespace container {

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


template<typename KEY, typename ITEM>
struct Binding
{
    typedef KEY  KeyType;
    typedef ITEM ItemType;
};

template<typename TYPE>
struct IsBinding
{
    static constexpr bool value = false;
};

template<typename KEY, typename ITEM>
struct IsBinding<Binding<KEY,ITEM>>
{
    static constexpr bool value = true;
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
    struct LossyCombine;

    template <typename... ITEMS>
    struct LossyCombine <TypeMap<ITEMS...>> {
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

    template<typename PHONEY=void, typename ENABLE=void>
    struct SortHelper {
        typedef Binding<UndefinedType,char> BiggestItem;
        typedef TypeMap<> Remainder;
        typedef TypeMap<> RemainderSorted;
        typedef TypeMap<> type;
    };

    struct Sort {
        typedef TypeMap<> type;
    };

    struct DefaultStructOrder {
        typedef TypeMap<> type;
    };

    static void structure_print_recurse() {}

    static void print_items() {
        printf("{\n");
        structure_print_recurse();
        printf("}\n");
    }

};

// Recursive case
template<typename HEAD, typename... TAIL>
struct TypeMap <HEAD,TAIL...>
{

    static_assert(
        IsBinding<HEAD>::value,
        "Every argument to the TypeMap template must be a Binding specialization."
    );

    typedef TypeMap<HEAD,TAIL...> SelfType;
    typedef TypeMap<TAIL...> TailType;
    typedef typename HEAD::KeyType  HeadKeyType;
    typedef typename HEAD::ItemType HeadItemType;

    static constexpr size_t ITEM_COUNT = TailType::ITEM_COUNT + 1;

    template<typename KEY>
    static constexpr bool has_key()
    {
        if constexpr (std::is_same<KEY,HeadKeyType>::value) {
            return true;
        } else {
            return TailType::template has_key<KEY>();
        }
    }

    static constexpr bool has_duplicate_key()
    {
        if constexpr (TailType::template has_key<HeadKeyType>()) {
            return true;
        } else {
            return TailType::has_duplicate_key();
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
        typename std::enable_if<!(std::is_same<HeadKeyType,KEY_QUERY>::value)>::type
    > {
        typedef typename TailType::template ItemAt<KEY_QUERY>::type type;
    };

    template <typename KEY_QUERY>
    struct ItemAt <
        KEY_QUERY,
        typename std::enable_if<std::is_same<HeadKeyType,KEY_QUERY>::value>::type
    > {
        typedef HeadItemType type;
    };


    template <typename OTHER,typename ENABLE=void>
    struct LossyCombine;

    template <typename... ITEMS>
    struct LossyCombine <
        TypeMap<ITEMS...>,
        typename std::enable_if<TypeMap<ITEMS...>::template has_key<HeadKeyType>()>::type
    > {
        typedef typename TailType::template LossyCombine<TypeMap<ITEMS...>>::type type;
    };

    template <typename... ITEMS>
    struct LossyCombine <
        TypeMap<ITEMS...>,
        typename std::enable_if<!(TypeMap<ITEMS...>::template has_key<HeadKeyType>())>::type
    > {
        typedef typename TailType::template LossyCombine<TypeMap<HEAD,ITEMS...>>::type type;
    };

    template <template<typename> typename SELECTOR,typename ENABLE=void>
    struct Filter;

    template <template<typename> typename SELECTOR>
    struct Filter <
        SELECTOR,
        typename std::enable_if<SELECTOR<HEAD>::value>::type
    > {
        typedef typename TypeMap<HEAD>::LossyCombine<typename TailType::Filter<SELECTOR>::type>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct Filter <
        SELECTOR,
        typename std::enable_if<!(SELECTOR<HEAD>::value)>::type
    > {
        typedef typename TailType::Filter<SELECTOR>::type type;
    };

    template <typename OTHER>
    struct Intersection;

    template <typename... ITEMS>
    struct Intersection <TypeMap<ITEMS...>> {

        typedef TypeMap<ITEMS...> OtherType;

        template<typename OTHER_ITEM>
        struct OtherHasKey {
            static constexpr bool value = OtherType::template has_key<typename OTHER_ITEM::KeyType>();
        };

        typedef typename SelfType::template Filter<OtherHasKey>::type type;
    };

    template <typename OTHER>
    struct Difference;

    template <typename... ITEMS>
    struct Difference <TypeMap<ITEMS...>> {

        typedef TypeMap<ITEMS...> OtherType;

        template<typename OTHER_ITEM>
        struct OtherDoesNotHaveKey {
            static constexpr bool value = !OtherType::template has_key<typename OTHER_ITEM::KeyType>();
        };

        typedef typename SelfType::template Filter<OtherDoesNotHaveKey>::type type;
    };

    struct Invert {
        typedef typename TypeMap<HeadItemType,HeadKeyType>::template LossyCombine<typename TailType::Invert>::type type;
    };

    template <typename PHONEY=void, typename ENABLE=void>
    struct SortHelper;

    template <typename PHONEY>
    struct SortHelper<
        PHONEY,
        typename std::enable_if<sizeof(HeadItemType)>=sizeof(typename TailType::template SortHelper<>::BiggestItem::ItemType),PHONEY>::type
    > {
        typedef HEAD     BiggestItem;
        typedef TailType Remainder;
        typedef typename Remainder::Sort::type RemainderSorted;
        typedef typename TypeMap<BiggestItem>::template LossyCombine<RemainderSorted>::type type;
    };

    template <typename PHONEY>
    struct SortHelper<
        PHONEY,
        typename std::enable_if<sizeof(HeadItemType)<sizeof(typename TailType::template SortHelper<>::BiggestItem::ItemType),PHONEY>::type
    > {
        typedef typename TailType::template SortHelper<>::BiggestItem BiggestItem;
        typedef typename TailType::template SortHelper<>::Remainder   TailRemainder;
        typedef typename TypeMap<HEAD>::template LossyCombine<TailRemainder>::type Remainder;
        typedef typename Remainder::Sort::type RemainderSorted;
        typedef typename TypeMap<BiggestItem>::template LossyCombine<RemainderSorted>::type type;
    };

    struct Sort {
        typedef typename SortHelper<>::type type;
    };


    template <typename PHONEY=void, typename ENABLE=void>
    struct DefaultStructOrderHelper;

    template <typename PHONEY>
    struct DefaultStructOrderHelper<
        PHONEY,
        typename std::enable_if<config::REORDER_STRUCT_MEMBERS,PHONEY>::type
    > {
        typedef typename Sort::type type;
    };

    template <typename PHONEY>
    struct DefaultStructOrderHelper<
        PHONEY,
        typename std::enable_if<(!config::REORDER_STRUCT_MEMBERS),PHONEY>::type
    > {
        typedef TypeMap<HEAD,TAIL...> type;
    };

    struct DefaultStructOrder {
        typedef typename DefaultStructOrderHelper<>::type type;
    };

    static void structure_print_recurse() {
        printf("\t%s : %s;\n",typeid(HeadKeyType).name(),typeid(HeadItemType).name());
        TailType::structure_print_recurse();
    }

    static void print_items() {
        printf("{\n");
        structure_print_recurse();
        printf("}\n");
    }


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
        typedef typename TypeMap<Binding<TypeIndex<INDEX>,HEAD>>::template LossyCombine<TailMap>::type type;
    };

    typedef typename TypeMapGenerator<0,ELEMENTS...>::type MapType;

    template<size_t INDEX>
    static constexpr bool has_index()
    {
        return MapType::template has_key<TypeIndex<INDEX>> ();
    }

};


}


#endif // HARMONIZE_CONTAINER_TYPE

