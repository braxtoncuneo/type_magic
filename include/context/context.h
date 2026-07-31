#ifndef HARMONIZE_CONTEXT_CONTEXT
#define HARMONIZE_CONTEXT_CONTEXT

#include "../container/mod.h"
#include "key.h"
#include "module.h"
#include "demux.h"
#include "transform.h"


#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>

using container::init::init;

namespace context {


    // Bundles trait/impl mappings together for transform states
    template<typename TRAIT_MAP, typename IMPL_MAP>
    struct DependencyMap {
        typedef TRAIT_MAP TraitMap;
        typedef IMPL_MAP  ImplMap;
    };



    // Expands the base trait/impl mappings, beginning with the traits in
    // STATE[keys::RequirementSet], then following trait->implementation
    // edges and implementation->requirement edges.
    template<typename STATE,typename ENABLE=void>
    struct SearchRecurse {
        static_assert(
            container::IsTypeMap<STATE>::value,
            ASSERT_TEXT("ERROR: STATE parameter must be a TypeMap.")
        );
        static_assert(
            !container::IsTypeMap<STATE>::value,
            ASSERT_TEXT("INTERNAL ERROR: This specialization should never recieve a TypeMap as a parameter.")
        );
    };
    

    // Performs expansion in cases where the frontier of traits and implementations
    // is non-empty
    template<typename...ARGS>
    struct SearchRecurse <
        container::TypeMap<ARGS...>,
        typename std::enable_if<container::TypeMap<ARGS...>::template ItemAt<context::key::search::TraitFrontier>::type::MapType::ITEM_COUNT!=0>::type
    > {

        // Access relevant information in transform state
        typedef container::TypeMap<ARGS...> STATE;
        typedef typename STATE::template ItemAt<context::key::RootModule>::type     Root;
        typedef typename STATE::template ItemAt<context::key::search::TraitFrontier>::type Frontier;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type       TraitMap;
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type        ImplMap;
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;

        static_assert(
            container::IsTypeSet<Frontier>::value,
            ASSERT_TEXT("INTERNAL ERROR: Frontier field must be a TypeSet.")
        );
        static_assert(
            container::IsTypeMap<TraitMap>::value,
            ASSERT_TEXT("INTERNAL ERROR: TraitMap parameter must be a TypeMap.")
        );
        static_assert(
            container::IsTypeMap<ImplMap>::value,
            ASSERT_TEXT("INTERNAL ERROR: ImplMap parameter must be a TypeMap.")
        );
        
        typedef SearchRecurse<STATE> SelfType;

        template <typename TYPE>
        struct NotInOldTraits {
            static constexpr bool value = ! TraitMap::template has_key<TYPE>();
        };

        // Get the set of required traits that have not yet been added to the trait map
        typedef typename Frontier::template Filter<NotInOldTraits>::type UnQueriedTraits;

        // Get a mapping of unqueried traits to the implementation maps returned for those traits
        typedef typename UnQueriedTraits::MapType::template MapItems<Root::template ImplFor>::type NewDepMap;
       
        //Extract a mapping from all queried traits to their implementations
        typedef typename NewDepMap::template MapItems<container::util::type_map::KeySet>::type NewTraitMap;

        // Combine this mapping with the current trait map
        typedef typename TraitMap::template Combine<NewTraitMap>::type UpdatedTraitMap;

        // Extract a combination of all implementation maps returned from the queries
        typedef typename NewDepMap::template FoldItems<container::TypeMap<>,container::util::type_map::MapOfSetsCombine>::type NewImplMap;
        // Combine these mappings with the current implementation map
        typedef typename ImplMap::template LossyCombine<NewImplMap>::type UpdatedImplMap;

        // Extract the union of all trait requirements listed by the newly-found impl maps
        typedef typename NewImplMap::template FoldItems<container::TypeSet<>,container::util::type_set::BinaryUnion>::type NewImplMapReqs;
        // Filter down set of traits to those that are not already listed in the updated trait map
        typedef typename NewImplMapReqs::template Difference<typename UpdatedTraitMap::KeySet>::type NewReqSet;
   
        // Recursively define the fully-resolved mappings of traits to implementations and vice-versa
        typedef typename TformQueue::template PushFront<Meta<SearchRecurse>>::type UpdatedTformQueue;

        // Update state field for next iteration
        typedef typename STATE::template UpdateItem<context::key::search::TraitFrontier,NewReqSet>::type
                     ::template UpdateItem<context::key::TraitMap,UpdatedTraitMap>::type
                     ::template UpdateItem<context::key::ImplMap,UpdatedImplMap>::type
                     ::template UpdateItem<context::key::TransformQueue,UpdatedTformQueue>::type
                     type;

    };


    namespace search {
        template <typename IMPL, typename ENABLE=void>
        struct HasTransform {
            static constexpr bool value = false; 
        };

        template<typename IMPL>
        struct HasTransform <IMPL,typename AlwaysVoid<Meta<IMPL::template Transform>>::type> {
            static constexpr bool value = true;
        };

        template<typename IMPL>
        struct GetTransform {
            typedef Meta<IMPL::template Transform> type;
        };
    }


