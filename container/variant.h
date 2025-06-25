#ifndef HARMONIZE_CONTAINER_UNION
#define HARMONIZE_CONTAINER_UNION

#include "type.h"
#include "undef.h"
#include "assign.h"

///////////////////////////////////////////////////////////////////////////////
// MapVariant
///////////////////////////////////////////////////////////////////////////////

template <typename MAP> union MapVariant;

template <typename... ITEMS>
struct MapVariant <TypeMap<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type map.");
    }
};


template <typename HEAD, typename... TAIL>
struct MapVariant <TypeMap<HEAD,TAIL...>> {

    typedef TypeMap<HEAD,TAIL...> MapType;
    typedef typename MapType::Type Type;
    typedef MapVariant<TypeMap<TAIL...>> Tail;

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


///////////////////////////////////////////////////////////////////////////////
// SetVariant
///////////////////////////////////////////////////////////////////////////////

template <typename SET> union SetVariant;

template <typename... ITEMS>
struct SetVariant <TypeSet<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type set.");
    }
};


template <typename HEAD, typename... TAIL>
struct SetVariant <TypeSet<HEAD,TAIL...>> {

    typedef typename TypeSet<HEAD,TAIL...>::MapType MapType;
    typedef typename MapType::Type Type;
    typedef SetVariant<TypeSet<TAIL...>> Tail;

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



///////////////////////////////////////////////////////////////////////////////
// ArrayVariant
///////////////////////////////////////////////////////////////////////////////

template <typename SET> union ArrayVariant;

template <typename... ITEMS>
struct ArrayVariant <TypeArray<ITEMS...>> {
    template<size_t INDEX>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<TypeIndex<INDEX>>::VALUE,"Index does not exist in type array.");
    }
};


template <typename HEAD, typename... TAIL>
struct ArrayVariant <TypeArray<HEAD,TAIL...>> {

    typedef typename TypeArray<HEAD,TAIL...>::MapType MapType;
    typedef typename MapType::Type Type;
    typedef ArrayVariant<TypeSet<TAIL...>> Tail;

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


#endif // HARMONIZE_CONTAINER_UNION

