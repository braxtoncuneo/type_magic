#include "../../include.h"

#include <fstream>


// Traits
struct Log{};
struct FindPrimes{};

template<typename CONTEXT>
struct PrimeFinder {
    int min;
    int max;
    PrimeFinder(int min,int max) : min(min), max(max) {}
    std::vector<int> find_primes() {
        std::vector<int> result;
        for (int i=min; i<=max; i++) {
            bool is_prime = true;
            for (int j=2; j<i; j++) {
                if (i%j==0) {
                    is_prime = false;
                }
            }
            if (is_prime) {
                result.push_back(i);
                via<Log>(this) << i << '\n';
            }
        }
        return result;
    }
};

using PrimeFindingModule = context::SimpleModule <
    Meta<PrimeFinder>,
    context::RequirementSet<Log>,
    context::ImplementationSet<FindPrimes>
>;


struct StdLog {

    std::ofstream log_file;

    StdLog(std::string path) : log_file(path) {}
    StdLog() : StdLog("log.txt") {}
    StdLog(StdLog &&) = default;

    template<typename T>
    StdLog& operator<<(T value) {
        log_file << value;
        return *this;
    }
};

using LogModule = context::SimpleModule <
    StdLog,
    context::RequirementSet<>,
    context::ImplementationSet<Log>
>;

typedef context::ModuleBundle<
    PrimeFindingModule,
    LogModule
> rootModule;



template<typename CTX>
void run() {
 
    if constexpr (CTX::Info::SATISFIED) {
        
        CTX ctx(
            As<FindPrimes,CTX>{1,100},
            As<Log,CTX>{"my_log.txt"}
        );

        as<FindPrimes>(ctx).find_primes();

    } else {
        CTX ctx;
        std::cout << as<context::ContextInfo>(ctx).error_string();
    }

}

int main() {
    
    typedef typename context::CreateContextType<
        rootModule,
        container::TypeSet<FindPrimes>,
        Meta<context::EagerSolve>
    >::type Ctx;

    run<Ctx>();

    return 0;
}




