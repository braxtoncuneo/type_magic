#ifndef HARMONIZE_CONTAINER_UNION
#define HARMONIZE_CONTAINER_UNION

#include "type.h"
#include "undef.h"
#include "assign.h"


namespace container {

///////////////////////////////////////////////////////////////////////////////
// MapUnion
///////////////////////////////////////////////////////////////////////////////

template <typename MAP> union MapUnion;

template <typename... ITEMS>
union MapUnion <TypeMap<ITEMS...>> {
    template<typename KEY>
    constexpr auto& get () const
    {
        static_assert(
            AlwaysFalse<KEY>::value,
            ASSERT_TEXT("Key does not exist in type map.")
        );
    }

    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(
            AlwaysFalse<KEY>::value,
            ASSERT_TEXT("Key does not exist in type map.")
        );
    }
};


template <typename HEAD, typename... TAIL>
union MapUnion <TypeMap<HEAD,TAIL...>> {

    typedef TypeMap<HEAD,TAIL...> MapType;
    typedef typename MapType::HeadItemType HeadItemType;
    typedef MapUnion<TypeMap<TAIL...>> TailType;

    private:
    HeadItemType data;
    TailType tail;

    template <typename TUPLE, std::size_t... INDEXES>
    MapUnion(init::Init<typename MapType::HeadKeyType,TUPLE> &&initializer,std::index_sequence<INDEXES...>)
        : data(std::get<INDEXES>(std::forward<TUPLE>(initializer.args))...)
    {}

    public:

    MapUnion() = default;
    MapUnion(MapUnion const &) = default;
    MapUnion &operator=(MapUnion const &) = default;

    template <typename KEY, typename TUPLE>
    MapUnion(init::Init<KEY,TUPLE> &&initializer)
        : tail(std::forward<init::Init<KEY,TUPLE>>(initializer))
    {}

    template <typename TUPLE>
    MapUnion(init::Init<typename MapType::HeadKeyType,TUPLE> &&initializer)
        : MapUnion(std::forward<init::Init<typename MapType::HeadKeyType,TUPLE>>(initializer),std::make_index_sequence<std::tuple_size_v<std::remove_reference_t<TUPLE>>>{})
    {}


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
                AlwaysFalse<KEY>::value,
                ASSERT_TEXT("Key does not exist in type map.")
            );
            return UndefinedType::value;
        }
    }

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
                AlwaysFalse<KEY>::value,
                ASSERT_TEXT("Key does not exist in type map.")
            );
            return UndefinedType::value;
        }
    }
};


///////////////////////////////////////////////////////////////////////////////
// SetUnion
///////////////////////////////////////////////////////////////////////////////

template <typename SET> struct SetUnion;

template <typename... ITEMS>
struct SetUnion <TypeSet<ITEMS...>> {

    typedef typename TypeSet<ITEMS...>::MapType MapType;

    MapUnion<MapType> map;
    
    SetUnion() = default;
    SetUnion(SetUnion const &) = default;
    SetUnion &operator=(SetUnion const &) = default;

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


    template <typename KEY, typename TUPLE>
    SetUnion(init::Init<KEY,TUPLE> &&initializer)
        : map(std::forward<init::Init<KEY,TUPLE>>(initializer))
    {}

};


///////////////////////////////////////////////////////////////////////////////
// ArrayUnion
///////////////////////////////////////////////////////////////////////////////

template <typename SET> struct ArrayUnion;

template <typename... ITEMS>
struct ArrayUnion <TypeArray<ITEMS...>> {

    typedef typename TypeArray<ITEMS...>::MapType MapType;

    MapUnion<MapType> map;

    ArrayUnion() = default;
    ArrayUnion(ArrayUnion const &) = default;
    ArrayUnion &operator=(ArrayUnion const &) = default;

    template<size_t INDEX>
    constexpr auto& get () const
    {
        return map.template get<TypeIndex<INDEX>>();
    }

    template<size_t INDEX>
    constexpr auto& get ()
    {
        return map.template get<TypeIndex<INDEX>>();
    }
    
    template <typename KEY, typename TUPLE>
    ArrayUnion(init::Init<KEY,TUPLE> &&initializer)
        : map(std::forward<init::Init<KEY,TUPLE>>(initializer))
    {}
};


}


#endif // HARMONIZE_CONTAINER_UNION

