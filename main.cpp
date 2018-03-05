#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/callable.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/bind.hpp>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>

#include <cxxabi.h>

#include <iostream>
#include <map>
#include <tuple>
#include <utility>

// Tuple manipulation that is standard in C++17.
/*
template <typename T, typename Signature>
struct signature_impl;

template <typename T, typename ReturnType, typename... Args>
struct signature_impl<T, ReturnType(T::*)(Args...)>
{
    using return_type = ReturnType;
    using param_types = std::tuple<Args...>;
};

template <typename T>
using signature_t = signature_impl<T, decltype(&T::operator())>;
*/

template <std::size_t...Is> struct index_sequence {};

template <std::size_t N, std::size_t...Is>
struct build : public build<N - 1, N - 1, Is...> {};

template <std::size_t...Is>
struct build<0, Is...> {
    using type = index_sequence<Is...>;
};

template <std::size_t N>
using make_index_sequence = typename build<N>::type;


template <class AbstractProduct, typename IdentifierType, typename... ProductCreators>
class Factory
{
    using function_variant = boost::variant<std::function<ProductCreators>...>;

    std::map<IdentifierType, function_variant> associations_;

    template <typename... Signatures>
    struct dispatcher_impl;

    template <typename Signature>
    struct dispatcher_impl<Signature>
    {
      AbstractProduct operator()(std::function<Signature> const &f) const
      {
        int status;
        std::cout << "static call to visitor: " << abi::__cxa_demangle(typeid(f).name(), nullptr, 0, &status) << "\n";
        return nullptr;
      }
    };

    template <typename Signature, typename... Signatures>
    struct dispatcher_impl<Signature, Signatures...> : dispatcher_impl<Signatures...>
    {
        using dispatcher_impl<Signatures...>::operator();

        AbstractProduct operator()(std::function<Signature> const &f) const
        {
            int status;
            std::cout << "static call to visitor: " << abi::__cxa_demangle(typeid(f).name(), nullptr, 0, &status) << "\n";
            return nullptr;
        }
    };

    template <typename... CreateArguments>
    struct dispatcher : boost::static_visitor<AbstractProduct>, dispatcher_impl<ProductCreators...>
    {
        std::tuple<CreateArguments...> args; // TODO: How is operator() is dispatcher_impl going to access this?
        // static constexpr make_index_sequence<std::tuple_size<std::tuple<CreateArguments...>>::value> seq{};

        dispatcher(CreateArguments &&... args) : args{std::forward<CreateArguments>(args)...} {}

        using dispatcher_impl<ProductCreators...>::operator();
    };

public:
    template <typename ProductCreator>
    bool Register(IdentifierType id, ProductCreator &&creator) {
        return associations_.emplace(id, std::forward<ProductCreator>(creator)).second;
    }

    bool Unregister(const IdentifierType& id) {
        // return associations_.erase(id) == 1;
    }

    template <typename... Arguments>
    AbstractProduct CreateObject(const IdentifierType& id, Arguments&& ... args) {
        auto i = associations_.find(id);
        if (i != associations_.end()) {
            dispatcher<Arguments...> impl(std::forward<Arguments>(args)...);
            return boost::apply_visitor(impl, i->second);
        }
        throw std::runtime_error("Creator not found.");
    }
};

struct Arity {
    virtual ~Arity() = default;
};

struct Nullary : Arity {};

struct Unary : Arity {
    Unary(double) {}
};


int main(void)
{
    Factory<Arity*, int, Arity*(), Arity*(const double&)> factory;
    factory.Register(0, boost::function<Arity*()>( boost::factory<Nullary*>() ));
    factory.Register(1, boost::function<Arity*(const double&)>(boost::factory<Unary*>()) );
    factory.CreateObject(0);
    factory.CreateObject(1, 2);
}
