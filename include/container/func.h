#ifndef HARMONIZE_CONTAINER_FUNC
#define HARMONIZE_CONTAINER_FUNC

#include "type.h"
#include "func_info.h"

namespace container {


template<auto FUNC>
struct Fn {
    static constexpr bool IS_FUNCTION = false;
};

template<typename RETURN, typename CLASS, typename... ARGS,RETURN (CLASS::*FUNC)(ARGS...)>
struct Fn <FUNC> {
    static constexpr bool IS_FUNCTION = true;
    static constexpr bool IS_STATIC   = false;
    typedef RETURN Return;
    typedef CLASS Class;
    typedef container::TypeArray<ARGS...> NonStaticArgs;
    typedef container::TypeArray<CLASS*,ARGS...> Args;
    typedef Return (Class::* NonStaticSignature)(ARGS...);
    typedef Return FnObjSignature (CLASS*,ARGS...);
    static constexpr auto non_static_function = FUNC;

    static Return function(Class *self, ARGS... args) {
        return (self->*non_static_function)(args...);
    }
};

template<typename RETURN, typename... ARGS,RETURN FUNC(ARGS...)>
struct Fn <FUNC> {
    static constexpr bool IS_FUNCTION = true;
    static constexpr bool IS_STATIC   = true;
    typedef RETURN Return;
    typedef container::TypeArray<ARGS...> Args;
    typedef Return FnObjSignature (ARGS...);
    static constexpr auto function = FUNC;
};



template <typename TYPE>
struct IsFnTrait {
    static constexpr bool value = false;
};


template <auto FUNC>
struct IsFnTrait <Fn<FUNC>> {
    static constexpr bool value = false;
};





template<template<typename>typename METHOD_MIXIN>
struct Method {
    
    template<typename CONTEXT>
    struct Function {
        typedef Fn<METHOD_MIXIN<CONTEXT>::operator()> type;
    };

};






/*

template <typename... ARGS>
struct FnObj;

template <typename RETURN, typename... ARGS,RETURN FUNC(ARGS...)>
struct FnObj <Fn<FUNC>> {
    private:
    MapStruct<BINDINGS...> arg_tuple;

    template<size_t INDEX, typename... ARGS>
    inline decltype(auto) exec(ARGS... args) {
        constexpr size_t count = MapStruct<BINDINGS...>::ITEM_COUNT;
        if constexpr (INDEX == count) {
            return FUNC(args...);
        } else if ( (INDEX>=0) && (INDEX<count) )  {
            return exec<INDEX+1>(args...);
        } else {
            static_assert(
                AlwaysFalse<TypeIndex<INDEX>>::value,
                ASSERT_TEXT("FnObj object invocation unrolling encountered an invalid parameter index.")
            );
            return UndefinedType::value;
        }
    }

    public:

    template <typename... ARGS>
    FnObj(ARGS... args)
        : arg_tuple(args...)
    {
        static_assert(
            std::is_same<TypeArray<ARGS...>,typename Fn<FUNC>::NonStaticArgs>::value,
            "Arguments provided to FnObj invocation do not match the set of unbound parameters."
        );
    }

    template <typename... ARGS>
    inline decltype(auto) operator()(ARGS... args) {
        static_assert(
            std::is_same<TypeArray<ARGS...>,TypeArray<typename Fn<FUNC>::Class*>>::value,
            "FnObj object is fully bound except for an implicit parameter. It will only accept parameters through its call operator."
        );
        exec<0>(args...);
    }

};





///////////////////////////////////////////////////////////////////////////////
// FuncMap
///////////////////////////////////////////////////////////////////////////////

template <typename MAP> struct FuncMap;

template <typename... ITEMS>
struct FuncMap <TypeMap<ITEMS...>> {
    template<typename KEY,typename... ARGS>
    decltype(auto) func(ARGS... args)
    {
        return TypeMap<ITEMS...>::MapType::template ItemAt<KEY>::func(args...);
    }
};
*/

///////////////////////////////////////////////////////////////////////////////
// FuncSet
///////////////////////////////////////////////////////////////////////////////

template <typename SET> struct FuncSet;

template <typename... ITEMS>
struct FuncSet <TypeSet<ITEMS...>> {
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
struct FuncArray <TypeArray<ITEMS...>> {
    template<size_t INDEX,typename... ARGS>
    decltype(auto) func(ARGS... args)
    {
        return TypeArray<ITEMS...>::MapType::template ItemAt<TypeIndex<INDEX>>::func(args...);
    }
};




}


#endif // HARMONIZE_CONTAINER_FUNC

