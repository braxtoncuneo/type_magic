#ifndef HARMONIZE_CONTAINER_TYPE
#define HARMONIZE_CONTAINER_TYPE

#include "undef.h"

namespace container {

///////////////////////////////////////////////////////////////////////////////
// Forward-declare templates
///////////////////////////////////////////////////////////////////////////////


// Like a tuple, but indexes members by a type template parameter
template<typename... BINDINGS>
struct TypeMap;

template<typename... BINDINGS>
struct TypeSet;

template<typename... BINDINGS>
struct TypeArray;

// Used to help typemaps determine the held type for a given type key
template<typename KEY, typename MAP>
struct TypeMapLookup;

// Used to bind a certain key type to a certain held type for a TypeMap
template<typename KEY, typename TYPE>
struct Binding;




///////////////////////////////////////////////////////////////////////////////
// Template Specialization Checkers
///////////////////////////////////////////////////////////////////////////////



template<typename... BINDINGS>
struct IsTypeMap
{
    static constexpr bool value = false;
};

template<typename... BINDINGS>
struct IsTypeMap<TypeMap<BINDINGS...>>
{
    static constexpr bool value = true;
};


template<typename... BINDINGS>
struct IsTypeSet
{
    static constexpr bool value = false;
};

template<typename... BINDINGS>
struct IsTypeSet<TypeSet<BINDINGS...>>
{
    static constexpr bool value = true;
};

template<typename... BINDINGS>
struct IsTypeArray
{
    static constexpr bool value = false;
};

template<typename... BINDINGS>
struct IsTypeArray<TypeArray<BINDINGS...>>
{
    static constexpr bool value = true;
};



template<typename TYPE>
struct IsBinding
{
    static constexpr bool value = false;
};

template<typename KEY, typename ITEM>
struct IsBinding<Binding<KEY,ITEM>>
{
    static constexpr bool value = true;
};

}



#include "meta.h"



namespace container {

///////////////////////////////////////////////////////////////////////////////
// Set/Arg Converter
///////////////////////////////////////////////////////////////////////////////


template<typename TYPE>
struct SetFromArgs;

template <template<typename...> typename TEMPLATE, typename... ARGS>
struct SetFromArgs <TEMPLATE<ARGS...>> {
    typedef TypeSet<ARGS...> type;
};


///////////////////////////////////////////////////////////////////////////////
// define utility templates
///////////////////////////////////////////////////////////////////////////////


namespace util {


namespace type_set {

    template <typename A, typename B>
    struct BinaryUnion
    {
        typedef typename A::Union<B>::type type;
    };

}


namespace type_map {

    template<typename TYPE>
    struct KeySet {
        static_assert(
            IsTypeMap<TYPE>::value,
            ASSERT_TEXT("ERROR: Only TypeMap specializations are valid arguments for this template.")
        );
        typedef typename TYPE::KeySet type;
    };

    template<typename TYPE>
    struct ItemSet {
        static_assert(
            IsTypeMap<TYPE>::value,
            ASSERT_TEXT("ERROR: Only TypeMap specializations are valid arguments for this template.")
        );
        typedef typename TYPE::ItemSet type;
    };

    template <typename A, typename B>
    struct BinaryCombine
    {
        typedef typename A::Combine<B>::type type;
    };


}


    template <template <typename> typename FUNC>
    struct Negate {
        template<typename TYPE>
        struct Template {
            static constexpr bool value = !FUNC<TYPE>::value;
        };
    };


}



///////////////////////////////////////////////////////////////////////////////
// define representation functionality (used for diagnostics)
///////////////////////////////////////////////////////////////////////////////

namespace repr {
    
    struct StringReprNode {

        std::string open;
        std::vector<StringReprNode> content;
        std::string close;

        size_t depth() {
            int result = 1;
            for (StringReprNode element : content) {
                int local_depth = element.depth();
                result = result >= local_depth+1 ? result : local_depth + 1;
            }
            return result;
        }

