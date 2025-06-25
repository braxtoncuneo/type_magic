#include <type_traits>

///////////////////////////////////////////////////////////////////////////////
// Forward-declare templates
///////////////////////////////////////////////////////////////////////////////


// Like a tuple, but indexes members by a type template parameter
template<typename... BINDINGS>
struct TypeMap;

// Used to help typemaps determine the held type for a given type key
template<typename KEY, typename MAP>
struct TypeMapLookup;

// Used to bind a certain key type to a certain held type for a TypeMap
template<typename KEY, typename TYPE>
struct Binding;




///////////////////////////////////////////////////////////////////////////////
// Define templates
///////////////////////////////////////////////////////////////////////////////


// Holds a template for later application
template<template <typename> typename TEMPLATE>
struct Meta
{
    template <typename ARG>
    struct Template {
        typedef TEMPLATE<ARG> Type;
    };
};

// Specializes the template provided
template<typename TYPE, typename ARG>
struct UnMeta
{
    typedef TYPE Type;
};

template<template <typename> typename TEMPLATE, typename ARG>
struct UnMeta <Meta<TEMPLATE>,ARG>
{
    typedef TEMPLATE<ARG> Type;
};


template<typename KEY, typename TYPE>
struct Binding
{
    typedef KEY  Key;
    typedef TYPE Type;
};

struct UndefinedType {};

template<typename T>
struct AlwaysFalse
{
    static constexpr bool VALUE = false;
};

template<typename... ELEMENTS>
struct TypeSet
{
    template<typename ITEM>
    static constexpr bool has_item()
    {
        return false;
    }

    static constexpr bool has_duplicates()
    {
        return false;
    }

    template<typename ITEM>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<ITEM>::VALUE,"Key does not exist in type map.");
    }
};

template<typename HEAD, typename... TAIL>
struct TypeSet <HEAD,TAIL...>
{

    typedef HEAD             Type;
    typedef TypeSet<TAIL...> Tail;

    Type data;
    Tail tail;

    template<typename ITEM>
    static constexpr bool has_item()
    {
        if constexpr (std::is_same<ITEM,HEAD>::value) {
            return true;
        } else {
            return Tail::template has_item<ITEM>();
        }
    }

    static constexpr bool has_duplicates()
    {
        if constexpr (Tail::template has_item<HEAD>()) {
            return true;
        } else {
            return Tail::has_duplicates();
        }
    }

    template<typename ITEM>
    constexpr auto& get ()
    {
        if constexpr (TypeSet<HEAD,TAIL...>::template has_item<ITEM>()) {
            if constexpr (std::is_same<ITEM,Type>::value) {
                return data;
            } else {
                return tail.template get<ITEM>();
            }
        } else {
            static_assert(AlwaysFalse<ITEM>::VALUE,"Key does not exist in type map.");
            UndefinedType *dummy = nullptr;
            return *dummy;
        }
    }
};



// Base case
template<typename... BINDINGS>
struct TypeMap
{
    template<typename KEY>
    static constexpr bool has_key()
    {
        return false;
    }

    template<typename KEY>
    constexpr auto& get ()
    {
        static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type map.");
    }

};

// Recursive case
template<typename HEAD, typename... TAIL>
struct TypeMap <HEAD,TAIL...>
{

    typedef TypeMap<TAIL...> Tail;
    typedef typename HEAD::Key  Key;
    typedef typename HEAD::Type Type;

    Type data;
    Tail tail;

    template<typename KEY>
    static constexpr bool has_key()
    {
        if constexpr (std::is_same<KEY,Key>::value) {
            return true;
        } else {
            return Tail::template has_key<KEY>();
        }
    }

    template<typename KEY>
    constexpr auto& get ()
    {
        if constexpr (TypeMap<HEAD,TAIL...>::has_key<KEY>()) {
            if constexpr (std::is_same<KEY,Key>::value) {
                return data;
            } else {
                return tail.template get<KEY>();
            }
        } else {
            static_assert(AlwaysFalse<KEY>::VALUE,"Key does not exist in type map.");
            UndefinedType *dummy = nullptr;
            return *dummy;
        }
    }

};

