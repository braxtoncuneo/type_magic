#include "../../include/include.h"

#include <fstream>


// Defines a transform which enforces a mutual exclusion between
// components that implement TRAIT. This transform iterates through the set
// of all components implementing TRAIT, treating each as a candidate for
// implementing TRAIT. If all requirements can be satisfied with all non-candidate
// implementors of the trait removed, all non-candidate implementors are removed from
// the dependency graph.
template<typename TRAIT>
struct TraitImplMutex {

    // Per-mutex key types used to reference the current candidate,
    // previously-checked candidates, and unchecked candidates.
    struct Candidate{};
    struct Checked{};
    struct Unchecked{};


    // Used to find implementations of TRAIT that are not CANDIDATE
    template<typename TRAIT_MAP, typename CANDIDATE>
    struct NotCandidateAndSelected {
        template <typename IMPL>
        struct Filter {
            static constexpr bool implements_trait = TRAIT_MAP::template ItemAt<TRAIT>::type::template has_item<IMPL>();
            static constexpr bool value = implements_trait && ! std::is_same<IMPL,CANDIDATE>::value;
        };
    };

    template<typename STATE>
    struct Sanitize {
        typedef typename STATE
            ::template RemoveItem<Candidate>::type
            ::template RemoveItem<Checked>::type
            ::template RemoveItem<Unchecked>::type
            type;
    };


    // Checks each candidate implementation to see if it results in a
    // context with all requirements satisfied.
    template<typename STATE,typename ENABLE=void>
    struct SolutionSearch;
   
  
    // Checks the current candidate implementation to see if it results in
    // a context with all requirements satisfied. This check is
    // accomplished by evaluating the rest of the transform with all other
    // implementors excluded from the base trait/impl map, then checking
    // if the resulting context type is marked as satisfied.
    //
    // If a candidate succeeds, the modified state is returned. Otherwise,
    // an additional SolutionSearch is queued, the next implementor is set
    // as the candidate, and this fallback state is returned.
    template<typename STATE>
    struct SolutionSearch <
        STATE,
        typename std::enable_if<STATE::template ItemAt<Unchecked>::type::ITEM_COUNT != 0>::type
    > {
        // Fetch relevant fields from the state
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;
        typedef typename STATE::template ItemAt<Candidate>::type CandidateImpl;
        typedef typename STATE::template ItemAt<Checked>::type   CheckedImplSet;
        typedef typename STATE::template ItemAt<Unchecked>::type UncheckedImplSet;
        
        // Find the set of all implementros that are not the candidate
        typedef typename ImplMap::KeySet
                ::template Filter<NotCandidateAndSelected<TraitMap,CandidateImpl>::template Filter>::type
                ImplsToRemove;

        // Excluded all non-candidate implementors from every item in the trait map
        typedef typename TraitMap
                ::template MapItems<container::util::type_set::Exclude<ImplsToRemove>::template Template>::type
                UpdatedTraitMap;

        // Exclude all non-candidate implementors from the impl map
        typedef typename ImplMap
                ::template FilterKeys<container::util::Negate<ImplsToRemove::template HasItem>::template Template>::type
                UpdatedImplMap;

        // Create an updated state with the candidate as the sole implementor
        typedef typename Sanitize<STATE>::type
                ::template UpdateItem<context::key::TraitMap,UpdatedTraitMap>::type
                ::template UpdateItem<context::key::ImplMap,UpdatedImplMap>::type
                ::template SetItem<context::key::unsat::FailEarly,AlwaysTrue<void>>::type
                CandidateState;

        typedef typename Sanitize<CandidateState>::type
                ::template SetItem<context::key::unsat::FailEarly,AlwaysFalse<void>>::type
                CandidateReturnState;

        // Evaluate the rest of the transform, and retrieve its diagnostic information
        typedef typename context::EvalTransform<CandidateState>::type
                ::template ItemAt<context::key::CheckInfo>::type
                CandidateInfo;
    
        static constexpr bool SATISFIED = CandidateInfo::ALL_REQS_SATISFIED;

        // In case the current candidate does not work, create a fallback state
        // with a new candidate.
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;
        typedef typename TformQueue::template PushFront<Meta<SolutionSearch>>::type FallbackTformQueue;
    
        typedef typename UncheckedImplSet::MapType::HeadKeyType      FallbackCandidate;
        typedef typename UncheckedImplSet::MapType::TailType::KeySet FallbackUnchecked;
        typedef typename CheckedImplSet::template Union<container::TypeSet<CandidateImpl>>::type FallbackChecked;

        typedef typename STATE
                ::template SetItem<Candidate,FallbackCandidate>::type
                ::template SetItem<Unchecked,FallbackUnchecked>::type
                ::template SetItem<Checked,FallbackChecked>::type
                ::template SetItem<context::key::TransformQueue,FallbackTformQueue>::type
                FallbackState;

        // Gate between the candidate and fallback state by whether or not the
        // candidate succeeded.
        typedef typename container::TypeArray<FallbackState,CandidateReturnState>
                ::template ItemAt<SATISFIED>::type
                type;
    
    };
    
    
    // If no additional alternative candidates are known, the remaining implementor is set
    // as the candidate, all other implementors are removed from the trait/impl maps.
    template<typename STATE>
    struct SolutionSearch <
        STATE,
        typename std::enable_if<STATE::template ItemAt<Unchecked>::type::ITEM_COUNT == 0>::type
    > {
        // Retrieve relevant fields from the state
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;
        typedef typename STATE::template ItemAt<Candidate>::type CandidateImpl;
        typedef typename STATE::template ItemAt<Checked>::type   CheckedImplSet;
        typedef typename STATE::template ItemAt<Unchecked>::type UncheckedImplSet;
       
        // Find the set of all non-candidate implementors 
        typedef typename ImplMap::KeySet
                ::template Filter<NotCandidateAndSelected<TraitMap,CandidateImpl>::template Filter>::type
                ImplsToRemove;

        // Remove all non-candidate implementors from all items in the trait map
        typedef typename TraitMap
                ::template MapItems<container::util::type_set::Exclude<ImplsToRemove>::template Template>::type
                UpdatedTraitMap;

        // Remove all non-candidate implementors from the impl map
        typedef typename ImplMap
                ::template FilterKeys<container::util::Negate<ImplsToRemove::template HasItem>::template Template>::type
                UpdatedImplMap;

        // Return the new state unconditionally, as it is the last resort.
        typedef typename Sanitize<STATE>::type
                ::template UpdateItem<context::key::TraitMap,UpdatedTraitMap>::type
                ::template UpdateItem<context::key::ImplMap,UpdatedImplMap>::type
                type;
    };
    

