#ifndef HARMONIZE_CONTAINER_VARIANT
#define HARMONIZE_CONTAINER_VARIANT

#include "type.h"
#include "undef.h"
#include "assign.h"


namespace container {

///////////////////////////////////////////////////////////////////////////////
// MapVariant
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// SetVariant
///////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
// ArrayVariant
///////////////////////////////////////////////////////////////////////////////



template <typename ARRAY> struct ArrayVariant;

template <typename... ITEMS>
struct ArrayVariant <TypeArray<ITEMS...>> {

    typedef typename TypeArray<ITEMS...>::MapType MapType;

    static constexpr size_t ITEM_COUNT = TypeArray<ITEMS...>::MapType::ITEM_COUNT;

    private:
    size_t index_value;


    public:


    template<size_t INDEX>
    struct ItemAt {
        typedef typename MapType::template ItemAt<TypeIndex<INDEX>>::type type;
    };


    ArrayUnion<TypeArray<ITEMS...>> field;

    ArrayVariant <TypeArray<ITEMS...>> ()
        : index_value(ITEM_COUNT)
    {}

    constexpr size_t index() const
    {
        return index_value;
    }

    constexpr bool valid() const
    {
        return (index() >= 0) && (index() <ITEM_COUNT);
    }

    template<size_t INDEX>
    constexpr bool holds() const
    {
        return (INDEX == index());
    }

    template<size_t INDEX>
    inline void set (typename MapType::template ItemAt<TypeIndex<INDEX>>::type value)
    {
        field.get<INDEX>() = value;
        index_value = INDEX;
    }

    template<size_t INDEX>
    inline auto& unsafe_get () const
    {
        return field.get<INDEX>();
    }

    template<size_t INDEX>
    inline auto& try_get () const
    {
        if (holds<INDEX>()) {
            return &field.get<INDEX>();
        } else {
            return nullptr;
        }
    }

    private:

    template<typename VISITOR,size_t INDEX=0>
    static constexpr bool visitor_is_valid()
    {
        if constexpr (ITEM_COUNT == 0) {
            return false;
        } else if constexpr (INDEX == (ITEM_COUNT-1)) {
            return true;
        } else {
            constexpr bool match = std::is_same<
                typename std::result_of<VISITOR(typename ItemAt<INDEX>::type)>::type,
                typename std::result_of<VISITOR(typename ItemAt<INDEX+1>::type)>::type
            >::value;
            if constexpr (match) {
                return visitor_is_valid<VISITOR,INDEX+1>();
            } else {
                return false;
            }
        }
    }


    template<typename VISITOR, size_t INDEX=0>
    inline constexpr decltype(auto) visit_recurse(VISITOR &&visitor)
    {
        if (INDEX == index()) {
            return visitor(unsafe_get<INDEX>());
        } else {
            if constexpr (INDEX == (ITEM_COUNT-1)) {
                return visitor(unsafe_get<INDEX>());
            } else {
                return visit_recurse<VISITOR,INDEX+1>(visitor);
            }
        }
    }

    public:

    template<typename VISITOR>
    inline decltype(auto) visit(VISITOR &&visitor)
    {
        static_assert(
            visitor_is_valid<VISITOR>(),
            ASSERT_TEXT("Visitor's call implementations should return the same type for all items.")
        );
        return visit_recurse(visitor);
    }

};


}


#endif // HARMONIZE_CONTAINER_VARIANT