    // Handles the base case (empty frontier)
    template<typename... ARGS>
    struct SearchRecurse <
        container::TypeMap<ARGS...>,
        typename std::enable_if<container::TypeMap<ARGS...>::template ItemAt<context::key::search::TraitFrontier>::type::MapType::ITEM_COUNT==0>::type
    > {
        typedef container::TypeMap<ARGS...> State;
        typedef typename State::template ItemAt<context::key::RootModule>::type     Root;
        typedef typename State::template ItemAt<context::key::TraitMap>::type       TraitMap;
        typedef typename State::template ItemAt<context::key::ImplMap>::type        ImplMap;
        typedef typename State::template ItemAt<context::key::TransformQueue>::type TformQueue;

        typedef SearchRecurse<State> SelfType;
        
        typedef typename TraitMap::KeySet
                ::template Filter<search::HasTransform>::type
                ::template Map<search::GetTransform>::type
                ::MapType::KeyArray
                TraitTransforms;
        
        typedef typename ImplMap::KeySet
                ::template Filter<search::HasTransform>::type
                ::template Map<search::GetTransform>::type
                ::MapType::KeyArray
                ImplTransforms;
        
        typedef typename TraitTransforms
                ::template Concatenate<ImplTransforms>::type
                ::template Concatenate<TformQueue>::type
                UpdatedTformQueue; 
       
        typedef typename State
                ::template UpdateItem<context::key::TransformQueue,UpdatedTformQueue>::type
                type;

    };
    
    template<typename STATE>
    struct Search {

        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;
        typedef typename STATE::template ItemAt<context::key::RequirementSet>::type ReqSet;
        
        typedef typename TformQueue
                ::template PushFront<Meta<SearchRecurse>>::type
                UpdatedTformQueue; 
       
        typedef typename STATE
                ::template SetItem<context::key::search::TraitFrontier,ReqSet>::type
                ::template UpdateItem<context::key::TransformQueue,UpdatedTformQueue>::type
                type;

    };

    
    // Filters for the Prune transform
    namespace prune {

        // Returns true if the provided IMPL_MAP contains the TRAIT supplied to
        // the inner template
        template <typename IMPL_MAP>
        struct ImplementsTrait {
            template <typename TRAIT>
            struct Selector {
                static constexpr bool value = IMPL_MAP::template has_key<TRAIT>();
            };
        };

        // Returns true if the provided trait has implementatiosn in the 
        template <typename IMPL_MAP>
        struct TraitHasImplementations {
            template <typename IMPL_SET>
            struct Selector {
                static constexpr bool value = IMPL_SET::template Filter<ImplementsTrait<IMPL_MAP>::template Selector>::type::MapType::ITEM_COUNT > 0;
            };
        };

        // Returns true if the provided TRAIT_MAP does not contain the provided TRAIT
        template<typename TRAIT_MAP>
        struct TraitMissing {
            template <typename TRAIT>
            struct Selector {
                static constexpr bool value = ! TRAIT_MAP::template has_key<TRAIT>();
            };

        };

        // Returns true if the provided TRAIT_MAP includes all elements of REQ_SET
        // as keys
        template <typename TRAIT_MAP>
        struct ImplementationHasTraits {
            template <typename REQ_SET>
            struct Selector {
                static constexpr bool value = REQ_SET::template Filter<TraitMissing<TRAIT_MAP>::template Selector>::type::MapType::ITEM_COUNT == 0;
            };
        };
    }

    namespace status { 
        struct SimpleFailStatus {
            static constexpr bool ALL_REQS_SATISFIED = false;
        };

        struct SimpleSuccessStatus {
            static constexpr bool ALL_REQS_SATISFIED = true;
        };
    
        struct EarlyFailContext {};
    }

    // Removes:
    //  - all traits from STATE[key::TraitMap] which have no implementations
    //    included as a key in STATE[key::ImplMap]
    //  - all implementations from STATE[key::ImplMap] which have at least one trait
    //    not included as a key in STATE[key::TraitMap]
    //
    // Queues a repeat of the transform unless no removals occurred in the
    // current iteration
    template <typename STATE>
    struct PruneRecurse {

        // Retrieve the trait map an impl map from STATE
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type OriginalTraitMap;
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type OriginalImplMap;
        typedef typename STATE::template ItemAt<context::key::RequirementSet>::type  ReqSet;

        typedef typename STATE
                ::template SetItem<context::key::Status,status::SimpleFailStatus>::type
                ::template SetItem<context::key::ContextType,status::EarlyFailContext>::type
                ::template SetItem<context::key::TransformQueue,container::TypeArray<>>::type
                EarlyFailState;

        // Perform removals via filter operations
        typedef typename OriginalTraitMap::template FilterItems<
                prune::TraitHasImplementations<OriginalImplMap>::template Selector
            >::type UpdatedTraitMap;
        typedef typename OriginalImplMap::template FilterItems<
                prune::ImplementationHasTraits<OriginalTraitMap>::template Selector
            >::type UpdatedImplMap;

        typedef typename OriginalTraitMap::template FilterItems<
                container::util::Negate<prune::TraitHasImplementations<OriginalImplMap>::template Selector>::template Template
            >::type::KeySet UnsatTraits;
        static constexpr bool UNSATISFIED       = UnsatTraits::template Intersection<ReqSet>::type::MapType::ITEM_COUNT != 0;
        static constexpr bool SHOULD_FAIL_EARLY = STATE::template ItemAt<context::key::unsat::FailEarly>::type::value;

