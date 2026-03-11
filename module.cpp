#include <type_traits>
#include <cassert>
#include <stdexcept>
#include <vector>
#include <cxxabi.h>

#include "preamble.h"


#include "sanity_check.cpp"


#include "config.h"


#include "container/mod.h"
#include "context/mod.h"
#include "core/mod.h"


#include "postamble.h"

#include "module_check.cpp"


typedef container::TypeSet<
	ArrayStack<float>
> Requirements;

typedef context::ModuleBundle<
	CPUArrayStack<float>,
	CPUDynArray<float>,
	GPUArrayAlloc<float>,
	CPUArrayAlloc<float>,
    CPU
> Bundle;

typedef context::DepMapBuild<Bundle,Requirements> DepMapBuild;
typedef typename DepMapBuild::Result DepMap;

typedef typename context::Prune<DepMap> Prune;

typedef typename context::DepMapCheck<Requirements,DepMap,typename Prune::Result> Check;

typedef typename context::Reify<Requirements,Bundle,context::EagerSolve>::Result Context;



int main() {

    std::cout << container::repr::StringRepr<typename context::SolutionSequence<DepMapBuild>::Result>::repr() << std::endl;
    std::cout << container::repr::StringRepr<DepMapBuild::Result>::repr() << std::endl;
    std::cout << container::repr::StringRepr<typename context::SolutionSequence<Prune>::Result>::repr() << std::endl;

    std::cout << Check::unsat_diagnostic_string();

    Context context;

    return 0;
}






