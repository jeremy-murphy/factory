#include <boost/type_erasure/any.hpp>
#include <boost/type_erasure/builtin.hpp>
#include <boost/type_erasure/callable.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/bind.hpp>
#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>

#include <map>
#include <utility>

struct Arity {
    virtual ~Arity() = default;
};

struct Nullary : Arity {};

struct Unary : Arity {
    Unary(double) {}
};

/*
template <typename Callable, typename ReturnType, typename... Args>
struct Impl<Callable, ReturnType(Callable::*)(Args...)> : Concept {
  */

template <class AbstractProduct, typename IdentifierType>
class Factory
{
    using signatures = boost::mpl::vector<>;
public:
    template <IdentifierType ID, typename ProductCreator>
    bool Register(ProductCreator creator) {
        // typename boost::mpl::push_back<signatures, ProductCreator>::type;
        variant_handler
        // return associations_.emplace(id, creator).second;
    }

    bool Unregister(const IdentifierType& id) {
        // return associations_.erase(id) == 1;
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
    template <typename... Signatures>
    using multifunction = boost::type_erasure::any< boost::mpl::vector< boost::type_erasure::copy_constructible<>,
    boost::type_erasure::typeid_<>, boost::type_erasure::relaxed, boost::type_erasure::callable<Signatures>... > >;

    using variant_type = boost::variant<int, double, std::string> ;

    using function_type = multifunction<Arity*(), Arity*(int)>;

    class variant_handler
    {
    public:
        void handle(IdentifierType id, const variant_type& arg)
        {
            boost::apply_visitor(impl, arg);
        }
    };

    struct dispatcher : boost::static_visitor<void>
    {
        // used for the leaves
        template <IdentifierType ID, typename... Args>
        void operator()(Args... args) { f(args...); }
        // For a vector, we recursively operate on the elements
        function_type f;
    };

    std::map<IdentifierType, dispatcher> associations_;
};


int main(void)
{
    Factory<Arity*, int> factory;
    factory.Register(0, boost::factory<Nullary *>() );
    // MultiCtors nullaryFactory = boost::bind( boost::factory<Nullary *>() );
    // MultiCtors nullaryLambda = [](){ return new Nullary(); };
    // MultiCtors unaryLambda = [](double x){ return new Unary(x); };
    // MultiCtors unaryFactory = boost::bind( boost::factory<Unary *>(), _1 );
}

