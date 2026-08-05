#ifndef HARMONIZE_CONTEXT_DEMUX
#define HARMONIZE_CONTEXT_DEMUX


template<typename IMPL, typename LABEL>
struct DeMux{};

namespace multiplex {

    template<typename TYPE>
    struct IsDeMux {
        static constexpr bool value = false;
    };
    
    template<typename IMPL, typename LABEL>
    struct IsDeMux <DeMux<IMPL,LABEL>> {
        static constexpr bool value = true;
    };

    template<typename ACC, typename ELEM, typename ENABLE=void>
    struct ImplMapFolder;
   
    template <typename... ACC_BINDINGS, typename BINDING>
    struct ImplMapFolder <
        container::TypeMap<ACC_BINDINGS...>,
        BINDING,
        typename std::enable_if<!IsDeMux<typename BINDING::KeyType>::value>::type
    > {
        typedef typename container::TypeMap<ACC_BINDINGS...,BINDING> type;
    };
   
    template<typename... ACC_BINDINGS, typename IMPL, typename LABEL, typename REQ_SET>
    struct ImplMapFolder <
        container::TypeMap<ACC_BINDINGS...>,
        container::Binding<DeMux<IMPL,LABEL>,REQ_SET>,
        typename std::enable_if<container::TypeMap<ACC_BINDINGS...>::template has_key<IMPL>()>::type
    > {
        typedef container::TypeMap<ACC_BINDINGS...> Acc;
        typedef typename Acc::template ItemAt<IMPL>::type MuxReqSet;
        typedef typename MuxReqSet::template Union<REQ_SET>::type UpdatedMuxReqSet;
        typedef typename Acc::template UpdateItem<IMPL,UpdatedMuxReqSet>::type type;
    };
    
    template<typename... ACC_BINDINGS, typename IMPL, typename LABEL, typename REQ_SET>
    struct ImplMapFolder <
        container::TypeMap<ACC_BINDINGS...>,
        container::Binding<DeMux<IMPL,LABEL>,REQ_SET>,
        typename std::enable_if<!container::TypeMap<ACC_BINDINGS...>::template has_key<IMPL>()>::type
    > {
        typedef container::Binding<IMPL,REQ_SET> NextBinding;
        typedef typename container::TypeMap<ACC_BINDINGS...,NextBinding> type;
    };


    template<typename TYPE>
    struct RemoveDeMux {
        typedef TYPE type;
    };

    template<typename IMPL, typename LABEL>
    struct RemoveDeMux <DeMux<IMPL,LABEL>>{
        typedef IMPL type;
    };

    template<typename ITEM>
    struct TraitMapItemMapper {
        typedef typename ITEM::template Map<RemoveDeMux>::type type;
    };


}


template<typename STATE>
struct Multiplex {
    
    typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
    typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;

    typedef typename ImplMap ::template Fold<container::TypeMap<>,multiplex::ImplMapFolder>::type  UpdatedImplMap;
    typedef typename TraitMap::template MapItems<multiplex::TraitMapItemMapper>::type UpdatedTraitMap;
    
    typedef typename STATE
            ::template UpdateItem<context::key::ImplMap, UpdatedImplMap>::type 
            ::template UpdateItem<context::key::TraitMap,UpdatedTraitMap>::type 
            type;

};

#endif
