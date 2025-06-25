
namespace sanity {

///////////////////////////////////////////////////////////////////////////////
// Checking that template aliases count as member templates of classes
///////////////////////////////////////////////////////////////////////////////

template <typename T>
struct SomeTemplate {
    typedef T Arg;
};

struct HasAlias {
    template <typename T>
    using _template = SomeTemplate<T>;
};


HasAlias:: template _template<void> has_alias;



} // namespace sanity


