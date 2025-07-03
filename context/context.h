
#include "../container/mod.h"


namespace context {

    template <typename... REQUIREMENTS>
    struct RequirementsList {
        typedef container::TypeArray<REQUIREMENTS...> Requirements;
    };


    template <typename... COMPONENTS>
    struct ComponentBundle {

        typedef container::TypeArray<COMPONENTS...> SubComponentList;

        template <typename REQ>
        struct Implements {
            static constexpr bool value = (COMPONENTS::template Implements<REQ>::value || ...);
        };

    };


    template <typename COMPONENT>
    struct Context {


    };


};