/*
template<typename EXEC_TYPE>
struct Schedule
{
    template<typename TASK_TYPE,typename... ARGS>
    void exec(RUNTIME rt, )
    {

    }
};


template<typename RT> struct Odd;
template<typename RT> struct Even;

template<typename RT>
struct Even
{
	auto operator()(RT rt, int original, int value, int step_count) {
		if (value <= 1) {
			return;
		}
		value /= 2;
		if (value%2 == 0) {
			rt.as<Schedule<GPU_thread>>().exec<Even<RT>>(original,value,step_count);
		} else {
			rt.as<Schedule<GPU_thread>>().exec<Odd<RT>> (original,value,step_count);
		}
	}
};

template<typename RT>
struct Odd
{
	auto operator()(RT rt, int original, int value, int step_count) {
		if (value <= 1) {
			return;
		}
		value /= 2;
		if (value%2 == 0) {
			rt.Schedule<GPU_thread>::exec<Even<RT>>(original,value,step_count);
		} else {
			rt.Schedule<GPU_thread>::exec<Odd<RT>> (original,value,step_count);
		}
	}
};

*/






template<typename TASK,typename RT>
struct Task {

    typedef typename UnMeta<TASK,RT>::Type TaskType;

    RT       &rt;
    TaskType &task;

    Task(TaskType &task, RT rt)
        : rt(rt)
        , task(task)
    {}

    template<typename...ARGS>
    auto operator()(ARGS... args) {
        task(rt,args...);
    }
};

template<typename RUNTIME> struct Odd;
template<typename RUNTIME> struct Even;

#include <iostream>

template<typename MOD,typename RT>
MOD get(RT& rt) {
    return MOD(rt.set.template get<MOD>(),rt);
}

/*
template<template<typename> typename TASK,typename RT>
struct Service {
    RT& runtime;
    TASK<RT>& task;

    Service<TASK,RT>(TASK<RT> &task, RT &rt) : task(task), runtime(rt) {}
};
*/


template<typename RT>
struct Even
{
    void operator()(RT rt, int original, int value, int count)
    {
        value /= 2;
        if(value <= 1) {
            std::cout << "Value " << original << " took " << count << " steps." << std::endl;
            return;
        }
        count++;
        if ((value%2) == 0) {
            get<Meta<Even>>(rt)(original,value,count);
        } else {
            get<Meta<Odd>> (rt)(original,value,count);
        }
    }
};

template<typename RT>
struct Odd
{
    void operator()(RT rt, int original, int value, int count)
    {
        value = value *3 + 1;
        if(value <= 1) {
            std::cout << "Value " << original << " took " << count << " steps." << std::endl;
            return;
        }
        count++;
        if ((value%2) == 0) {
            get<Meta<Even>>(rt)(original,value,count);
        } else {
            get<Meta<Odd>> (rt)(original,value,count);
        }
    }
};


template<typename... MODULES>
struct Runtime
{
    typedef Runtime<MODULES...> Self;

    template<typename TYPE>
    struct ModuleType
    {
        typedef TYPE Type;
    };

    template<template<typename> typename TEMPLATE>
    struct ModuleType <Meta<TEMPLATE>>
    {
        typedef TEMPLATE<Runtime> Type;
    };

    typedef TypeSet<ModuleType<MODULES>::typename Type> SetType;
    SetType set;

    template<typename MOD>
    struct get_adapter
    {
        MOD& get(SetType& set)
        {
            return set.template get<ModuleType<MOD>::typename Type>();
        }
    };

    template<template <typename> typename MOD>
    struct get_adapter <Meta<MOD>>
    {
        MOD<Runtime<MODULES...>>& get(SetType& set)
        {
            return set.template get<ModuleType<Meta<MOD>>::typename Type>();
        }
    };

