#ifndef HARMONIZE_CONTAINER_FUNC
#define HARMONIZE_CONTAINER_FUNC

#include "type.h"
#include "func_info.h"

namespace container {


template<auto FUNC>
struct Fn {
    static constexpr bool IS_FUNCTION = false;
    static constexpr bool IS_STATIC   = false;
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







template <typename... ARGS>
struct FnObj;

template <typename RETURN, typename... ARGS,RETURN FUNC(ARGS...)>
struct FnObj <Fn<FUNC>> {
    private:
    ArrayStruct<TypeArray<ARGS...>> arg_tuple;
    static constexpr size_t ARG_COUNT = TypeArray<ARGS...>::ITEM_COUNT;

    template<size_t INDEX, typename... UNPACK_ARGS>
    inline decltype(auto) exec(UNPACK_ARGS... args) {
        if constexpr (INDEX == ARG_COUNT) {
            return FUNC(args...);
        } else if ( (INDEX>=0) && (INDEX<ARG_COUNT) )  {
            return exec<INDEX+1>(args...,arg_tuple.template get<INDEX>());
        } else {
            static_assert(
                AlwaysFalse<TypeIndex<INDEX>>::value,
                ASSERT_TEXT("FnObj object invocation unrolling encountered an invalid parameter index.")
            );
            return UndefinedType::value;
        }
    }

    public:

    FnObj() = default;
    FnObj(FnObj const &) = default;
    FnObj& operator=(FnObj const &) = default;

    FnObj(ARGS... args)
        : arg_tuple(args...)
    {}

    inline decltype(auto) operator()() {
        return exec<0>();
    }

};

template <typename RETURN, typename CLASS, typename... ARGS,RETURN (CLASS::*FUNC)(ARGS...)>
struct FnObj <Fn<FUNC>> {
    private:
    ArrayStruct<TypeArray<CLASS*,ARGS...>> arg_tuple;
    static constexpr size_t ARG_COUNT = TypeArray<CLASS*,ARGS...>::ITEM_COUNT;

    template<size_t INDEX, typename... UNPACK_ARGS>
    inline decltype(auto) exec(UNPACK_ARGS... args) {
        if constexpr (INDEX == 0) {
            return (arg_tuple.template get<INDEX>()->*FUNC)(args...);
        } else if constexpr ( (INDEX>0) && (INDEX<ARG_COUNT) )  {
            return exec<INDEX-1>(arg_tuple.template get<INDEX>(),args...);
        } else {
            static_assert(
                AlwaysFalse<TypeIndex<INDEX>>::value,
                ASSERT_TEXT("FnObj object invocation unrolling encountered an invalid parameter index.")
            );
            return UndefinedType::value;
        }
    }

    public:
    
    FnObj() = default;
    FnObj(FnObj const &) = default;
    FnObj& operator=(FnObj const &) = default;

    FnObj(CLASS* self, ARGS... args)
        : arg_tuple(self,args...)
    {}

    inline decltype(auto) operator()() {
        return exec<ARG_COUNT-1>();
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

