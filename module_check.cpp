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

namespace container {

    using namespace ::container;


    void repr_check() {

        info("Checking Representations");
        typedef TypeMap<
            Binding<float,int>,
            Binding<bool,float>,
            Binding<int,bool>
        > SmallMap;
        std::cout << repr::StringRepr<SmallMap>::repr() << std::endl;

        typedef TypeMap<
            Binding<
                TypeMap<Binding<int,float>>,
                int
            >,
            Binding<
                bool,
                TypeMap<
                    Binding<bool,TypeMap<>>
                >
            >,
            Binding<int,bool>
        > ComplexMap;
        std::cout << repr::StringRepr<ComplexMap>::repr() << std::endl;
       
        typedef TypeSet<int,float,bool> SmallSet;
        std::cout << repr::StringRepr<SmallSet>::repr() << std::endl;
       
        typedef TypeArray<int,float,bool> SmallArray;
        std::cout << repr::StringRepr<SmallArray>::repr() << std::endl;


    }

    void map_check() {
        info("Checking MapStruct");
        typedef TypeMap<
            Binding<float,int>,
            Binding<bool,float>,
            Binding<int,bool>
        > M;
        MapStruct<M> map;
        map.get<float>() = 123;
        map.get<bool>() = 4.56f;
        map.get<int>() = false;
        assert(map.get<float>()==123);
        assert(map.get<bool>()==4.56f);
        assert(map.get<int>()==false);
    }

    void union_check() {
        info("Checking TypeSet union");
        typedef TypeSet<int,bool>   A;
        typedef TypeSet<bool,float> B;
        typedef typename A::template Union<B>::type C;
        static_assert(std::is_same<
            C,
            TypeSet<int,bool,float>
        >::value);
    }

    void intersect_check() {
        info("Checking TypeSet intersect");
        typedef TypeSet<int,bool,float> A;
        typedef TypeSet<double,float,bool>  B;
        typedef typename A::template Intersection<B>::type C;
        static_assert(std::is_same<
            C,
            TypeSet<bool,float>
        >::value);
    }

    void difference_check() {
        info("Checking TypeSet difference");
        typedef TypeSet<int,bool,float,double> A;
        typedef TypeSet<int,float>  B;
        typedef typename A::template Difference<B>::type C;
        static_assert(std::is_same<
            C,
            TypeSet<bool,double>
        >::value);
    }


    void set_union_check() {
        info("Checking SetUnion");
        typedef SetUnion<TypeSet<int,float,bool>> SA;
        SA set_union;
        set_union.get<int>() = 123;
        assert(set_union.get<int>() == 123);
        set_union.get<float>() = 4.56f;
        assert(set_union.get<float>() == 4.56f);
        set_union.get<bool>() = true;
        assert(set_union.get<bool>() == true);
    };

    void type_array_concat_check() {
        info("Checking TypeArray concat");
        typedef TypeArray<int,float> A;
        typedef TypeArray<bool,double> B;
        typedef typename A::template Concatenate<B>::type C;
        static_assert(std::is_same<C,TypeArray<int,float,bool,double>>::value);
    };


    struct TruthyVisitor {
        template<typename T> bool operator()(T value) { return (bool) value; }
    };

    void array_variant_check() {
        info("Checking ArrayVariant");
        typedef ArrayVariant<TypeArray<int,float,bool>> AV;
        AV array_variant;
        assert(!array_variant.holds<0>());
        assert(!array_variant.valid());
        array_variant.set<0>(123);
        assert(array_variant.valid());
        assert(array_variant.unsafe_get<0>() == 123);
        assert(array_variant.holds<0>());
        assert(!array_variant.holds<1>());

        assert(array_variant.visit(TruthyVisitor{}));
    };

    void order_check() {
        typedef TypeMap<
            Binding<int,bool>,
            Binding<double,char>,
            Binding<float,float>,
            Binding<char,double>
        > M;
        typedef TypeMap<
            Binding<char,double>,
            Binding<float,float>,
            Binding<int,bool>,
            Binding<double,char>
        > Q;
        static_assert(std::is_same<MapStruct<M>::MapType,MapStruct<Q>::MapType>::value);
    }

}


    class CheckLaunch {
        static CheckLaunch launcher;
        public:
        CheckLaunch() {
            using namespace container;
            info("Running checks...");
            repr_check();
            map_check();
            union_check();
            intersect_check();
            difference_check();
            set_union_check();
            type_array_concat_check();
            array_variant_check();
            order_check();
        }
    };
    CheckLaunch CheckLaunch::launcher = CheckLaunch();

};