        size_t raw_size() {
            size_t result = 0;
            result += open.size();
            for (StringReprNode element : content) {
                result += element.raw_size();
            }
            result += close.size();
            return result;
        }

        std::string to_string(int nesting_depth, bool force_indent) {
            std::string result = "";
            if (force_indent) {
                result += std::string(nesting_depth*4,' ');
            } else {
                result += " ";
            }
            result += open;
            if (content.size() == 0) {
                result += close + ',';
                return result;
            } else if ( (depth()<4) && (raw_size() < 200)) {
                for (StringReprNode node : content) {
                    result += node.to_string(nesting_depth+1,false);
                }
                result += ' ' + close + ',';
                return result;
            } else {
                for (StringReprNode node : content) {
                    result += '\n' + node.to_string(nesting_depth+1,true);
                }
                result += '\n' + std::string(nesting_depth*4,' ') + close + ',';
                return result;
            }
        }
        
        std::string to_string() {
            return to_string(0,true);
        }

    };

    template<typename TYPE>
    struct StringRepr;
    
    template<typename TYPE>
    struct StringContentRepr {
        static_assert(IsTypeArray<TYPE>::value,"Only TypeArrays may be supplied.");

        static void repr_recurse(std::vector<StringReprNode> &result) {
            result.push_back(StringRepr<typename TYPE::Front<>::type>::repr_node());
            StringContentRepr<typename TYPE::PopFront<>::type>::repr_recurse(result);
        }

        static std::vector<StringReprNode> repr() {
            std::vector<StringReprNode> result;
            StringContentRepr<TYPE>::repr_recurse(result);
            return result;
        }

    };
    
    template<>
    struct StringContentRepr <TypeArray<>> {
        static void repr_recurse(std::vector<StringReprNode> result) {}
        static std::vector<StringReprNode> repr() {
            return std::vector<StringReprNode>();
        }
    };
   

    template<typename TYPE>
    std::string type_name() {
        size_t size = 0;
        int    status = 0;
        char *demangled = abi::__cxa_demangle(typeid(TYPE).name(),nullptr,&size,&status);
        std::string name = "[demangle error]";
        if (status == 0) {
            name = demangled;
            free(demangled);
        }
        return name;
    }


    template<typename TYPE>
    struct StringRepr {
        static StringReprNode repr_node() {
            return StringReprNode{type_name<TYPE>(),{},""};
        }
        static std::string repr() {
            return repr_node().to_string();
        }
    };
    
    
    template<typename... TYPES>
    struct StringRepr <TypeMap<TYPES...>> {
        typedef TypeMap<TYPES...> Type; 
        static StringReprNode repr_node() {
            return StringReprNode {
                "TypeMap {",
                StringContentRepr<typename Type::BindingArray>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }
    };
    
    template<typename... TYPES>
    struct StringRepr <TypeSet<TYPES...>> {
        typedef TypeSet<TYPES...> Type; 
        static StringReprNode repr_node() {
            return StringReprNode {
                "TypeSet {",
                StringContentRepr<typename Type::MapType::KeyArray>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }
    };
    
    template<typename... TYPES>
    struct StringRepr <TypeArray<TYPES...>> {
        typedef TypeArray<TYPES...> Type; 
        static StringReprNode repr_node() {
            return StringReprNode {
                "TypeArray {",
                StringContentRepr<Type>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }
    };

    template<typename... TYPES>
    struct StringRepr <Binding<TYPES...>> {
        typedef Binding<TYPES...> Type; 
        static StringReprNode repr_node() {
            return StringReprNode {
                "Binding {",
                StringContentRepr<TypeArray<typename Type::KeyType,typename Type::ItemType>>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }
    };

    

}



///////////////////////////////////////////////////////////////////////////////
// define templates
///////////////////////////////////////////////////////////////////////////////




template<typename KEY, typename ITEM>
struct Binding
{
    typedef KEY  KeyType;
    typedef ITEM ItemType;
    
    static std::string repr_open() {
        return "Binding {";
    }

    static std::string repr_close() {
        return "}";
    }

