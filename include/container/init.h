#ifndef HARMONIZE_CONTAINER_INIT
#define HARMONIZE_CONTAINER_INIT

namespace container {


namespace init {

template<typename TRAIT, typename TUPLE>
struct Init {
    typedef TRAIT Trait;
    typedef TUPLE Tuple;

    TUPLE args;

    explicit Init(TUPLE&& args) : args(std::forward<TUPLE>(args)) {}
    Init(Init const&) = delete;
    Init& operator=(Init const&) = delete;
    Init(Init&&) = default;
    Init& operator=(Init&&) = delete;
};

template<typename TYPE>
struct IsInit {
    static constexpr bool value = false;
};

template<typename TRAIT, typename TUPLE>
struct IsInit<Init<TRAIT,TUPLE>> {
    static constexpr bool value = true;
};

template<typename TRAIT, typename... ARGS>
auto init(ARGS&&... args) {
    return Init<TRAIT,decltype(std::forward_as_tuple(std::forward<ARGS>(args)...))>(
        std::forward_as_tuple(std::forward<ARGS>(args)...)
    );
}


}

}

#endif
