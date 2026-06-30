#ifndef HARMONIZE_CONTAINER_META
#define HARMONIZE_CONTAINER_META


template<typename T>
struct AlwaysFalse
{
    static constexpr bool value = false;
};

template<typename T>
struct AlwaysTrue
{
    static constexpr bool value = false;
};

template<typename T>
struct AlwaysVoid
{
    typedef void type;
};

struct Never{
    Never() = delete;
    ~Never() = delete;
    Never(Never const&) = delete;
    Never operator=(Never const&) = delete;
};

struct DoesNotExist : Never {};


// Essentially the detector idiom
template <class DEFAULT, class SHOULD_BE_VOID, template <class...> class TEMPLATE, class... ARGS>
struct SpecializeOrFallBack {
    static constexpr bool fell_back = true;
    typedef std::true_type fell_back_t;
    typedef DEFAULT type;
};

template <class DEFAULT, template <class...> class TEMPLATE, class... ARGS>
struct SpecializeOrFallBack<DEFAULT, typename AlwaysVoid<TEMPLATE<ARGS...>>::type, TEMPLATE, ARGS...> {
    static constexpr bool fell_back = false;
    typedef std::false_type fell_back_t;
    typedef TEMPLATE<ARGS...> type;
};


// Detector idiom, but for overload checks
template <typename FUNC>
struct InvokeCheck {

    constexpr InvokeCheck(FUNC f) {}

    template<typename TYPE>
    struct Invoker {

        auto invoke(FUNC f, TYPE value) {
            return f(value);
        }

    };

    template<typename TYPE>
    constexpr bool can_invoke() {
        return !::SpecializeOrFallBack< int, void, Invoker , TYPE>::fell_back;
    }
};



template <template <class...> class TEMPLATE, class... ARGS>
using SpecializeOrNever = typename SpecializeOrFallBack<Never, void, TEMPLATE, ARGS...>::type;

template <template <class...> class TEMPLATE, class... ARGS>
using SpecializeOrDNE = typename SpecializeOrFallBack<DoesNotExist, void, TEMPLATE, ARGS...>::type;

template <typename DEFAULT, template <class...> class TEMPLATE, class... ARGS>
using SpecializeOr = typename SpecializeOrFallBack<DEFAULT, void, TEMPLATE, ARGS...>::type;




// Holds a template for later application
template<template <typename...> typename TEMPLATE>
struct Meta
{
    template <typename... ARGS>
    struct Template {
        typedef TEMPLATE<ARGS...> Type;
    };

    template <typename... ARGS>
    struct CanSpecialize {
        static constexpr bool value = !SpecializeOrNever<TEMPLATE,ARGS...>::fell_back;
    };

    template<typename TYPE>
    struct Generalizes {
        static constexpr bool value = false;
    };

    template<typename...ARGS>
    struct Generalizes <TEMPLATE<ARGS...>> {
        static constexpr bool value = true;
    };

    template<typename TYPE_SET>
    struct SpecializeFromTypeSet {
        static_assert(
            container::IsTypeSet<TYPE_SET>::value,
            ASSERT_TEXT("ERROR: Only TypeSet specializations may be passed to Meta's SepecializeFromTypeSet member template.")
        );
        typedef typename TYPE_SET::SpecializeWith<TEMPLATE>::type type;
    };

};

namespace _util {

template <typename TYPE>
struct GetTemplateArgsHelper {
    typedef container::TypeArray<> type;  
};

template <template <typename...> typename TEMPLATE, typename... ARGS>
struct GetTemplateArgsHelper <TEMPLATE<ARGS...>> {
    typedef container::TypeArray<ARGS...> type;  
};

}


template<typename TYPE>
using GetTemplateArgs = _util::GetTemplateArgsHelper<TYPE>::type;


// Specializes the template provided
template<typename TYPE, typename... ARGS>
struct UnMeta
{
    typedef TYPE Type;
};

template<template <typename> typename TEMPLATE, typename... ARGS>
struct UnMeta <Meta<TEMPLATE>,ARGS...>
{
    typedef TEMPLATE<ARGS...> Type;
};


template<typename TYPE>
struct EnsureMetaWrap
{

    template <typename... ARGS>
    struct Template {
        typedef TYPE Type;
    };
    
    typedef Meta<Template> Type;
};

template<template<typename...>typename TEMPLATE>
struct EnsureMetaWrap <Meta<TEMPLATE>>
{
    typedef Meta<TEMPLATE> Type;
};


#endif // HARMONIZE_CONTAINER_META
