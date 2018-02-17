#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/callable.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/bind.hpp>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>

#include <map>
#include <utility>

template <class AbstractProduct, typename IdentifierType, typename ProductCreator>
class Factory
{
public:
    bool Register(const IdentifierType& id, ProductCreator creator) {
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
    std::map<IdentifierType, ProductCreator> associations_;
};

namespace te = boost::type_erasure;
template <typename... Signatures>
using multifunction = te::any< boost::mpl::vector< te::copy_constructible<>,
te::typeid_<>, te::relaxed, te::callable<Signatures>... > >;

struct Arity {
    virtual ~Arity() = default;
};

struct Nullary : Arity {};

struct Unary : Arity {
    Unary(double) {}
};

using MultiCtors = multifunction<Arity *(), Arity *(double)>;

int main(void)
{
    Factory<Arity*, int, boost::function<Arity*()>> factory;
    factory.Register(0, boost::factory<Nullary *>() );
    // MultiCtors nullaryFactory = boost::bind( boost::factory<Nullary *>() );
    // MultiCtors nullaryLambda = [](){ return new Nullary(); };
    // MultiCtors unaryLambda = [](double x){ return new Unary(x); };
    // MultiCtors unaryFactory = boost::bind( boost::factory<Unary *>(), _1 );
}

