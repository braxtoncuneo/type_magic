#ifndef HARMONIZE_CONTAINER_UNDEF
#define HARMONIZE_CONTAINER_UNDEF

struct UndefinedType {
    private:
    UndefinedType(){};
    public:
    static UndefinedType value;

    template <typename TYPE>
    operator TYPE() {
        throw std::runtime_error("Attempted to cast value of undefined type.");
    }
};
UndefinedType UndefinedType::value = UndefinedType();

#endif // HARMONIZE_CONTAINER_UNDEF