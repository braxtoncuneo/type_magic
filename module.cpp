#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cxxabi.h>
#include <fstream>
#include <sstream>
#include "preamble.hpp"

#include "sanity_check.cpp"

#include "config.h"

#include "container/mod.h"
#include "context/mod.h"
#include "core/mod.h"

#include "postamble.h"

// Traits
struct isA
{
};
struct isB
{
};

struct isPrimeFinder
{
};

struct isLogger
{
};

template <typename T>
struct StaticSaveInfo
{
};

template <typename T>
struct SerDeInfo
{
};

/*
template <typename CONTEXT>
struct A
{
    int x;
    double &get_y()
    {
        return via<isB>(this).y;
    }
};

using aModule = context::SimpleModule<
    Meta<A>,
    context::RequirementSet<>,
    context::ImplementationSet<isA>>;
*/

template <typename T>
struct StaticSaveInfoMeta
{
    template <typename CONTEXT>
    struct StaticSaveInfoComponent
    {
        T field;
        std::fstream saveFile;

        StaticSaveInfoComponent() : saveFile()
        {
        }
        void save()
        {
            saveFile.open(typeid(T).name(), std::ios::trunc | std::ios::out);
            std::string saveInfo = via<SerDeInfo<T>>(this).toString(field);
            saveFile << saveInfo << '\n';
            saveFile.close();
        }
        T load()
        {
            // seekg(0)
            // loop to fill primes vector
            // erase the whole file
            // save a variable for looping
            saveFile.open(typeid(T).name());
            std::string loadString = "";
            std::string value;
            while (saveFile >> value)
            {
                loadString += value + " ";
            }
            field = via<SerDeInfo<T>>(this).fromString(loadString);
            saveFile.close();
            return field;
        }
    };

    typedef Meta<StaticSaveInfoComponent> Component;
    typedef container::TypeSet <SerDeInfo<T>> Requirements;

};


typedef context::MetaModule<
    StaticSaveInfo,
    StaticSaveInfoMeta
> StaticSaveInfoModule;


struct SerDeComponent
{
    struct GivenText
    {
        std::string toString(std::string field)
        {
            return field;
        }
        std::string fromString(std::string str)
        {
            return str;
        }
    };
    template <typename T>
    struct GivenArithmetic
    {
        std::string toString(T field)
        {
            std::stringstream ss;
            ss << field;
            return ss.str();
        }
        T fromString(std::string str)
        {
            std::stringstream ss(str);
            T field;
            ss >> field;
            return field;
        }
    };
    template <typename T>
    struct GivenVector
    {
        template <typename CONTEXT>
        struct Component
        {
            std::string toString(std::vector<T> field)
            {
                std::stringstream ss;
                for (T item : field)
                {
                    std::string convertedItem = via<SerDeInfo<T>>(this).toString(item);
                    ss << item << " ";
                }
                return ss.str();
            }
            std::vector<T> fromString(std::string str)
            {
                std::stringstream ss(str);
                std::vector<T> field;
                T item;
                while (ss >> item)
                {
                    field.push_back(item);
                }
                return field;
            }
        };
    };
};

template <typename TRAIT, typename T2 = void>
struct SerDeModuleHelper
{
    typedef container::TypeMap<> type;
};

template <>
struct SerDeModuleHelper<SerDeInfo<std::string>>
{
    typedef container::TypeMap<
        container::Binding<
            SerDeComponent::GivenText,
            container::TypeSet<>
        >
    > type;
};
template <typename T>
struct SerDeModuleHelper<SerDeInfo<T>, typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
    typedef container::TypeMap<
        container::Binding<
            SerDeComponent::template GivenArithmetic<T>,
            container::TypeSet<>
        >
    > type;
};

template <typename T>
struct SerDeModuleHelper<SerDeInfo<std::vector<T>>>
{
    typedef container::TypeMap<
        container::Binding<
            Meta<SerDeComponent::template GivenVector<T>::template Component>,
            container::TypeSet<SerDeInfo<T>>
        >
    > type;
};

struct SerDeModule
{
    template <typename TRAIT>
    struct ImplFor
    {
        typedef typename SerDeModuleHelper<TRAIT>::type type;
    };
};

template <typename CONTEXT>
struct PrimeFinder
{

    std::vector<int> findPrimes(int min, int max)
    {
        std::vector<int> primes;
        if (via<isLogger>(this).returnFile.is_open())
        {
            int value;
            while (via<isLogger>(this).returnFile >> value)
            {
                primes.push_back(value);
            }
        }
        for (int i = min; i < max; i++)
        {
            bool is_prime = true;
            for (int j = 2; j < i; j++)
            {
                if (i % j == 0)
                {
                    is_prime = false;
                    break;
                }
            }
            if (is_prime)
            {
                primes.push_back(i);
                via<isLogger>(this) << i << '\n';
            }
        }
        via<StaticSaveInfo<std::vector<int>>>(this).field = primes;
        via<StaticSaveInfo<std::vector<int>>>(this).save();
        
        return primes;
    }
};

using primeFinderModule = context::SimpleModule<
    Meta<PrimeFinder>,
    context::RequirementSet<isLogger,StaticSaveInfo<std::vector<int>>>,
    context::ImplementationSet<isPrimeFinder>>;

template <typename CONTEXT>
struct Logger
{
    std::fstream returnFile;
    Logger(std::string filePath) : returnFile(filePath, std::ios::in | std::ios::out | std::ios::app)
    {
    }
    Logger(Logger &&other) : returnFile(std::move(other.returnFile)) {}
    template <typename T>
    Logger &operator<<(T message)
    {
        returnFile << message;
        return (*this);
    }
    void toLog(std::string message)
    {
        returnFile << "Log " << message << '\n';
    }
};

using LoggerModule = context::SimpleModule<
    Meta<Logger>,
    context::RequirementSet<>,
    context::ImplementationSet<isLogger>>;
/*
template <typename CONTEXT>
struct B
{
    double y;
    int &get_x()
    {
        return via<isA>(this).x;
    }
};

using bModule = context::SimpleModule<
    Meta<B>,
    context::RequirementSet<>,
    context::ImplementationSet<isB>>;

typedef context::ModuleBundle<
    aModule,
    bModule>
    rootModule;
*/

typedef context::ModuleBundle<
    primeFinderModule,
    LoggerModule,
    StaticSaveInfoModule,
    SerDeModule
> practiceRootModule;

template <typename CTX>
void run()
{

    if constexpr (CTX::Info::SATISFIED)
    {

        CTX ctx(As<isLogger, CTX>{"log.txt"});

        int saveValue = 42;
        as<StaticSaveInfo<int>>(ctx).field = saveValue;
        as<StaticSaveInfo<int>>(ctx).save();
        as<isPrimeFinder>(ctx).findPrimes(1,100);
    }

    else
    {
        CTX ctx;
        std::cout << as<context::ContextInfo>(ctx).error_string();
    }
}
int main()
{

    typedef typename context::CreateContextType<
        practiceRootModule,
        container::TypeSet<isPrimeFinder, StaticSaveInfo<int>>,
        Meta<context::EagerSolve>>::type Ctx;

    run<Ctx>();

    return 0;
}
