#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cxxabi.h>
#include <fstream>
#include <sstream>
#include "preamble.h"

#include "sanity_check.cpp"

#include "config.h"

#include "container/mod.h"
#include "context/mod.h"
#include "core/mod.h"

#include "postamble.h"

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

    typedef context::SimpleModule <
        Meta<StaticSaveInfoComponent>,
        context::RequirementSet<SerDeInfo<T>>,
        context::ImplementationSet<StaticSaveInfo<T>>
    > Module;

};


typedef context::MetaModule<
    StaticSaveInfo,
    StaticSaveInfoMeta
> StaticSaveInfoModule;


template <typename T, typename ENABLE=void>
struct SerDeComponent {};

template <>
struct SerDeComponent <std::string>
{
    struct Component
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

    typedef context::SimpleModule <
        Component,
        context::ImplementationSet<SerDeInfo<std::string>>
    > Module;

};


template <typename T>
struct SerDeComponent <T,typename std::enable_if<std::is_arithmetic<T>::value>::type>
{
    struct Component
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

    typedef context::SimpleModule <
        Component,
        context::ImplementationSet<SerDeInfo<T>>
    > Module;

};


template <typename T>
struct SerDeComponent <std::vector<T>>
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

    typedef context::SimpleModule <
        Meta<Component>,
        context::RequirementSet<SerDeInfo<T>>,
        context::ImplementationSet<SerDeInfo<std::vector<T>>>
    > Module;

};


typedef context::MetaModule<
    SerDeInfo,
    SerDeComponent
> SerDeModule;

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