    // Sets up and evaluates the SolutionSearch
    template<typename STATE>
    struct Transform {

        typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;

        // Find the set of all components implementing TRAIT
        typedef typename TraitMap::template ItemAt<TRAIT>::type MutexSet;

        // Add SolutionSearch to the transform queue
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;
        typedef typename TformQueue::template PushFront<Meta<SolutionSearch>>::type UpdatedTformQueue;
        
        // Initialize defaults for fields relevant to SolutionSearch
        typedef typename STATE
                ::template SetItem<Candidate,typename MutexSet::MapType::HeadKeyType>::type 
                ::template SetItem<Checked,container::TypeSet<>>::type
                ::template SetItem<Unchecked,typename MutexSet::MapType::TailType::KeySet>::type
                ::template SetItem<context::key::TransformQueue,UpdatedTformQueue>::type
                type;
        
    };

};



#include <iostream>

// Two traits corresponding to log behaviour
struct Log : TraitImplMutex<Log> {};
struct FileLog{};
struct PrintLog{};
struct LogStyle : TraitImplMutex<LogStyle> {};


// Log text to a file
template<typename CONTEXT>
struct FileLogImpl {
    
    std::ofstream log_file;

    FileLogImpl() : log_file("out.log") {}

    void log(std::string text) {
        log_file << via<LogStyle>(this)(text) << std::endl;
    }
};

// Log text to standard out 
template<typename CONTEXT>
struct PrintLogImpl {
    void log(std::string text) {
        std::cout << via<LogStyle>(this)(text) << std::endl;
    }
};

using FileLogModule = context::SimpleModule <
    Meta<FileLogImpl>,
    context::RequirementSet<LogStyle>,
    context::ImplementationSet<Log,FileLog>
>;

using PrintLogModule = context::SimpleModule <
    Meta<PrintLogImpl>,
    context::RequirementSet<LogStyle>,
    context::ImplementationSet<Log,PrintLog>
>;



struct StandardLogStyle {
    std::string operator()(std::string text) {
        return text;
    } 
};

struct ColoredLogStyle {
    std::string operator()(std::string text) {
        return std::string("\033[33m\033[1m") + text + "\033[0m";
    }
};

using StandardLogStyleModule = context::SimpleModule <
    StandardLogStyle,
    context::RequirementSet<Log>,
    context::ImplementationSet<LogStyle,StandardLogStyle>
>;

using ColoredLogStyleModule = context::SimpleModule <
    ColoredLogStyle,
    context::RequirementSet<PrintLog,Log>,
    context::ImplementationSet<LogStyle,ColoredLogStyle>
>;





using RootModule = context::ModuleBundle<
    FileLogModule,
    PrintLogModule,
    StandardLogStyleModule,
    ColoredLogStyleModule
>;



template<typename CTX>
void run() {
 
    if constexpr (CTX::Info::SATISFIED) {
        
        CTX ctx;

        std::string log_name       = container::repr::type_name<CTX>();
        std::string log_style_name = container::repr::type_name<As<LogStyle,CTX>>();

        as<Log>(ctx).log(log_name + " " + log_style_name);

    } else {
        CTX ctx;
        std::cout << as<context::ContextInfo>(ctx).error_string();
    }

}

int main() {

    using namespace container;   
    using namespace context;   

    typedef TypeMap<Binding<key::RootModule,RootModule>> BaseInputState;

    typedef typename BaseInputState
            ::template SetItem<key::RequirementSet,TypeSet<FileLog,StandardLogStyle>>::type
            StandardFile;

    typedef typename BaseInputState
            ::template SetItem<key::RequirementSet,TypeSet<FileLog,ColoredLogStyle>>::type
            ColoredFile;

    typedef typename BaseInputState
            ::template SetItem<key::RequirementSet,TypeSet<PrintLog,StandardLogStyle>>::type
            StandardPrint;
    typedef typename BaseInputState
            ::template SetItem<key::RequirementSet,TypeSet<PrintLog,ColoredLogStyle>>::type
            ColoredPrint;

    
    run<typename context::CreateContextType<StandardFile>::type>();
    run<typename context::CreateContextType<ColoredFile>::type>();
    run<typename context::CreateContextType<StandardPrint>::type>();
    run<typename context::CreateContextType<ColoredPrint>::type>();

    return 0;
}