        // Determine if any changes occured in the trait/impl map
        static constexpr bool NO_TRAIT_CHANGE = std::is_same<OriginalTraitMap,UpdatedTraitMap>::value;
        static constexpr bool NO_IMPL_CHANGE  = std::is_same<OriginalImplMap,UpdatedImplMap>::value;
        static constexpr bool NO_CHANGE = NO_TRAIT_CHANGE && NO_IMPL_CHANGE;


        // Insert a repeat of the transform if any change occured
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;
        typedef typename container::TypeArray <
                typename TformQueue::template PushFront<Meta<PruneRecurse>>::type,
                TformQueue
            >::template ItemAt<NO_CHANGE>::type
            UpdatedTformQueue;

        // Update the trait/impl maps and the transform queue
        typedef typename STATE
                ::template UpdateItem<context::key::TraitMap,UpdatedTraitMap>::type
                ::template UpdateItem<context::key::ImplMap,UpdatedImplMap>::type
                ::template UpdateItem<context::key::TransformQueue,UpdatedTformQueue>::type
                AnticipatedReturnState;


        typedef typename container::TypeArray<
                AnticipatedReturnState,
                EarlyFailState
            >::template ItemAt<UNSATISFIED && SHOULD_FAIL_EARLY>::type
            type;
    };

    // Saves the original, unpruned versions of the trait/impl maps before pruning,
    // then queues up PruneRecurse to begin pruning
    template <typename STATE>
    struct Prune {

        // Retrieve relevant fields from STATE
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type UnprunedTraitMap;
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type UnprunedImplMap;
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;
       
        // Bundle together the original trait/impl 
        typedef context::DependencyMap<UnprunedTraitMap,UnprunedImplMap> UnprunedMap;
        
        // Create an updated transform queue beginning with PruneRecurse
        typedef typename TformQueue::template PushFront<Meta<PruneRecurse>>::type UpdatedTformQueue;

        // Update and return STATE
        typedef typename STATE
                ::template DefaultItem<context::key::unsat::FailEarly,AlwaysFalse<void>>::type
                ::template SetItem<context::key::UnprunedMap,UnprunedMap>::type
                ::template UpdateItem<context::key::TransformQueue,UpdatedTformQueue>::type
                type;
    };


    // Recursively searches for unsatisfiable traits/impls that are reacheable
    // from context requirements through a path consisting of only such 
    // unsatisfiable traits/impls.
    template<typename STATE>
    struct UnsatRecurse {

        // Retrieve relevant fields from STATE
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type        TraitMap;
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type         ImplMap;
        typedef typename STATE::template ItemAt<context::key::UnprunedMap>::type     Unpruned;
        typedef typename STATE::template ItemAt<key::unsat::Traits>::type            UnsatTraits;
        typedef typename STATE::template ItemAt<key::unsat::Impls>::type             UnsatImpls;
        typedef typename STATE::template ItemAt<key::unsat::ReqTraitFrontier>::type  FrontierTraitSet;
        typedef typename STATE::template ItemAt<key::unsat::ReqImplFrontier>::type   FrontierImplSet;
        typedef typename STATE::template ItemAt<key::unsat::ReqTraits>::type         TraitSet;
        typedef typename STATE::template ItemAt<key::unsat::ReqImpls>::type          ImplSet;
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type  TformQueue;

        // Get mapping of all impls referenced in the trait frontier
        typedef typename FrontierTraitSet::template Map<Unpruned::TraitMap::template ItemAt>::type TraitFrontierImplSets;
        // Convert the aformentioned mapping into a set
        typedef typename Meta<container::TypeSet<>::template Union>::SpecializeFromTypeSet<TraitFrontierImplSets>::type::type UnfilteredImplFrontier;
        // Filter down the set down to unreached impls
        typedef typename UnfilteredImplFrontier::template Intersection<UnsatImpls>::type::template Difference<ImplSet>::type UpdatedImplFrontier;
        
        // Get mapping of all traits referenced in the impl frontier
        typedef typename FrontierImplSet::template Map<Unpruned::ImplMap::template ItemAt>::type ImplFrontierTraitSets;
        // Convert the aformentioned mapping into a set
        typedef typename Meta<container::TypeSet<>::template Union>::template SpecializeFromTypeSet<ImplFrontierTraitSets>::type::type UnfilteredTraitFrontier;
        // Filter down the set down to unreached traits
        typedef typename UnfilteredTraitFrontier::template Intersection<UnsatTraits>::type::template Difference<TraitSet>::type UpdatedTraitFrontier; 

        // Update set of visited traits/impls
        typedef typename TraitSet::template Union<FrontierTraitSet>::type UpdatedTraitSet;
        typedef typename ImplSet::template Union<FrontierImplSet> ::type UpdatedImplSet;

        // Determine if there is still more of the graph to explore
        static constexpr bool EMPTY_TRAIT_FRONTIER = std::is_same<container::TypeSet<>,UpdatedTraitFrontier>::value;
        static constexpr bool EMPTY_IMPL_FRONTIER  = std::is_same<container::TypeSet<>,UpdatedImplFrontier>::value;
        static constexpr bool EMPTY_FRONTIER       = EMPTY_TRAIT_FRONTIER && EMPTY_IMPL_FRONTIER;