    template<typename MOD>
    auto& get()
    {
        return get_adapter<MOD>::get(set);
    }

};


// Type Containers:
// - Meta<template<typename>> : Represents a template taking one type parameter
// - TypeSet<typename...>     : Represents a set of types
// - TypeList<typename...>    : Represents a sequence of types. Provides `Set` as a conversion to TypeSet.
// - TypeMap<typename...>     : Represents a mapping of types to other types. Provides TypeList of keys and their corresponding items as `Key` and `Item`

// Type Processors:
// - TryInstantiate<Input,Arg>  : Provides Temp<Arg> as Type if Input is Meta<Temp>. If not, Type is just Input.
// - Filter<Meta<Temp>,TypeSet> : Filters for types in TypeSet where Temp<type>::VALUE == true, provides it as Type
// - Flatten<TypeList<>>        : Expands all TypeList types directly cont
// - FlattenRec<TypeList<>>     :
// - For<TypeList<>,TypeList<>>




// Component Descriptors:
// - Req<type>                 : Requires the `type` trait to be implemented to be used.
// - Impl<type>                : Directly implements the `type` trait.
// -

// - All<template<typename>>   : Corresponds to all specializations of the template.
// - Mixin<template<typename>> : Indicates that the underlying template should have the runtime type plugged in
// -



// Impl<For<Each<Meta<SlabAllocator<RT>>>::Type,TypedAllocator<RT> >::Type>



// Bridge type :
//    Used as a handle to represent a set of functionality and as a guard to
//    check whether or not a type implements that functionality. Concepts may
//    be used by bridges if compiled for C++20 and above.
//
// ComponentType :
//     Literally any type.
// ComponentMixin :
//     A template which produces components based off of an input runtime
//     type. This allows one to write code that can adapt based upon the
//     underlying runtime, and its other components.
// ComponentTemplate :
//     A template which can supply types implementing
// ComponentTemplateMixin :
//     A template which produces a component template based off of an input
//     runtime


// Questions to answer :
//  - Could we compile something that accomplishes this trait? a.k.a. Is it implemented?
//  - Is this trait compiled and available to use in some context? a.k.a. Is it required?
//  - Is this trait actually available to use in this specific context? a.k.a. Is it provided?



// Component member types :
//
// nonlocal :
//     A TypeSet of Member<X,Y> types, with X being a memory space
//     type, and Y being a type that should be held within that memory space.
//     This serves as a way of defining members that are "static" within
//     the lifetime of the owning runtime, while also specifying their
//     memory space. In cases where memory spaces limit mutual accesibility
//     between different execution contexts (e.g. private memory, thread-local
//     memory, and shared memory), exactly one instance is created for each
//     independent region of mutual accessibility.
//
// requires_traits :
//     A TypeSet of traits required by the component. If no such type is
//     provided, it is assumed that the component may always be included in
//     any runtime.
//
// implements_traits :
//     A TypeSet of traits directly implemented by the component. If no such
//     type is provided, it is assumed that the component does not
//     automatically implement any trait simply by being included in a runtime
//     that meets all of its requirements.
//
// reifies_traits :
//     A TypeSet of traits directly reified by the component. If no such type
//     is provided, it is assumed that the component does not reify any trait.
//
// uses_traits :
//     A TypeSet of traits directly used by the component, and hence must be
//     reified directly or by an outter runtime. If no such type is provided,
//     it is assumed that the component does
//
// components :
//     A template that maps trait types to components that implement the
//     given interface. Traits implemented within the "Components" template
//     of a runtime's component also count as implementations for the parent
//     runtime. This allows arbitrary sets of implementations to be provided
//     to runtimes in response to specific conditions.
//



int main() {

    Runtime<Meta<Even>,Meta<Odd>> dummy;
    get<Even>(dummy)(16,16,0);

    return 0;
}


#if BLAH


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

#endif




