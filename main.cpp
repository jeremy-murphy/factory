#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/callable.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/bind.hpp>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>

#include <map>
#include <utility>

// Generic multi-function factory

template <class AbstractProduct, typename IdentifierType>
class Factory
{
public:
    template <typename Callable>
    bool Register(const IdentifierType& id, Callable creator) {
        return associations_.emplace(id, Impl<Callable, decltype(&Callable::operator())>(creator)).second;
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
    struct Concept {
        template <typename... Args>
        AbstractProduct operator()(Args... args) {}
    };

    template <typename Callable, typename Signature>
    struct Impl {};

    template <typename Callable, typename ReturnType, typename... Args>
    struct Impl<Callable, ReturnType(Callable::*)(Args...)> : Concept {
        Impl(Callable f) : f{f} {}

        ReturnType operator()(Args... args) const {
            return f(args...);
        }
        boost::function<ReturnType(Args...)> f;
    };

    std::map<IdentifierType, Concept> associations_;
};


struct Arity {
    virtual ~Arity() = default;
};

struct Nullary : Arity {};

struct Unary : Arity {
    Unary(double) {}
};

struct NullaryFactory
{
    Nullary *operator()() { return new Nullary(); }
};

struct UnaryFactory
{
    Unary *operator()(int x) { return new Unary(x); }
};

int main(void)
{
    Factory<Arity*, int> factory;
    factory.Register(0, NullaryFactory() );
    factory.Register(1, UnaryFactory() );

    auto a = factory.CreateObject(0);
    // auto b = factory.CreateObject(1, 7);

    // factory.Register(0, [&](){ return new Nullary(); } );
    // factory.Register(0, boost::factory<Nullary *>());
    // factory.Register(0, boost::bind( boost::factory<Nullary *>() ));
    // factory.Register(1, boost::bind( boost::factory<Unary *>(), _1 ));
    // MultiCtors nullaryFactory = boost::bind( boost::factory<Nullary *>() );
    // MultiCtors nullaryLambda = [](){ return new Nullary(); };
    // MultiCtors unaryLambda = [](double x){ return new Unary(x); };
    // MultiCtors unaryFactory = boost::bind( boost::factory<Unary *>(), _1 );
}


namespace te = boost::type_erasure;
template <typename... Signatures>
using multifunction = te::any< boost::mpl::vector< te::copy_constructible<>,
te::typeid_<>, te::relaxed, te::callable<Signatures>... > >;
using MultiCtors = multifunction<Arity *(), Arity *(double)>;