        // Insert a new UnsatRecurse iteration into the front of the transform queue
        // if the frontier is non-empty
        typedef typename container::TypeArray<
                typename TformQueue::template PushFront<Meta<UnsatRecurse>>::type,
                TformQueue
            >::template ItemAt<EMPTY_FRONTIER>::type UpdatedTformQueue;


        // Update and return the transform state
        typedef typename STATE
                ::template UpdateItem<key::unsat::ReqTraitFrontier,UpdatedTraitFrontier>::type
                ::template UpdateItem<key::unsat::ReqImplFrontier, UpdatedImplFrontier>::type
                ::template UpdateItem<key::unsat::ReqTraits,       UpdatedTraitSet>::type
                ::template UpdateItem<key::unsat::ReqImpls,        UpdatedImplSet>::type
                ::template UpdateItem<context::key::TransformQueue,UpdatedTformQueue>::type
                type;



    };


    namespace info {
        struct SimpleFailInfo {
            static constexpr bool SATISFIED = false;
        };
    }

    // Trait representing the context reflection information provided to contexts
    struct ContextInfo {};

    // The standard implementation of ContextInfo
    template<typename STATE>
    struct ContextInfoImpl {

        typedef typename STATE::template ItemAt<context::key::Sequence>::type Sequence;
        typedef typename STATE::template ItemAt<context::key::RootModule>::type Root;
        typedef typename STATE::template ItemAt<context::key::Status>::type Status;
        
        template<typename CONTEXT>
        struct Impl {
            typedef Root RootModule;
            static constexpr bool SATISFIED = Status::ALL_REQS_SATISFIED;

            static std::string error_string() {
                return Status::unsat_diagnostic_string();
            }
            static std::string solve_sequence_string() {
                return container::repr::StringRepr<Sequence>::repr();
            }
        };
    };

    template<typename STATE>
    struct CullFinalize {
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type        TraitMap;
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type         ImplMap;
        typedef typename STATE::template ItemAt<context::key::cull::ReqTraits>::type ReqTraits;
        typedef typename STATE::template ItemAt<context::key::cull::ReqImpls>::type  ReqImpls;

        typedef typename TraitMap::template FilterKeys<ReqTraits::template HasItem>::type UpdatedTraitMap;
        typedef typename ImplMap ::template FilterKeys<ReqImpls ::template HasItem>::type UpdatedImplMap;

        typedef typename STATE
                ::template SetItem<context::key::TraitMap,UpdatedTraitMap>::type
                ::template SetItem<context::key::ImplMap, UpdatedImplMap>::type
                ::template RemoveItem<context::key::cull::ReqTraits>::type
                ::template RemoveItem<context::key::cull::ReqImpls>::type
                ::template RemoveItem<context::key::cull::ReqTraitFrontier>::type
                ::template RemoveItem<context::key::cull::ReqImplFrontier>::type
                type;
    };


    template<typename STATE>
    struct CullRecurse {

        // Get relevant fields from STATE
        typedef typename STATE::template ItemAt<context::key::RequirementSet>::type  ReqSet;
        typedef typename STATE::template ItemAt<context::key::UnculledMap>::type     UnculledMap;
        typedef typename STATE::template ItemAt<context::key::cull::ReqTraits>::type ReqTraits;
        typedef typename STATE::template ItemAt<context::key::cull::ReqImpls>::type  ReqImpls;
        typedef typename STATE::template ItemAt<context::key::cull::ReqTraitFrontier>::type ReqTraitFrontier;
        typedef typename STATE::template ItemAt<context::key::cull::ReqImplFrontier>::type  ReqImplFrontier;

        // Add current frontier to the set of known traits
        typedef typename ReqTraits::template Union<ReqTraitFrontier>::type UpdatedReqTraits;
        typedef typename ReqImpls ::template Union<ReqImplFrontier> ::type UpdatedReqImpls;

        // Create filters to find the new frontier
        typedef typename container::util::Negate<UpdatedReqTraits::MapType::template HasKey> TraitIsNotKnown; 
        typedef typename container::util::Negate<UpdatedReqImpls ::MapType::template HasKey> ImplIsNotKnown; 
        
        // Find next frontiers
        typedef typename ReqTraitFrontier
                ::template Map<UnculledMap::TraitMap::template ItemAt>::type // Lookup impls for each trait
                ::template Fold<container::TypeSet<>,container::util::type_set::BinaryUnion>::type // Combine into a new set
                ::template Filter<ImplIsNotKnown::template Template>::type // Filter out already-visited impls
                UpdatedReqImplFrontier;
        
        typedef typename ReqImplFrontier
                ::template Map<UnculledMap::ImplMap::template ItemAt>::type // Lookup traits for each impl
                ::template Fold<container::TypeSet<>,container::util::type_set::BinaryUnion>::type // Combine into a new set
                ::template Filter<TraitIsNotKnown::template Template>::type // Filter out already-visited traits
                UpdatedReqTraitFrontier;
              
        // Determine if anything necessary was unsatisfied
        static constexpr bool EMPTY_TRAIT_FRONTIER = UpdatedReqTraitFrontier::MapType::ITEM_COUNT == 0;
        static constexpr bool EMPTY_IMPL_FRONTIER  = UpdatedReqImplFrontier::MapType::ITEM_COUNT == 0;
        static constexpr bool EMPTY_FRONTIER = EMPTY_TRAIT_FRONTIER && EMPTY_IMPL_FRONTIER;

