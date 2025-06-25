#ifndef HARMONIZE_CONTAINER_ASSIGN
#define HARMONIZE_CONTAINER_ASSIGN

template<typename TYPE>
struct SurjectiveAssign {
    TYPE &value;
};

template<typename TYPE>
struct InjectiveAssign {
    TYPE &value;
};

template<typename TYPE>
struct BijectiveAssign {
    TYPE &value;
};


#endif
