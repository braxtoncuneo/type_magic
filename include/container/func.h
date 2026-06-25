#ifndef HARMONIZE_CONTAINER_FUNC
#define HARMONIZE_CONTAINER_FUNC

#include "type.h"
#include "func_info.h"

namespace container {




///////////////////////////////////////////////////////////////////////////////
// Function
///////////////////////////////////////////////////////////////////////////////

template <auto FUNC, typename BINDMAP>
struct Function;

template <
    auto FUNC,
    typename... BINDINGS
>
struct Function <FUNC,TypeMap<BINDINGS...>>
{

    static_assert(
        IsFunction<decltype(FUNC)>::value,
        "First argument for the Promise template type must be a function."
    );
    static_assert(
        std::is_same<typename FuncInfo<decltype(FUNC)>::arg_types::TypeMap,TypeMap<BINDINGS...>>::value,
        "Function objects currently only support no bindings or full bindings. The given bindings do not match the full argument list."
    );

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
                "Function object invocation unrolling encountered an invalid parameter index."
            );
            return UndefinedType();
        }
    }

    public:

    template <typename... ARGS>
    Function(ARGS... args)
        : arg_tuple(args...)
    {
        static_assert(
            std::is_same<TypeArray<ARGS...>,typename FuncInfo<decltype(FUNC)>::arg_types>::value,
            "Arguments provided to Function invocation do not match the set of unbound parameters."
        );
    }

    template <typename... ARGS>
    inline decltype(auto) operator()() {
        static_assert(
            std::is_same<TypeArray<ARGS...>,TypeArray<>>::value,
            "Function object is fully bound, and so should not accept any explicit parameters through its call operator."
        );
        exec<0>();
    }

};

template <auto FUNC>
struct Function <FUNC,TypeMap<>>
{

    static_assert(
        IsFunction<decltype(FUNC)>::value,
        "First argument for the Promise template type must be a function."
    );

    template <typename... ARGS>
    inline decltype(auto) operator()(ARGS... args) {
        static_assert(
            std::is_same<TypeArray<ARGS...>,typename FuncInfo<decltype(FUNC)>::arg_types>::value,
            "Arguments provided to Function invocation do not match the set of unbound parameters."
        );
        FUNC(args...);
    }


    template <typename... ARGS>
    inline decltype(auto) bind (ARGS... args) {
        static_assert(
            std::is_same<TypeArray<ARGS...>,typename FuncInfo<decltype(FUNC)>::arg_types>::value,
            "Arguments provided to Function invocation do not match the set of unbound parameters."
        );
        Function<FUNC,typename TypeArray<ARGS...>::TypeMap>(args...);
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

