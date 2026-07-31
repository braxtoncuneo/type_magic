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

template <>
struct MapStruct <TypeMap<>> {

    MapStruct() = default;
    MapStruct(MapStruct const &other) = default;
    MapStruct& operator=(MapStruct const &other) = default;
    MapStruct(MapStruct &&other) = default;

    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(
            AlwaysFalse<KEY>::VALUE,
            "Key does not exist in type map."
        );
    }

    template<typename KEY>
    constexpr auto& get () const
    {
        static_assert(
            AlwaysFalse<KEY>::VALUE,
            "Key does not exist in type map."
        );
    }

};


template <typename HEAD>
struct MapStruct <TypeMap<HEAD>> {

    typedef typename util::type_map::DefaultStructOrder<TypeMap<HEAD>>::type MapType;
    typedef typename HEAD::ItemType HeadItemType;

    private:
    HeadItemType data;
    
    public:
    MapStruct() = default;
    MapStruct(MapStruct const &other) = default;
    MapStruct& operator=(MapStruct const &other) = default;
    MapStruct(MapStruct &&other) = default;

    MapStruct (HeadItemType data)
        : data(data)
    {}

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

    template<typename KEY>
    constexpr auto& get () const
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

    //typedef typename util::type_map::DefaultStructOrder<TypeMap<HEAD,TAIL...>>::type MapType;
    typedef TypeMap<HEAD,TAIL...> MapType;
    typedef typename MapType::HeadItemType HeadItemType;
    typedef MapStruct<TypeMap<TAIL...>> TailType;

    private:
    HeadItemType data;
    TailType tail;
    

    public:
    MapStruct() = default;
    MapStruct(MapStruct const &other) = default;
    MapStruct& operator=(MapStruct const &other) = default;
    MapStruct(MapStruct &&other) = default;

    MapStruct (HeadItemType data, typename TAIL::ItemType... tail_items)
        : data(data)
        , tail(tail_items...)
    {}

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

    template<typename KEY>
    constexpr auto& get () const
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

    typedef typename TypeSet<ITEMS...>::MapType MapType;

    private:
    MapStruct<MapType> map;
    public:

    template<typename KEY>
    constexpr auto& get ()
    {
        return map.template get<KEY>();
    }

    template<typename KEY>
    constexpr auto& get () const
    {
        return map.template get<KEY>();
    }
    
    SetStruct() = default;
    SetStruct(SetStruct const&) = default;
    SetStruct &operator=(SetStruct const&) = default;

    SetStruct(ITEMS... args)
        : map(args...)
    {}
};


///////////////////////////////////////////////////////////////////////////////
// ArrayStruct
///////////////////////////////////////////////////////////////////////////////

template <typename ARRAY> struct ArrayStruct;

template <typename... ITEMS>
struct ArrayStruct <TypeArray<ITEMS...>> {

    typedef typename TypeArray<ITEMS...>::MapType MapType;

    private:
    MapStruct<MapType> map;

    public:

    template<size_t INDEX>
    constexpr auto& get ()
    {
        return map.template get<TypeIndex<INDEX>>();
    }

    template<size_t INDEX>
    constexpr auto& get () const
    {
        return map.template get<TypeIndex<INDEX>>();
    }

    ArrayStruct() = default;
    ArrayStruct(ArrayStruct const&) = default;
    ArrayStruct &operator=(ArrayStruct const&) = default;

    ArrayStruct(ITEMS... args)
        : map(args...)
    {}


};




}


#endif // HARMONIZE_CONTAINER_STRUCT


