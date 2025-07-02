#ifndef HARMONIZE_FUNC_INFO
#define HARMONIZE_FUNC_INFO

#include "type.h"

namespace container {




template <typename FN>
struct IsFunction
{
    static constexpr bool value = false;
};

template <typename RESULT, typename... ARGS>
struct IsFunction <RESULT(ARGS...)>
{
    static constexpr bool value = true;
};

template <typename RESULT, typename... ARGS>
struct IsFunction <RESULT(*)(ARGS...)>
{
    static constexpr bool value = true;
};

template <typename RESULT, typename TYPE, typename... ARGS>
struct IsFunction <RESULT(TYPE::*)(ARGS...)>
{
    static constexpr bool value = true;
};






template <typename FN>
struct FuncInfo
{
    static_assert(
        AlwaysFalse<FN>::value,
        ASSERT_TEXT("Type parameter to FuncInfo is not a function.")
    );
};

template <typename RESULT, typename... ARGS>
struct FuncInfo <RESULT(ARGS...)>
{
    static constexpr bool       IS_POINTER = false;
    static constexpr bool       IS_METHOD  = false;
    typedef RESULT func_type(ARGS...);
    typedef RESULT              result_type;
    typedef TypeArray<ARGS...>  arg_types;
};

template <typename RESULT, typename... ARGS>
struct FuncInfo <RESULT(*)(ARGS...)>
{
    static constexpr bool       IS_POINTER = true;
    static constexpr bool       IS_METHOD  = false;
    typedef RESULT(*func_type)(ARGS...);
    typedef RESULT              result_type;
    typedef TypeArray<ARGS...>  arg_types;
};

template <typename RESULT, typename TYPE, typename... ARGS>
struct FuncInfo <RESULT(TYPE::*)(ARGS...)>
{
    static constexpr bool                IS_POINTER = true;
    static constexpr bool                IS_METHOD  = true;
    typedef RESULT(TYPE::*func_type)(ARGS...);
    typedef RESULT                       result_type;
    typedef TypeArray<ARGS...>           arg_types;
};


}


#endif // HARMONIZE_FUNC_INFO