        // Insert an additional CullRecurse if the new frontier is non-empty
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type  TformQueue;
        typedef typename container::TypeArray <
                typename TformQueue::template PushFront<Meta<CullRecurse>>::type,
                typename TformQueue::template PushFront<Meta<CullFinalize>>::type
            >::template ItemAt<EMPTY_FRONTIER>::type
            UpdatedTformQueue;

        // Return updated state
        typedef typename STATE
                ::template SetItem<context::key::TransformQueue,UpdatedTformQueue>::type
                ::template SetItem<context::key::cull::ReqTraits,UpdatedReqTraits>::type
                ::template SetItem<context::key::cull::ReqImpls, UpdatedReqImpls>::type
                ::template SetItem<context::key::cull::ReqTraitFrontier,UpdatedReqTraitFrontier>::type
                ::template SetItem<context::key::cull::ReqImplFrontier, UpdatedReqImplFrontier>::type
                type;

    };


    template<typename STATE>
    struct Cull {

        typedef typename STATE::template ItemAt<context::key::TraitMap>::type TraitMap;
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type  ImplMap;
        typedef typename STATE::template ItemAt<context::key::RequirementSet>::type
                ::template Filter<TraitMap::template HasKey>::type
                ReqSet;
        typedef DependencyMap<TraitMap,ImplMap> UnculledMap;

        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type  TformQueue;
        typedef typename TformQueue::template PushFront<Meta<CullRecurse>>::type     UpdatedTformQueue;

        typedef typename STATE
                ::template SetItem<context::key::UnculledMap,UnculledMap>::type
                ::template SetItem<context::key::TransformQueue,UpdatedTformQueue>::type
                ::template SetItem<context::key::cull::ReqTraits,container::TypeSet<>>::type
                ::template SetItem<context::key::cull::ReqImpls, container::TypeSet<>>::type
                ::template SetItem<context::key::cull::ReqTraitFrontier,ReqSet>::type
                ::template SetItem<context::key::cull::ReqImplFrontier, container::TypeSet<>>::type
                type;
    };



    // Sets up and evaluates satisfiability checks through UnsatRecurse
    //
    // Provides diagnostics to show unsatisfiable elements
    template<typename STATE>
    struct Check {
        
        // Get relevant fields from STATE
        typedef typename STATE::template ItemAt<context::key::RequirementSet>::type ReqSet;
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type       TraitMap;
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type        ImplMap;
        typedef typename STATE::template ItemAt<context::key::UnprunedMap>::type    UnprunedMap;
        typedef typename STATE::template ItemAt<context::key::TransformQueue>::type TformQueue;

        // Set up filters to find unsatisfied traits/impls
        typedef typename container::util::Negate<TraitMap::template HasKey> TraitIsNotSat; 
        typedef typename container::util::Negate<ImplMap ::template HasKey> CompIsNotSat; 
        
        // Find unsatisfied traits and implementations
        typedef typename UnprunedMap::TraitMap::template FilterKeys<TraitIsNotSat::template Template>::type UnsatTraitMap;
        typedef typename UnprunedMap::ImplMap::template  FilterKeys<CompIsNotSat ::template Template>::type  UnsatImplMap;

        // Find the set of unsatisfied requirements
        typedef typename ReqSet::template Intersection<typename UnsatTraitMap::KeySet>::type TopLevelUnsatReqs;
        
        // Set up the input for UnsatRecurse
        typedef typename STATE
                ::template SetItem<context::key::unsat::Traits,typename UnsatTraitMap::KeySet>::type
                ::template SetItem<context::key::unsat::Impls,typename UnsatImplMap::KeySet>::type
                ::template SetItem<context::key::unsat::ReqTraitFrontier,TopLevelUnsatReqs>::type
                ::template SetItem<context::key::unsat::ReqImplFrontier,container::TypeSet<>>::type
                ::template SetItem<context::key::unsat::ReqTraits,container::TypeSet<>>::type
                ::template SetItem<context::key::unsat::ReqImpls,container::TypeSet<>>::type
                ::template SetItem<context::key::TransformQueue,container::TypeArray<Meta<UnsatRecurse>>>::type
                UnsatStateInput;

        // Evaluate UnsatRecurse
        typedef typename EvalTransform<UnsatStateInput>::type UnsatState;
        
        // Get set of traits/impls that are both necessary an unsatisfied
        typedef typename UnsatState::template ItemAt<context::key::unsat::ReqTraits>::type ReqUnsatTraits;
        typedef typename UnsatState::template ItemAt<context::key::unsat::ReqImpls>::type  ReqUnsatImpls;
              
        // Determine if anything necessary was unsatisfied
        static constexpr bool SOME_TRAITS_UNSATISFIED = UnsatTraitMap::ITEM_COUNT > 0;
        static constexpr bool SOME_IMPLS_UNSATISFIED  = UnsatImplMap::ITEM_COUNT > 0;
        static constexpr bool ALL_REQS_SATISFIED   = (ReqUnsatTraits::MapType::ITEM_COUNT + ReqUnsatImpls::MapType::ITEM_COUNT) == 0;

