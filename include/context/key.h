#ifndef HARMONIZE_CONTEXT_KEY
#define HARMONIZE_CONTEXT_KEY

namespace context {

namespace key {

    struct RootModule{};
    struct TraitMap{};
    struct ImplMap{};
    struct UnprunedMap{};
    struct UnculledMap{};
    struct TransformQueue{};
    struct RequirementSet{};
    struct CheckInfo{};
    struct ContextType{};


    namespace search {
        struct TraitFrontier{};
    };

    namespace cull {
        struct ReqTraits{};
        struct ReqImpls{};
        struct ReqTraitFrontier{};
        struct ReqImplFrontier{};
    }

    namespace unsat {
        struct FailEarly{};
        struct Traits{};
        struct Impls{};
        struct ReqTraits{};
        struct ReqImpls{};
        struct ReqTraitFrontier{};
        struct ReqImplFrontier{};
    }

}

}

#endif