    typedef TypeArray<KeyType,ItemType> ReprContent;
};


// Base case
template<typename... BINDINGS>
struct TypeMap
{

    typedef TypeSet  <>  KeySet;
    typedef TypeSet  <>  BindingSet;
    typedef TypeArray<>  KeyArray;
    typedef TypeArray<>  ItemArray;
    typedef TypeArray<>  BindingArray;

    static constexpr size_t ITEM_COUNT = 0;

    template<typename KEY>
    static constexpr bool has_key()
    {
        return false;
    }

    template<typename KEY>
    static constexpr bool has_item()
    {
        return false;
    }

    static constexpr bool has_duplicate_key()
    {
        return false;
    }

    static constexpr bool has_duplicate_item()
    {
        return false;
    }

    template <typename KEY>
    struct ItemAt {
        static_assert(
            AlwaysFalse<KEY>::value,
            ASSERT_TEXT("Key does not exist in type map.")
        );
        typedef UndefinedType type;
    };

    template <typename OTHER,typename ENABLE=void>
    struct LossyCombine {
        static_assert(
            IsTypeMap<OTHER>::value,
            "ERROR: LossyCombine operation can only occur between TypeMap specializations."
        );
        typedef OTHER type;
        static constexpr bool duplicate_key = false;
    };

    template <typename OTHER>
    struct Combine {
        typedef typename LossyCombine<OTHER>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct Filter {
        typedef TypeMap<> type;
    };

    template <template<typename> typename SELECTOR>
    struct FilterItems {
        typedef TypeMap<> type;
    };

    template <template<typename> typename SELECTOR>
    struct FilterKeys {
        typedef TypeMap<> type;
    };

    template <typename OTHER>
    struct Intersection;

    template <typename... ITEMS>
    struct Intersection <TypeMap<ITEMS...>> {
        typedef TypeMap<> type;
    };

    template <typename OTHER>
    struct Difference;

    template <typename... ITEMS>
    struct Difference <TypeMap<ITEMS...>> {
        typedef TypeMap<> type;
    };


    template <template<typename> typename MAPPER>
    struct Map {
        typedef TypeMap<> type;
    };

    template <template<typename> typename MAPPER>
    struct MapItems {
        typedef TypeMap<> type;
    };

    template <template<typename> typename MAPPER>
    struct MapKeys {
        typedef TypeMap<> type;
    };

    template <typename BASE, template<typename,typename> typename FOLDER>
    struct Fold {
        typedef BASE type;
    };

    template <typename BASE, template<typename,typename> typename FOLDER>
    struct FoldItems {
        typedef BASE type;
    };

    template <typename BASE, template<typename,typename> typename FOLDER>
    struct FoldKeys {
        typedef BASE type;
    };

    struct Invert {
        typedef TypeMap<> type;
    };

    template<typename PHONEY=void, typename ENABLE=void>
    struct SortHelper {
        typedef Binding<UndefinedType,char> BiggestItem;
        typedef TypeMap<> Remainder;
        typedef TypeMap<> RemainderSorted;
        typedef TypeMap<> type;
    };

    struct Sort {
        typedef TypeMap<> type;
    };

    struct DefaultStructOrder {
        typedef TypeMap<> type;
    };

};

// Recursive case
template<typename HEAD, typename... TAIL>
struct TypeMap <HEAD,TAIL...>
{

    static_assert(
        IsBinding<HEAD>::value,
        ASSERT_TEXT("Every argument to the TypeMap template must be a Binding specialization.")
    );

    typedef TypeMap<HEAD,TAIL...> SelfType;
    typedef TypeMap<TAIL...> TailType;
    typedef typename HEAD::KeyType  HeadKeyType;
    typedef typename HEAD::ItemType HeadItemType;

    typedef TypeSet  <typename HEAD::KeyType, typename TAIL::KeyType...>  KeySet;
    typedef TypeSet  <HEAD,TAIL...>                                       BindingSet;
    typedef TypeArray<typename HEAD::KeyType, typename TAIL::KeyType...>  KeyArray;
    typedef TypeArray<typename HEAD::ItemType,typename TAIL::ItemType...> ItemArray;
    typedef TypeArray<HEAD,TAIL...>                                       BindingArray;