        // Update and return the state
        typedef typename UnsatState
                ::template SetItem<context::key::Status,Check<STATE>>::type
                ::template SetItem<context::key::TransformQueue,TformQueue>::type
                type;
  
 
        // Lists the types in the provided TYPE_ARRAY
        template<typename TYPE_ARRAY>
        static std::string type_list_string() {
            // List is traversed recursively
            if constexpr (TYPE_ARRAY::MapType::ITEM_COUNT == 0) {
                return "[Nothing]";
            } else {
                typedef typename TYPE_ARRAY::template PopFront<>::type Tail;
                std::string result = container::repr::type_name<typename TYPE_ARRAY::template Front<>::type>();
                if constexpr (Tail::MapType::ITEM_COUNT != 0) {
                    result = result + ", " + type_list_string<Tail>();
                }
                return result; 
            }
        }

        // Indicates that UNSAT_TRAIT is not implemented, and lists implementation candidates
        template<typename UNSAT_TRAIT>
        static std::string unsat_trait_diagnostic() {
            std::string trait_name = container::repr::type_name<UNSAT_TRAIT>();
            std::string result = std::string("Trait '")+trait_name+"' is not implemented. Implementation candidates include: ";
            
            typedef typename UnprunedMap::TraitMap::template ItemAt<UNSAT_TRAIT>::type ImplSet;
            typedef typename ImplSet::template Intersection<typename UnsatImplMap::KeySet>::type::MapType::KeyArray UnsatCompArray;
            
            result += type_list_string<UnsatCompArray>();
            return result;
        }

        // Indicates that UNSAT_COMP does not have all requirements satisfied, and lists
        // the unsatisfied traits
        template<typename UNSAT_COMP>
        static std::string unsat_comp_diagnostic() {
            std::string comp_name = container::repr::type_name<UNSAT_COMP>();
            std::string result = std::string("Component '") + comp_name + "' requires traits that have not been implemented. The traits: ";

            typedef typename UnprunedMap::ImplMap::template ItemAt<UNSAT_COMP>::type TraitSet;
            typedef typename TraitSet::template Intersection<typename UnsatTraitMap::KeySet>::type::MapType::KeyArray UnsatTraitArray;
            result += type_list_string<UnsatTraitArray>();
            return result;
        }

        // Prints diagnostic messages for each unsatisfied trait 
        template<typename TYPE_ARRAY>
        static std::string unsat_trait_list_string() {
            if constexpr (TYPE_ARRAY::MapType::ITEM_COUNT == 0) {
                return "";
            } else {
                typedef typename TYPE_ARRAY::template Front   <>::type Front;
                typedef typename TYPE_ARRAY::template PopFront<>::type Tail;
                std::string result = unsat_trait_diagnostic<Front>()+ "\n";
                if constexpr (Tail::MapType::ITEM_COUNT != 0) {
                    result = result  + unsat_trait_list_string<Tail>();
                }
                return result; 
            }
        }

        // Prints diagnostic messages for each unsatisfied implementation
        template<typename TYPE_ARRAY>
        static std::string unsat_comp_list_string() {
            if constexpr (TYPE_ARRAY::MapType::ITEM_COUNT == 0) {
                return "";
            } else {
                typedef typename TYPE_ARRAY::template Front   <>::type Front;
                typedef typename TYPE_ARRAY::template PopFront<>::type Tail;
                std::string result = unsat_comp_diagnostic<Front>()+ "\n";
                if constexpr (Tail::MapType::ITEM_COUNT != 0) {
                    result = result  + unsat_comp_list_string<Tail>();
                }
                return result; 
            }
        }
        
        // Print diagnostic messages for each unsatisfied trait/impl 
        static std::string unsat_diagnostic_string() {
            typedef typename ReqUnsatTraits::MapType::KeyArray TraitArray;
            typedef typename ReqUnsatImpls::MapType::KeyArray ImplArray;
            return unsat_trait_list_string<TraitArray>() + unsat_comp_list_string<ImplArray>();
        }
        

    
    };



    template<typename... ARGS>
    struct Context;


    // Will eventually be used to allow contexts to contain other "parent"
    // contexts as components.
    template <typename...ARGS>
    struct BaseContext;

    template <typename...ARGS>
    struct BaseContext <Context<ARGS...>> {
        typedef Context<ARGS...> Type;
        Type& ref;
    };
    


    template<typename COMPONENT>
    struct ContextComponent : COMPONENT {
        ContextComponent() = default;

        ContextComponent(COMPONENT const& component) : COMPONENT{component} {}

        ContextComponent(COMPONENT&& component) : COMPONENT{std::move(component)} {}

        template<
            typename INIT,
            typename ENABLE=typename std::enable_if<::container::init::IsInit<typename std::decay<INIT>::type>::value>::type
        >
        explicit ContextComponent(INIT&& init)
            : ContextComponent(
                std::forward<INIT>(init),
                std::make_index_sequence<
                    std::tuple_size<typename std::decay<INIT>::type::Tuple>::value
                >{}
            )
        {}

    private:
        template<typename INIT, std::size_t... I>
        ContextComponent(INIT&& init, std::index_sequence<I...>)
            : COMPONENT{std::get<I>(std::forward<INIT>(init).args)...}
        {}
    };

    template<
        typename CONTEXT,
        typename ARG,
        bool IS_INIT=::container::init::IsInit<typename std::decay<ARG>::type>::value
    >
    struct ContextComponentForArg {
        typedef ContextComponent<typename std::decay<ARG>::type> type;
    };

