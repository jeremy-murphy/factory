#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <functional>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>

template <typename T, typename Signature>
struct signature_impl;

template <typename T, typename ReturnType, typename... Args>
struct signature_impl<T, ReturnType(T::*)(Args...)>
{
    using type = ReturnType(Args...);
};

template <typename T>
using signature_t = signature_impl<T, decltype(&T::operator())>;

template <class AbstractProduct, typename IdentifierType>
class Factory
{
    using AssociativeContainers = std::map<IdentifierType, boost::factory<AbstractProduct>>;
public:
    template <typename ProductCreator>
    bool Register(const IdentifierType& id, ProductCreator creator) {
        // auto &foo = std::get<std::map<IdentifierType, boost::factory<AbstractProduct>>>(associations_);
        return associations_.emplace(id, creator).second;
    }

    bool Unregister(const IdentifierType& id) {
        return associations_.erase(id) == 1;
    }

    template <typename... Arguments>
    AbstractProduct CreateObject(const IdentifierType& id, Arguments&& ... args) {
        auto i = associations_.find(id);
        if (i != associations_.end()) {
            return (i->second)(std::forward<Arguments>(args)...);
        }
        throw std::runtime_error("Creator not found.");
    }

private:
    AssociativeContainers associations_;
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
    Factory<Arity*, int> factory;
    factory.Register(0, boost::factory<Nullary*>() );
    // factory.Register(0, boost::function<Unary*(double)>(boost::bind(boost::factory<Unary*>(), _1)) );
}

