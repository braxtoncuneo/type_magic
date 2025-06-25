#include <iostream>

namespace check {

    char const * YELLOW  = "\e[33m";
    char const * DEFAULT = "\e[0m";

    template<typename T>
    void info(T& value) {
        std::cout << YELLOW << "# "<< value << DEFAULT << std::endl;
    }

    /*
    void set_check() {
        info("Checking TypeSet");
        TypeSet<int,float,bool> set;
        set.get<int>() = 123;
        set.get<float>() = 4.56f;
        set.get<bool>() = false;
        assert(set.get<int>()==123);
        assert(set.get<float>()==4.56f);
        assert(set.get<bool>()==false);
    }
    */

    void map_check() {
        info("Checking MapStruct");
        MapStruct<TypeMap<
            Binding<float,int>,
            Binding<bool,float>,
            Binding<int,bool>
        >> map;
        map.get<float>() = 123;
        map.get<bool>() = 4.56f;
        map.get<int>() = false;
        assert(map.get<float>()==123);
        assert(map.get<bool>()==4.56f);
        assert(map.get<int>()==false);
    }

    void union_check() {
        info("Checking TypeMap union");
        typedef TypeMap<Binding<int,bool>>   A;
        typedef TypeMap<Binding<bool,float>> B;
        typedef typename A::template Union<B>::type C;
        static_assert(std::is_same<
            C,
            TypeMap<Binding<int,bool>,Binding<bool,float>>
        >::value);
    }

    void intersect_check() {
        info("Checking TypeMap intersect");
        typedef TypeMap<Binding<int,bool>,  Binding<bool,float>> A;
        typedef TypeMap<Binding<bool,float>,Binding<float,int>>  B;
        typedef typename A::template Intersection<B>::type C;
        static_assert(std::is_same<
            C,
            TypeMap<Binding<bool,float>>
        >::value);
    }

    void difference_check() {
        info("Checking TypeMap difference");
        typedef TypeMap<Binding<int,bool>,  Binding<bool,float>> A;
        typedef TypeMap<Binding<bool,float>>  B;
        typedef typename A::template Difference<B>::type C;
        static_assert(std::is_same<
            C,
            TypeMap<Binding<int,bool>>
        >::value);
    }

    class CheckLaunch {
        static CheckLaunch launcher;
        public:
        CheckLaunch() {
            info("Running checks...");
            //set_check();
            map_check();
            union_check();
            intersect_check();
            difference_check();
        }
    };
    CheckLaunch CheckLaunch::launcher = CheckLaunch();

};