    template<typename CONTEXT, typename ARG>
    struct ContextComponentForArg <CONTEXT,ARG,true> {
        typedef typename std::decay<ARG>::type InitType;
        typedef ContextComponent<typename CONTEXT::template ComponentLookup<typename InitType::Trait>> type;
    };



    template <typename TRAIT_MAP, typename... COMPONENTS>
    struct Context <TRAIT_MAP,COMPONENTS...> :
        ContextComponent<typename UnMeta<COMPONENTS,Context<TRAIT_MAP,COMPONENTS...>>::Type>...
    {
        
        typedef Context<TRAIT_MAP,COMPONENTS...> Self;  
        typedef TRAIT_MAP TraitMap;

        template <typename TRAIT>
        static constexpr bool implements_trait () {
            return TRAIT_MAP::template has_key<TRAIT>();
        }

        template<typename TRAIT>
        using ComponentLookup = typename UnMeta<typename TRAIT_MAP::template ItemAt<TRAIT>::type,Self>::Type;

        typedef ComponentLookup<ContextInfo> Info;
      
        template<typename TRAIT>
        ComponentLookup<TRAIT>& as() {
            if constexpr (TRAIT_MAP::template has_key<TRAIT>()) {
                typedef ComponentLookup<TRAIT> COMPONENT;
                if constexpr (Meta<BaseContext>::Generalizes<COMPONENT>::value) {
                    return (*static_cast<ComponentLookup<TRAIT>*>(this)).ref.template as<TRAIT>();
                } else {
                    return *static_cast<ComponentLookup<TRAIT>*>(this);
                }
            } else {
                throw 1;
            }
        }

        template<typename... ARGS>
        Context(ARGS&&... args)
            : ContextComponentForArg<Self,ARGS>::type(std::forward<ARGS>(args))...
        {}
        
    };


    template<typename TRAIT_MAP, typename COMPONENT_SET>
    struct ContextFromComponents;

    template<typename TRAIT_MAP, typename... COMPONENTS>
    struct ContextFromComponents <TRAIT_MAP,container::TypeSet<COMPONENTS...>> {
        typedef Context<TRAIT_MAP,COMPONENTS...> type;
    };
    

    template<typename STATE, typename ENABLE=void>
    struct Reify;
   
    template<typename STATE>
    struct Reify <
        STATE,
        typename std::enable_if<STATE::template ItemAt<context::key::Status>::type::ALL_REQS_SATISFIED>::type
    > {
        typedef typename STATE::template ItemAt<context::key::TraitMap>::type    UnculledTraitMap;

        typedef typename UnculledTraitMap::template MapItems<container::util::type_set::GetFirst>::type CulledTraitMap; 
        typedef typename CulledTraitMap::template SetItem<ContextInfo,info::SimpleFailInfo>::type TraitMap;

        typedef typename TraitMap::Invert::type::KeySet BaseComponentSet;
        typedef typename BaseComponentSet::template Union<container::TypeSet<info::SimpleFailInfo>>::type ComponentSet;

        typedef typename context::ContextFromComponents<TraitMap,ComponentSet>::type ContextType;

        typedef typename STATE::template SetItem<context::key::ContextType,ContextType>::type type;

    };

   
    template<typename STATE>
    struct Reify <
        STATE,
        typename std::enable_if<! STATE::template ItemAt<context::key::Status>::type::ALL_REQS_SATISFIED>::type
    > {
        typedef container::TypeMap<container::Binding<ContextInfo,info::SimpleFailInfo>> TraitMap;
        typedef container::TypeSet<info::SimpleFailInfo> ComponentSet;

        typedef typename context::ContextFromComponents<TraitMap,ComponentSet>::type ContextType;

        typedef typename STATE::template SetItem<context::key::ContextType,ContextType>::type type;
    };


    /*
    template<typename STATE>
    struct ComponentTransform {
        typedef typename STATE::template ItemAt<context::key::ImplMap>::type ImplMap;
        typedef typename ImplMap
                ::template FilterKeys<>::type
                ::KeySet
                ::template Map<>::type
                TformSet;
        
    };
    */


    template<typename STATE>
    struct CreateContextType {

        typedef container::TypeArray<
            Meta<Search>,
            // (user-provided transforms invoked here)
            Meta<Prune>,
            Meta<Cull>, 
            Meta<Check>,
            Meta<Multiplex>,
            Meta<Reify>
        > DefaultTformQueue;

        typedef typename STATE
                         ::template DefaultItem<context::key::TraitMap,container::TypeMap<>>::type
                         ::template DefaultItem<context::key::ImplMap,container::TypeMap<>>::type
                         ::template DefaultItem<context::key::TransformQueue,DefaultTformQueue>::type
                         ::template DefaultItem<key::Sequence,container::TypeArray<>>::type
                         InputState;

        typedef typename context::EvalTransform<InputState>::type State;

        #ifdef HARMONIZE_TRACK_SEQUENCE
        typedef typename State::template ItemAt<context::key::Sequence>::type  Sequence;
        #endif

        typedef typename State::template ItemAt<context::key::ContextType>::type RawContext;


        struct ShortState : State {};

        template<typename CONTEXT>
        struct ShortInfo : ContextInfoImpl<ShortState>::template Impl<CONTEXT> {};

        template<typename TYPE>
        struct CleanedContext;

