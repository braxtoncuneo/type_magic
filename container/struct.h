#ifndef HARMONIZE_CONTAINER_STRUCT
#define HARMONIZE_CONTAINER_STRUCT

#include "type.h"
#include "undef.h"
#include "assign.h"




///////////////////////////////////////////////////////////////////////////////
// MapStruct
///////////////////////////////////////////////////////////////////////////////

template <typename MAP> struct MapStruct;

template <typename... ITEMS>
struct MapStruct <TypeMap<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(
            AlwaysFalse<KEY>::VALUE,
            "Key does not exist in type map."
        );
    }
};


template <typename HEAD>
struct MapStruct <TypeMap<HEAD>> {

    typedef TypeMap<HEAD> MapType;
    typedef typename MapType::Type Type;

    Type data;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            return data;
        } else {
            static_assert(
                AlwaysFalse<KEY>::VALUE,
                "Key does not exist in type map."
            );
            return UndefinedType::value;
        }
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
            static_assert(
                AlwaysFalse<KEY>::VALUE,
                "Key does not exist in type map."
            );
            return UndefinedType::value;
        }
    }
};



///////////////////////////////////////////////////////////////////////////////
// SetStruct
///////////////////////////////////////////////////////////////////////////////

template <typename SET> struct SetStruct;

template <typename... ITEMS>
struct SetStruct <TypeSet<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(
            AlwaysFalse<KEY>::VALUE,
            "Key does not exist in type set."
        );
    }
};


template <typename HEAD>
struct SetStruct <TypeSet<HEAD>> {

    typedef typename TypeSet<HEAD>::MapType MapType;
    typedef typename MapType::Type Type;

    Type data;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            return data;
        } else {
            static_assert(
                AlwaysFalse<KEY>::VALUE,
                "Item does not exist in type set."
            );
            return UndefinedType::value;
        }
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
            static_assert(
                AlwaysFalse<KEY>::VALUE,
                "Item does not exist in type set."
            );
            return UndefinedType::value;
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
// ArrayStruct
///////////////////////////////////////////////////////////////////////////////

template <typename SET> struct ArrayStruct;

template <typename... ITEMS>
struct ArrayStruct <TypeArray<ITEMS...>> {
    template<size_t INDEX>
    constexpr auto& get ()
    {
        static_assert(
            AlwaysFalse<TypeIndex<INDEX>>::VALUE,
            "Index does not exist in type array."
        );
    }
};


template <typename HEAD>
struct ArrayStruct <TypeArray<HEAD>> {

    typedef typename TypeArray<HEAD>::MapType MapType;
    typedef typename MapType::Type Type;

    Type data;

    template<size_t INDEX>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<TypeIndex<INDEX>>()) {
            return data;
        } else {
            static_assert(
                AlwaysFalse<TypeIndex<INDEX>>::VALUE,
                "Index does not exist in type array."
            );
            return UndefinedType::value;
        }
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
            static_assert(
                AlwaysFalse<TypeIndex<INDEX>>::VALUE,
                "Index does not exist in type array."
            );
            return UndefinedType::value;
        }
    }
};


#endif // HARMONIZE_CONTAINER_STRUCT


