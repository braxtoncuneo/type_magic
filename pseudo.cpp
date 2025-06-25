



template<typename RUNTIME, template <typename> typename DEFINITION>
struct component_factory
{
    template<typename RUNTIME>
    struct meta {
        template<typename TRAIT>
        struct component {
            typedef DEFINITION<TRAIT>::typename type type;
        };
    };

};


template<typename Space>
struct storage
{
    typedef storage<Space>          default_type;
    typedef typeSet<Storage<Space>> implements_traits;
    typedef typeSet<>               requires_traits;
};



struct MyRuntimeSpec {

    typedef TypeSet <
        rt::Marker<mem::space::CPUGlobal>,
        type::Meta<type::BaseAlloc>,
        type::Meta<mem::Arena>,
        type::Meta<mem::SlabAlloc>,
        rt::LazyBind<
            type::Meta<mem::GeneralAlloc>,
            type::Meta<mem::DefaultGeneralAlloc>,
        >,
        rt::Bind<
            sched::CPUThread,
            sched::DefaultCPUThread
        >,
        rt::Bind<
            rng::FastRNG,
            rng::DefaultFastRNG
        >,
        rt::MetaBind<
            rng::FastRandom,
            rng::DefaultFastRandom
        >
    > Components;

};




