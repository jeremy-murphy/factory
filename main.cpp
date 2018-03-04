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
#include <utility>
#include <type_traits>
#include <unordered_map>
#include <vector>


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

template <class AbstractProduct, typename IdentifierType, typename... ProductCreators>
class Factory
{
    /*
    using multifunction = boost::type_erasure::any< boost::mpl::vector< boost::type_erasure::copy_constructible<>,
    boost::type_erasure::typeid_<>, boost::type_erasure::relaxed, boost::type_erasure::callable<ProductCreators>... > >;
    */

    using function_variant = boost::variant<std::function<ProductCreators>...>;

    template <typename Signature>
    struct dispatcher_impl;

    template <typename ReturnType, typename... Args>
    struct dispatcher_impl<ReturnType(Args...)>
    {
        AbstractProduct operator()(Args... args) const
        {
            int status;
            std::cout << "static call to visitor: " << abi::__cxa_demangle(typeid(*this).name(), nullptr, 0, &status) << "\n";
        }
    };

    struct dispatcher : boost::static_visitor<AbstractProduct>, dispatcher_impl<ProductCreators>...
    {
    };

    dispatcher impl;

    std::map<IdentifierType, function_variant> associations_;

public:
    template <typename ProductCreator>
    bool Register(IdentifierType id, ProductCreator creator) {
        return associations_.emplace(id, creator).second;
    }

    bool Unregister(const IdentifierType& id) {
        // return associations_.erase(id) == 1;
    }

    template <typename... Arguments>
    AbstractProduct CreateObject(const IdentifierType& id, Arguments&& ... args) {
        auto i = associations_.find(id);
        if (i != associations_.end()) {
            return boost::apply_visitor(impl, *i);
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
    // factory.CreateObject(0);
}
