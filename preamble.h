#ifdef COLOR_ASSERTS

#define STRINGIFY(x) STRINGIFY_HELPER(x)
#define STRINGIFY_HELPER(x) #x
#define LINE_STRING STRINGIFY(__LINE__)

#define ASSERT_TEXT(TEXT) "\n\n\e[33m\e[1m" __FILE__ ":" LINE_STRING " \e[0m\e[33m" TEXT "\e[0m\n"


#define ASSERT_HAS_MEMBER_HELPER(TYPE_NAME,MEMBER_NAME,MEMBER_SIGNATURE,EXTRA_TEXT) \
    ::InvokeCheck([](auto x){ return AlwaysTrue< decltype( decltype(x) :: MEMBER_NAME ) >::value;}).template can_invoke<TYPE_NAME>() 

/*
#define ASSERT_HAS_MEMBER_HELPER(TYPE_NAME,MEMBER_NAME,MEMBER_SIGNATURE,EXTRA_TEXT) \
    template < typename TYPE_NAME ## _ ## MEMBER_NAME ## _DETECTOR_ARG > \
    using TYPE_NAME ## _ ## MEMBER_NAME ## _DETECTOR = decltype( TYPE_NAME ## _ ## MEMBER_NAME ## _DETECTOR_ARG :: MEMBER_NAME ); \
    static_assert( \
        !::SpecializeOrFallBack< DoesNotExist, void, TYPE_NAME ## _ ## MEMBER_NAME ## _DETECTOR , TYPE_NAME >::fell_back, \
        ASSERT_TEXT("`" #MEMBER_NAME "` member of `" #TYPE_NAME "` does not exist" EXTRA_TEXT ) \
    ); \
    static_assert( \
        ::std::is_same<::SpecializeOrDNE< TYPE_NAME ## _ ## MEMBER_NAME ## _DETECTOR , TYPE_NAME >, MEMBER_SIGNATURE>::value, \
        ASSERT_TEXT("`" #MEMBER_NAME "` member of `" #TYPE_NAME "` does not have signature `" #MEMBER_SIGNATURE  "`" EXTRA_TEXT) \
    );
*/

#define ASSERT_HAS_MEMBER(TYPE_NAME,MEMBER_NAME,TYPE_SIGNATURE) ASSERT_HAS_MEMBER_HELPER(TYPE_NAME,MEMBER_NAME,TYPE_SIGNATURE,".")
#define ASSERT_HAS_MEMBER_FOR_TRAIT(TRAIT_NAME, TYPE_NAME,MEMBER_NAME,TYPE_SIGNATURE) ASSERT_HAS_MEMBER_HELPER(TYPE_NAME,MEMBER_NAME,TYPE_SIGNATURE,": The `" #TRAIT_NAME "` trait requires a member named `" #MEMBER_NAME "` with type `" #TYPE_SIGNATURE "`.")

#endif
