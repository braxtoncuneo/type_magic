#include "../../../include/include.h"

#include <chrono>
#include <thread>

#include <functional>


struct ExecA{};
struct ExecB{};

struct ProgressDisplay{};

template<typename T>
struct ProgressReport{};

template<typename T>
struct ProgressRelay{};









template<typename CONTEXT>
struct TaskA
{

    size_t index;
    size_t limit;

    std::thread task;
    
    TaskA (size_t limit) : index(0), limit(limit) {}

    void run () {
        for (index=0; index < limit; index++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));        
            if constexpr ( implements_trait<ProgressRelay<ExecA>,CONTEXT>() ) {
                via<ProgressRelay<ExecA>>(this).progress = index/(float)limit;
            }
        }
        if constexpr ( implements_trait<ProgressRelay<ExecA>,CONTEXT>() ) {
            via<ProgressRelay<ExecA>>(this).progress = 1;
        }
    }

    void start() {
        task = std::thread(std::mem_fn(&TaskA::run),this);
    }

};

using TaskAModule = context::SimpleModule<
    Meta<TaskA>,
    context::RequirementSet<>,
    context::ImplementationSet<ExecA,ProgressReport<ExecA>>
>;



template<typename CONTEXT>
struct TaskB
{

    size_t index;
    size_t limit;

    std::thread task;

    TaskB (size_t limit) : index(0), limit(limit) {}

    void run () {
        for (index=0; index < limit; index++) {
            std::this_thread::sleep_for(std::chrono::milliseconds(2));        
            if constexpr ( implements_trait<ProgressRelay<ExecB>,CONTEXT>() ) {
                via<ProgressRelay<ExecB>>(this).progress = (index/(float)limit);
            }
        }
        if constexpr ( implements_trait<ProgressRelay<ExecB>,CONTEXT>() ) {
            via<ProgressRelay<ExecB>>(this).progress = 1;
        }
    }

    void start() {
        task = std::thread(std::mem_fn(&TaskB::run),this);
    }

};

using TaskBModule = context::SimpleModule<
    Meta<TaskB>,
    context::RequirementSet<>,
    context::ImplementationSet<ExecB,ProgressReport<ExecB>>
>;









template <typename LABEL>
struct ProgressRelayComponent {
    
    template <typename CONTEXT>
    struct Component {
        float progress;
    };

    using Module = context::SimpleModule <
        Meta<Component>,
        context::ImplementationSet<ProgressRelay<LABEL>>,
        context::RequirementSet<ProgressReport<LABEL>>
    >;

};


using ProgressRelayModule = context::MetaModule <
    ProgressRelay,
    ProgressRelayComponent
>;




template <typename CONTEXT>
struct ProgressDisplayComponent {
    
    private:
    
    template<typename T>
    bool complete_recurse () {
        if constexpr ( std::is_same<T,container::TypeSet<>>::value ) {
            return true;
        } else {
            float progress = via<typename T::MapType::HeadItemType>(this).progress;
            if (progress >= 1.0) {
                return complete_recurse<typename T::MapType::TailType::KeySet>();
            } else {
                return false;
            }
        }
    }
    
    template<typename T>
    int display_recurse (unsigned int width) {
        if constexpr ( std::is_same<T,container::TypeSet<>>::value ) {
            return 0;
        } else {
            using CurrentLabel = typename T::MapType::HeadItemType;
            float progress = via<CurrentLabel>(this).progress;
            std::cout << "["; 
            for (unsigned int i=0; i<width; i++) {
                if ( (i/(float)width) <= progress ) {
                    std::cout << "|"; 
                } else {
                    std::cout << " ";
                }
            }
            std::cout << "] " << container::repr::type_name<CurrentLabel>() << std::endl; 
            return display_recurse<typename T::MapType::TailType::KeySet>(width) + 1;
        }
    }

    public:
    bool complete() {
        typedef typename CONTEXT::TraitMap::KeySet::template Filter<Meta<ProgressRelay>::template Generalizes>::type RelaySet;
        return complete_recurse<RelaySet>();
    }

    void display(unsigned int width) {
        typedef typename CONTEXT::TraitMap::KeySet::template Filter<Meta<ProgressRelay>::template Generalizes>::type RelaySet;
        int bar_count = display_recurse<RelaySet>(width);
        std::cout << "\x1b[" << bar_count << "A\r";
    }

};


using ProgressDisplayModule = context::SimpleModule<
    Meta<ProgressDisplayComponent>,
    context::ImplementationSet<ProgressDisplay>
>;



using RootModule = context::ModuleBundle<
    TaskAModule,
    TaskBModule,
    ProgressRelayModule,
    ProgressDisplayModule
>;

template <typename CTX>
void run()
{

    if constexpr (CTX::Info::SATISFIED)
    {

        CTX ctx(
            As<ExecA, CTX>{1000},
            As<ExecB, CTX>{1000}
        );



        as<ExecA>(ctx).start();
        as<ExecB>(ctx).start();
        
        while (! as<ProgressDisplay>(ctx).complete() ) {
            as<ProgressDisplay>(ctx).display(30);
        }

        as<ExecA>(ctx).task.join();
        as<ExecB>(ctx).task.join();
    }

    else
    {
        CTX ctx;
        std::cout << as<context::ContextInfo>(ctx).error_string();
    }
}


int main() {

    typedef typename context::CreateContextType<
        RootModule,
        container::TypeSet<
            ExecA,
            ExecB,
            ProgressRelay<ExecA>,
            ProgressRelay<ExecB>,
            ProgressDisplay
        >,
        Meta<context::EagerSolve>>::type Ctx;

    run<Ctx>();

    return 0;
}



