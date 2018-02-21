#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <cassert>
#include <map>
#include <tuple>
#include <type_traits>
#include <utility>


template <class AbstractProduct, typename IdentifierType, typename... ProductCreators>
class Factory
{
    using AssociativeContainers = std::tuple<std::map<IdentifierType, boost::function<ProductCreators>>...>;
public:
    template <typename Product, typename... Arguments>
    bool Register(const IdentifierType& id, boost::function<Product(Arguments...)> creator) {
        auto &foo = std::get<std::map<IdentifierType, boost::function<AbstractProduct(const Arguments&...)>>>(associations_);
        return foo.emplace(id, creator).second;
    }

    bool Unregister(const IdentifierType& id) {
        return associations_.erase(id) == 1;
    }

    template <typename... Arguments>
    AbstractProduct CreateObject(const IdentifierType& id, Arguments&& ... args) const {
        auto const &foo = std::get<std::map<IdentifierType, boost::function<AbstractProduct(const Arguments&...)>>>(associations_);
        auto const i = foo.find(id);
        if (i != foo.end()) {
            return (i->second)(std::forward<Arguments...>(args)...);
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
    Unary() {}
    Unary(double x) : x(x) {}

    double x;
};


int main(void)
{
    Factory<Arity*, int, Arity*(), Arity*(const double&)> factory;
    factory.Register(0, boost::function<Arity*()>{boost::factory<Nullary*>()} );
    factory.Register(1, boost::function<Arity*(const double&)>{boost::bind(boost::factory<Unary*>(), _1)});
    auto x = factory.CreateObject(1, 2.0);
    assert(typeid(*x) == typeid(Unary));
    x = factory.CreateObject(0);
    assert(typeid(*x) == typeid(Nullary));
}