        template<typename TRAIT_MAP, typename... COMPONENTS>
        struct CleanedContext <Context<TRAIT_MAP,COMPONENTS...>> {
            typedef Meta<ShortInfo> Info;
            typedef typename TRAIT_MAP::template SetItem<ContextInfo,Info>::type UpdatedTraitMap;
            typedef typename container::TypeSet<COMPONENTS...>::template Union<container::TypeSet<Info>>::type UpdatedCompSet;
            typedef typename ContextFromComponents<UpdatedTraitMap,UpdatedCompSet>::type type;
        };

        typedef typename CleanedContext<RawContext>::type type;

    };

}



namespace detector {

    template<typename TYPE>
    struct Detect {
        static constexpr bool IS_CONTEXT   = false;
        static constexpr bool IS_COMPONENT = false;
        static constexpr bool RECOGNIZED   = false;
    };

    template<
        typename TRAIT_MAP,
        typename...COMPONENTS
    >
    struct Detect <context::Context<TRAIT_MAP,COMPONENTS...>> {
        static constexpr bool IS_CONTEXT   = true;
        static constexpr bool IS_COMPONENT = false;
        static constexpr bool RECOGNIZED   = true;
    };

    template<
        template<typename> typename START_COMP,
        typename TRAIT_MAP,
        typename...COMPONENTS
    >
    struct Detect <START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>>> {
        static constexpr bool IS_CONTEXT   = false;
        static constexpr bool IS_MIXIN_COMPONENT = std::is_base_of<
                START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>>,
                context::Context<TRAIT_MAP,COMPONENTS...>
            >::value;
        static constexpr bool RECOGNIZED   = IS_MIXIN_COMPONENT;
    };

}

template<typename TRAIT,typename TYPE>
typename std::enable_if<!detector::Detect<TYPE>::RECOGNIZED, UndefinedType>::type& as(TYPE value) {
        static_assert(
            AlwaysFalse<TYPE>::value,
            ASSERT_TEXT("Input to `as` must be either:\n    - a pointer/reference to a context (Context<TYPE_MAP,COMPONENTS...>)\n    - a pointer/reference to a template specialization which is both specialized on the context type and a parent of the context type (COMPONENT<Context<TYPE_MAP,COMPONENTS...>>).")
        );
        return UndefinedType::value;
}

template<
    typename TRAIT,
    typename TRAIT_MAP,
    typename... COMPONENTS
>
auto& as(context::Context<TRAIT_MAP,COMPONENTS...> &context) {
    return context.template as<TRAIT>();
}

template<
    typename TRAIT,
    typename TRAIT_MAP,
    typename... COMPONENTS
>
auto& as(context::Context<TRAIT_MAP,COMPONENTS...> *context) {
    return (*context).template as<TRAIT>();
}

template<
    typename TRAIT,
    template<typename> typename START_COMP,
    typename TRAIT_MAP,
    typename...COMPONENTS
>
auto& as(START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>> *comp) {
    static_assert(
        std::is_base_of<
            START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>>,
            context::Context<TRAIT_MAP,COMPONENTS...>
        >::value,
        ASSERT_TEXT("Input to `as` must be a pointer/reference to a context or a template specialization which is both specialized on the context type and a parent of the context type.")
    );
    return as<TRAIT>(*static_cast<context::Context<TRAIT_MAP,COMPONENTS...>*>(comp));
}

template<
    typename TRAIT,
    template<typename> typename START_COMP,
    typename TRAIT_MAP,
    typename...COMPONENTS
>
auto& as(START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>> &comp) {
    static_assert(
        std::is_base_of<
            START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>>,
            context::Context<TRAIT_MAP,COMPONENTS...>
        >::value,
        ASSERT_TEXT("Input to `as` must be a pointer/reference to a context or a template specialization which is both specialized on the context type and a parent of the context type.")
    );
    return as<TRAIT>(*static_cast<context::Context<TRAIT_MAP,COMPONENTS...>*>(comp));
}



template<
    typename TRAIT,
    template<typename> typename START_COMP,
    typename TRAIT_MAP,
    typename...COMPONENTS
>
auto& via(START_COMP<context::Context<TRAIT_MAP,COMPONENTS...>>* comp) {
    return as<TRAIT>(*static_cast<context::Context<TRAIT_MAP,COMPONENTS...>*>(comp));
}





template<typename TRAIT,typename CONTEXT>
constexpr bool implements_trait() {
    return CONTEXT::template implements_trait<TRAIT>();
}




template<typename TRAIT,typename CTX>
using As = typename CTX::template ComponentLookup<TRAIT>;




namespace container {
namespace repr {

    template<typename STATE>
    struct StringRepr <context::Search<STATE>> {

        typedef context::Search<STATE> Type; 
        
        static StringReprNode repr_node() {
            return StringReprNode {
                "Search {",
                StringContentRepr<typename STATE::BindingArray>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }

    };

    
    template <typename... TYPES>
    struct StringRepr<context::RequirementSet<TYPES...>>
    {
        typedef context::RequirementSet<TYPES...> Type;
        static StringReprNode repr_node() {
            return StringReprNode {
                "RequirementSet {",
                StringContentRepr<typename Type::SetType::MapType::KeyArray>::repr(),
                "}"
            };
        }

        static std::string repr() {
            return repr_node().to_string();
        }
    };
    

}
}

#endif // HARMONIZE_CONTEXT_CONTEXT

