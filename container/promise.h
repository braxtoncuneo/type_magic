#ifndef HARMONIZE_CONTAINER_PROMISE
#define HARMONIZE_CONTAINER_PROMISE

#include "type.h"

namespace container {

///////////////////////////////////////////////////////////////////////////////
// FuncMap
///////////////////////////////////////////////////////////////////////////////

template <typename MAP> struct FuncMap;

template <typename... ITEMS>
struct PromiseMap <TypeMap<ITEMS...>> {
    template<typename KEY,typename... ARGS>
    decltype(auto) func(ARGS... args)
    {
        return TypeMap<ITEMS...>::MapType::template ItemAt<KEY>::func(args...);
    }
};


///////////////////////////////////////////////////////////////////////////////
// FuncSet
///////////////////////////////////////////////////////////////////////////////

template <typename SET> struct FuncSet;

template <typename... ITEMS>
struct PromiseSet <TypeSet<ITEMS...>> {
    template<typename ITEM,typename... ARGS>
    decltype(auto) func(ARGS... args)
    {
        return TypeSet<ITEMS...>::MapType::template ItemAt<ITEM>::func(args...);
    }
};


///////////////////////////////////////////////////////////////////////////////
// FuncArray
///////////////////////////////////////////////////////////////////////////////

template <typename ARRAY> struct FuncArray;

template <typename... ITEMS>
struct PromiseArray <TypeArray<ITEMS...>> {
    template<size_t INDEX,typename... ARGS>
    decltype(auto) func(ARGS... args)
    {
        return TypeArray<ITEMS...>::MapType::template ItemAt<TypeIndex<INDEX>>::func(args...);
    }
};

}


#endif // HARMONIZE_CONTAINER_PROMISE