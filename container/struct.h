#ifndef HARMONIZE_CONTAINER_STRUCT
#define HARMONIZE_CONTAINER_STRUCT

#include "undef.h"
#include "type.h"
#include "assign.h"


namespace container {

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

    typedef typename TypeMap<HEAD>::DefaultStructOrder::type MapType;
    typedef typename MapType::HeadItemType HeadItemType;

    HeadItemType data;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType:: template has_key<KEY>()) {
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

    typedef typename TypeMap<HEAD,TAIL...>::DefaultStructOrder::type MapType;
    typedef typename MapType::HeadItemType HeadItemType;
    typedef MapStruct<TypeMap<TAIL...>> TailType;

    HeadItemType data;
    TailType tail;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            if constexpr (std::is_same<KEY,typename MapType::HeadKeyType>::value) {
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
    typedef typename MapType::HeadItemType HeadItemType;

    HeadItemType data;

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
    typedef typename MapType::HeadItemType HeadItemType;
    typedef SetStruct<TypeMap<TAIL...>> TailType;

    HeadItemType data;
    TailType tail;

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<KEY>()) {
            if constexpr (std::is_same<KEY,typename MapType::HeadKeyType>::value) {
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
    typedef typename MapType::HeadItemType HeadItemType;

    HeadItemType data;

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
    typedef typename MapType::HeadItemType HeadItemType;
    typedef ArrayStruct<TypeArray<TAIL...>> TailType;

    HeadItemType data;
    TailType tail;

    template<size_t INDEX>
    constexpr auto& get ()
    {
        if constexpr (MapType::template has_key<TypeIndex<INDEX>>()) {
            if constexpr (std::is_same<TypeIndex<INDEX>,typename MapType::HeadKeyType>::value) {
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


}


#endif // HARMONIZE_CONTAINER_STRUCT