    static constexpr size_t ITEM_COUNT = TailType::ITEM_COUNT + 1;

    template<typename KEY>
    static constexpr bool has_key()
    {
        if constexpr (std::is_same<KEY,HeadKeyType>::value) {
            return true;
        } else {
            return TailType::template has_key<KEY>();
        }
    }

    template<typename ITEM>
    static constexpr bool has_item()
    {
        if constexpr (std::is_same<ITEM,HeadItemType>::value) {
            return true;
        } else {
            return TailType::template has_item<ITEM>();
        }
    }

    static constexpr bool has_duplicate_key()
    {
        if constexpr (TailType::template has_key<HeadKeyType>()) {
            return true;
        } else {
            return TailType::has_duplicate_key();
        }
    }

    static constexpr bool has_duplicate_item()
    {
        if constexpr (TailType::template has_item<HeadItemType>()) {
            return true;
        } else {
            return TailType::has_duplicate_item();
        }
    }

    static_assert(
        (!TypeMap<HEAD,TAIL...>::has_duplicate_key()),
        ASSERT_TEXT("TypeMap cannot contain duplicate key types.")
    );


    template <typename KEY_QUERY,typename ENABLE=void>
    struct ItemAt;

    template <typename KEY_QUERY>
    struct ItemAt <
        KEY_QUERY,
        typename std::enable_if<!(std::is_same<HeadKeyType,KEY_QUERY>::value)>::type
    > {
        typedef typename TailType::template ItemAt<KEY_QUERY>::type type;
    };

    template <typename KEY_QUERY>
    struct ItemAt <
        KEY_QUERY,
        typename std::enable_if<std::is_same<HeadKeyType,KEY_QUERY>::value>::type
    > {
        typedef HeadItemType type;
    };


    template <typename OTHER,typename ENABLE=void>
    struct LossyCombine {
        static_assert(
            IsTypeMap<OTHER>::value,
            ASSERT_TEXT("ERROR: LossyCombine operation can only occur between TypeMap specializations.")
        );
    };

    template <typename... ITEMS>
    struct LossyCombine <
        TypeMap<ITEMS...>,
        typename std::enable_if<TypeMap<ITEMS...>::template has_key<HeadKeyType>()>::type
    > {
        typedef typename TailType::template LossyCombine<TypeMap<ITEMS...>>::type type;
        static constexpr bool duplicate_key = true;
    };

    template <typename... ITEMS>
    struct LossyCombine <
        TypeMap<ITEMS...>,
        typename std::enable_if<!(TypeMap<ITEMS...>::template has_key<HeadKeyType>())>::type
    > {
        typedef typename TailType::template LossyCombine<TypeMap<HEAD,ITEMS...>> TailCombine;
        typedef typename TailCombine::type type;
        static constexpr bool duplicate_key = TailCombine::duplicate_key;
    };


    template <typename OTHER>
    struct Combine {

        typedef LossyCombine<OTHER> LossyCombineType;

        static_assert(
            !LossyCombineType::duplicate_key,
            ASSERT_TEXT(
                "ERROR: Combine operation resulted in duplicate keys. "
                "If duplicates should be coalesced, use TypeMap's LossyCombine operation instead."
            )
        );

        typedef typename LossyCombineType::type type;
    };

    template <template<typename> typename SELECTOR,typename ENABLE=void>
    struct Filter;

