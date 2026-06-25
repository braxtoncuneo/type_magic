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
    public:

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
};


///////////////////////////////////////////////////////////////////////////////
// ArrayUnion
///////////////////////////////////////////////////////////////////////////////

template <typename SET> struct ArrayUnion;

template <typename... ITEMS>
struct ArrayUnion <TypeArray<ITEMS...>> {

    typedef typename TypeArray<ITEMS...>::MapType MapType;

    MapUnion<MapType> map;


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
};


}


#endif // HARMONIZE_CONTAINER_UNION

