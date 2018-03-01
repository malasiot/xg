#ifndef __XG_VARIANT_HPP__
#define __XG_VARIANT_HPP__

#include <iostream>
#include <utility>
#include <typeinfo>
#include <type_traits>
#include <string>

// simple variant class inspired from https://gist.github.com/tibordp/6909880

namespace xg {

template<typename... Ts>
struct VariantHelper;

template<typename F, typename... Ts>
struct VariantHelper<F, Ts...> {
    inline static void destroy(size_t id, void * data)
    {
        if (id == typeid(F).hash_code())
            reinterpret_cast<F*>(data)->~F();
        else
            VariantHelper<Ts...>::destroy(id, data);
    }

    inline static void copy(size_t old_t, const void * old_v, void * new_v)
    {
        if (old_t == typeid(F).hash_code())
            new (new_v) F(*reinterpret_cast<const F*>(old_v));
        else
            VariantHelper<Ts...>::copy(old_t, old_v, new_v);
    }
};

template<> struct VariantHelper<>  {
    inline static void destroy(size_t id, void * data) { }
    inline static void copy(size_t old_t, const void * old_v, void * new_v) { }
};


template <typename...> struct is_one_of {
    static constexpr bool value = false;
};

template <typename T, typename S, typename... Ts> struct is_one_of<T, S, Ts...> {
    static constexpr bool value = std::is_same<T, S>::value || is_one_of<T, Ts...>::value;
};

template<typename... Ts>
struct Variant {
private:
    using data_t = typename std::aligned_union<0, Ts...>::type;
    using helper_t = VariantHelper<Ts...>;

    size_t type_id_ ;
    data_t data_ ;

    static inline size_t invalid_type() {
        return typeid(void).hash_code();
    }

public:

    Variant() : type_id_(invalid_type()) {   }

    Variant(const Variant<Ts...>& old) : type_id_(old.type_id_)
    {
        helper_t::copy(old.type_id_, &old.data_, &data_);
    }


    template <typename T, typename... Args, typename = typename std::enable_if<is_one_of<T, Ts...>::value, void>::type>
    void set(Args&&... args)  {
        // First we destroy the current contents
        helper_t::destroy(type_id_, &data_);
        new (&data_) T(std::forward<Args>(args)...);
        type_id_ = typeid(T).hash_code();
    }

    template <typename T, typename = typename std::enable_if<is_one_of<T, Ts...>::value, void>::type>
    T get() const {
        // It is a dynamic_cast-like behaviour
        if (type_id_ == typeid(T).hash_code())
            return *reinterpret_cast<const T*>(&data_);
        else
            throw std::bad_cast();
    }

    template <typename T, typename = typename std::enable_if<is_one_of<T, Ts...>::value, void>::type>
    T& get() {
        // It is a dynamic_cast-like behaviour
        if (type_id_ == typeid(T).hash_code())
            return *reinterpret_cast<T*>(&data_);
        else
            throw std::bad_cast();
    }

    ~Variant() {
        helper_t::destroy(type_id_, &data_);
    }
};


} // namespace xg

#endif