    template <template<typename> typename SELECTOR>
    struct Filter <
        SELECTOR,
        typename std::enable_if<SELECTOR<HEAD>::value>::type
    > {
        typedef typename TypeMap<HEAD>::Combine<typename TailType::Filter<SELECTOR>::type>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct Filter <
        SELECTOR,
        typename std::enable_if<!(SELECTOR<HEAD>::value)>::type
    > {
        typedef typename TailType::Filter<SELECTOR>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct FilterItems
    {
        template <typename TYPE>
        struct FilterAdapter {
            static constexpr bool value = SELECTOR<typename TYPE::ItemType>::value;
        };
        typedef typename Filter<FilterAdapter>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct FilterKeys
    {
        template <typename TYPE>
        struct FilterAdapter {
            static constexpr bool value = SELECTOR<typename TYPE::ItemType>::value;
        };
        typedef typename Filter<FilterAdapter>::type type;
    };

    template <template<typename> typename MAPPER>
    struct Map {
        typedef typename TypeMap<typename MAPPER<HEAD>::type>::template Combine<TypeMap<typename MAPPER<TAIL>::type ...>>::type type;
    };

    template <template<typename> typename MAPPER>
    struct MapItems {
        template <typename TYPE>
        struct MapAdapter {
            typedef Binding<typename TYPE::KeyType,typename MAPPER<typename TYPE::ItemType>::type> type;
        };
        typedef typename Map<MapAdapter>::type type;
    };

    template <template<typename> typename MAPPER>
    struct MapKeys {
        template <typename TYPE>
        struct MapAdapter {
            typedef Binding<typename MAPPER<typename TYPE::KeyType>::type,typename TYPE::ItemType> type;
        };
        typedef typename Map<MapAdapter>::type type;
    };

    template <typename BASE, template<typename,typename> typename FOLDER>
    struct Fold {
        typedef typename TypeMap<TAIL...>::template Fold<typename FOLDER<BASE,HEAD>::type,FOLDER>::type type;
    };

    template <typename BASE, template<typename,typename> typename FOLDER>
    struct FoldItems {
        template <typename A, typename B>
        struct FoldAdapter {
            typedef typename FOLDER<A,typename B::ItemType>::type type;
        };
        typedef typename Fold<BASE,FoldAdapter>::type type;
    };

    template <typename BASE, template<typename,typename> typename FOLDER>
    struct FoldKeys {
        template <typename A, typename B>
        struct FoldAdapter {
            typedef typename FOLDER<A,typename B::KeyType>::type type;
        };
        typedef typename Fold<BASE,FoldAdapter>::type type;
    };


    struct Invert {
        typedef typename TypeMap<Binding<HeadItemType,HeadKeyType>>::template LossyCombine<typename TailType::Invert::type>::type type;
    };

    template <typename PHONEY=void, typename ENABLE=void>
    struct SortHelper;

    template <typename PHONEY>
    struct SortHelper<
        PHONEY,
        typename std::enable_if<sizeof(HeadItemType)>=sizeof(typename TailType::template SortHelper<>::BiggestItem::ItemType),PHONEY>::type
    > {
        typedef HEAD     BiggestItem;
        typedef TailType Remainder;
        typedef typename Remainder::Sort::type RemainderSorted;
        typedef typename TypeMap<BiggestItem>::template LossyCombine<RemainderSorted>::type type;
    };

    template <typename PHONEY>
    struct SortHelper<
        PHONEY,
        typename std::enable_if<sizeof(HeadItemType)<sizeof(typename TailType::template SortHelper<>::BiggestItem::ItemType),PHONEY>::type
    > {
        typedef typename TailType::template SortHelper<>::BiggestItem BiggestItem;
        typedef typename TailType::template SortHelper<>::Remainder   TailRemainder;
        typedef typename TypeMap<HEAD>::template LossyCombine<TailRemainder>::type Remainder;
        typedef typename Remainder::Sort::type RemainderSorted;
        typedef typename TypeMap<BiggestItem>::template LossyCombine<RemainderSorted>::type type;
    };

    struct Sort {
        typedef typename SortHelper<>::type type;
    };


    template <typename PHONEY=void, typename ENABLE=void>
    struct DefaultStructOrderHelper;

    template <typename PHONEY>
    struct DefaultStructOrderHelper<
        PHONEY,
        typename std::enable_if<config::REORDER_STRUCT_MEMBERS,PHONEY>::type
    > {
        typedef typename Sort::type type;
    };

    template <typename PHONEY>
    struct DefaultStructOrderHelper<
        PHONEY,
        typename std::enable_if<(!config::REORDER_STRUCT_MEMBERS),PHONEY>::type
    > {
        typedef TypeMap<HEAD,TAIL...> type;
    };

    struct DefaultStructOrder {
        typedef typename DefaultStructOrderHelper<>::type type;
    };

};





template<typename... ELEMENTS>
struct TypeSet
{

    typedef TypeMap<Binding<ELEMENTS,ELEMENTS>...> MapType;
    typedef TypeSet<ELEMENTS...> SelfType;

    static_assert(
        !MapType::has_duplicate_key(),
        ASSERT_TEXT("TypeSet cannot contain duplicate items.")
    );

    template<typename ITEM>
    static constexpr bool has_item()
    {
        return MapType::template has_key<ITEM>();
    }


    template<typename ITEM>
    struct HasItem {
        static constexpr bool value = has_item<ITEM>();
    };

    template <typename ITEM>
    struct ItemAt {
        typedef typename MapType::template ItemAt<ITEM> type;
    };

    template <typename... PARAMS>
    struct Union {
        typedef SelfType type;
    };

    template <typename HEAD, typename... TAIL>
    struct Union <HEAD,TAIL...> {
        static_assert(
            IsTypeSet<HEAD>::value,
            ASSERT_TEXT("TypeSet Union operations can only occur between TypeSet specializations.")
        );
        typedef typename MapType::template LossyCombine<typename HEAD::MapType>::type::KeySet::template Union<TAIL...>::type type;
    };


    template <typename OTHER>
    struct Intersection {

        static_assert(
            IsTypeSet<OTHER>::value,
            ASSERT_TEXT("TypeSet Intersection operations can only occur between TypeSet specializations.")
        );

        typedef OTHER OtherType;

        template<typename OTHER_ITEM>
        struct OtherHasItem {
            static constexpr bool value = OtherType::MapType::template has_key<typename OTHER_ITEM::KeyType>();
        };

        typedef typename MapType::template Filter<OtherHasItem>::type::KeySet type;
    };

    template <typename OTHER>
    struct Difference {

        static_assert(
            IsTypeSet<OTHER>::value,
            ASSERT_TEXT("TypeSet Difference operations can only occur between TypeSet specializations.")
        );

        typedef OTHER OtherType;

        template<typename OTHER_ITEM>
        struct OtherDoesNotHaveItem {
            static constexpr bool value = !OtherType::MapType::template has_key<typename OTHER_ITEM::KeyType>();
        };

        typedef typename MapType::template Filter<OtherDoesNotHaveItem>::type::KeySet type;
    };


    template <template<typename> typename MAPPER>
    struct LossyMap {

        typedef typename MapType::template MapItems<MAPPER>::type MappedMapType;
        static constexpr bool duplicate_items = MapType::has_duplicate_item();

        typedef typename MappedMapType::Invert::type::KeySet type;
    };


    template <template<typename> typename MAPPER>
    struct Map {

        typedef LossyMap<MAPPER> LossyMapType;

        static_assert(
            !LossyMapType::duplicate_items,
            ASSERT_TEXT(
                "ERROR: Map operation resulted in duplicate results, which would lead to duplicate items for the set. "
                "If duplicates should be coalesced, use TypeSet's LossyMap operation instead."
            )
        );

        typedef typename LossyMapType::type type;
    };

    template <typename BASE, template<typename,typename> typename FOLDER>
    struct Fold {
        typedef typename MapType::template FoldKeys<BASE,FOLDER>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct Filter {
        typedef typename MapType::template FilterKeys<SELECTOR>::type::KeySet type;
    };



    template <template<typename...> typename TEMPLATE>
    struct SpecializeWith {
        typedef TEMPLATE<ELEMENTS...> type;
    };


    template <template<typename...> typename TEMPLATE>
    struct CollapseAll {
        typedef typename Filter<Meta<TEMPLATE>::template Generalizes>::type MatchingTypes;
        typedef typename MatchingTypes::Map<SetFromArgs>::type ArgSets;
        typedef typename ArgSets::template Fold<TypeSet<>,util::type_set::BinaryUnion>::type CombinedSet;
        typedef typename CombinedSet::template SpecializeWith<TEMPLATE>::type type;
    };
    
};




template <size_t INDEX>
struct TypeIndex {
    static constexpr size_t value = INDEX;
};

template<typename... ELEMENTS>
struct TypeArray
{


    typedef TypeArray<ELEMENTS...> SelfType;

    template<size_t INDEX, typename... ELEMS>
    struct TypeMapGenerator;

    template<size_t INDEX>
    struct TypeMapGenerator <INDEX> {
        typedef TypeMap<> type;
    };

    template<size_t INDEX, typename HEAD, typename... TAIL>
    struct TypeMapGenerator <INDEX,HEAD,TAIL...> {
        typedef typename TypeMapGenerator<INDEX+1,TAIL...>::type TailMap;
        typedef typename TypeMap<Binding<TypeIndex<INDEX>,HEAD>>::template LossyCombine<TailMap>::type type;
    };

    typedef typename TypeMapGenerator<0,ELEMENTS...>::type MapType;

    template<size_t INDEX>
    static constexpr bool has_index()
    {
        return MapType::template has_key<TypeIndex<INDEX>> ();
    }

    template <size_t INDEX>
    struct ItemAt {
        typedef typename MapType::template ItemAt<TypeIndex<INDEX>>::type type;
    };

    template<typename... PARAMS>
    struct Concatenate {
        typedef TypeArray<ELEMENTS...> type;
    };

    template<typename HEAD, typename... TAIL>
    struct Concatenate <HEAD,TAIL...>{
        static_assert(
            IsTypeArray<HEAD>::value,
            ASSERT_TEXT("TypeArray Concatenate operations can only occur between TypeArray specalizations.")
        );
    };

    template<typename... HEAD_ELEMENTS, typename... TAIL>
    struct Concatenate <TypeArray<HEAD_ELEMENTS...>,TAIL...>{
        typedef typename TypeArray<ELEMENTS...,HEAD_ELEMENTS...>::template Concatenate<TAIL...>::type type;
    };

    template <template<typename> typename SELECTOR>
    struct Filter {
        typedef typename MapType::template Filter<SELECTOR>::type::ItemArray type;
    };

    static constexpr bool IS_EMPTY = (MapType::ITEM_COUNT == 0);

    template<typename TYPE=void>
    struct PopFront {
        static_assert(
            !IS_EMPTY,
            ASSERT_TEXT("ERROR: Attempted to use PopFront for an empty TypeArray.")
        );
        typedef typename MapType::TailType::ItemArray type;
    };

    template<typename TYPE=void>
    struct Front {
        static_assert(
            !IS_EMPTY,
            ASSERT_TEXT("ERROR: Attempted to use Front for an empty TypeArray.")
        );
        typedef typename MapType::HeadItemType type;
    };

    template<typename TYPE>
    struct PushFront {
        typedef TypeArray<TYPE,ELEMENTS...> type;
    };

    template<typename TYPE>
    struct PushBack {
        typedef TypeArray<ELEMENTS...,TYPE> type;
    };
    

};







template<typename TYPE>
struct EnsureTypeSetWrap
{
    typedef TypeSet<TYPE> type;
};

template<typename... SET_ARGS>
struct EnsureTypeSetWrap <TypeSet<SET_ARGS...>>
{
    typedef TypeSet<SET_ARGS...> type;
};


template<typename TYPE>
struct EnsureTypeArrayWrap
{
    typedef TypeArray<TYPE> type;
};

template<typename... ARRAY_ARGS>
struct EnsureTypeArrayWrap <TypeArray<ARRAY_ARGS...>>
{
    typedef TypeArray<ARRAY_ARGS...> type;
};





}


#endif // HARMONIZE_CONTAINER_TYPE

