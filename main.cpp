#include <boost/functional/factory.hpp>
#include <boost/function.hpp>
#include <boost/variant.hpp>

#include <cxxabi.h>

#include <map>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>
// Just for debugging.
#include <iostream>
#include <typeinfo>

// Tuple manipulation that is standard in C++17.

template <typename Signature>
struct signature_impl;

template <typename ReturnType, typename... Args>
struct signature_impl<ReturnType(Args...)>
{
    using return_type = ReturnType;
    using param_types = std::tuple<Args...>;
};

template <typename T>
using signature_t = signature_impl<T>;


template <std::size_t... Ints>
struct indices {};

template <std::size_t N, std::size_t... Ints>
struct build_indices : build_indices<N-1, N-1, Ints...> {};

template <std::size_t... Ints>
struct build_indices<0, Ints...> : indices<Ints...> {};

template <typename Tuple>
using make_tuple_indices = build_indices<std::tuple_size<typename std::remove_reference<Tuple>::type>::value>;


template <class AbstractProduct, typename IdentifierType, typename... ProductCreators>
class multifactory
{
    using function_variant = boost::variant<boost::function<ProductCreators>...>;

    std::map<IdentifierType, function_variant> associations_;

    template <typename Signature>
    struct dispatch_foo
    {
        template <typename CreateArgs, std::size_t... Indices>
        typename std::enable_if<std::is_convertible<CreateArgs, typename signature_t<Signature>::param_types>::value, AbstractProduct>::type
        apply(boost::function<Signature> const &f, CreateArgs && t, indices<Indices...>) const
        {
            return f(std::get<Indices>(std::forward<CreateArgs>(t))...);
        }

        template <typename CreateArgs, std::size_t... Indices>
        typename std::enable_if<!std::is_convertible<CreateArgs, typename signature_t<Signature>::param_types>::value, AbstractProduct>::type
        apply(boost::function<Signature> const &, CreateArgs &&, indices<Indices...>) const
        {
            return nullptr;
        }
    };

    template <typename CreateArguments, typename... Signatures>
    struct dispatcher_impl;

    template <typename CreateArguments, typename Signature>
    struct dispatcher_impl<CreateArguments, Signature> : dispatch_foo<Signature>
    {
        CreateArguments args;

        template <typename... Foo>
        dispatcher_impl(Foo &&... args) : args{std::forward_as_tuple(args...)} {}

        using dispatch_foo<Signature>::apply;

        AbstractProduct operator()(boost::function<Signature> const &f) const
        {
            int status;
            std::cout << "visitor: " << abi::__cxa_demangle(typeid(Signature).name(), nullptr, 0, &status) << "\n";
            return apply(f, args, make_tuple_indices<CreateArguments>{});
        }
    };

    template <typename CreateArguments, typename Signature, typename... Signatures>
    struct dispatcher_impl<CreateArguments, Signature, Signatures...> : dispatcher_impl<CreateArguments, Signatures...>, dispatch_foo<Signature>
    {
        using dispatcher_impl<CreateArguments, Signatures...>::operator();
        using dispatcher_impl<CreateArguments, Signatures...>::args;

        template <typename... Foo>
        dispatcher_impl(Foo &&... args) : dispatcher_impl<CreateArguments, Signatures...>(std::forward<Foo>(args)...) {}

        using dispatch_foo<Signature>::apply;

        AbstractProduct operator()(boost::function<Signature> const &f) const
        {
            int status;
            std::cout << "visitor: " << abi::__cxa_demangle(typeid(Signature).name(), nullptr, 0, &status) << "\n";
            return apply(f, args, make_tuple_indices<CreateArguments>{});
        }
    };

    template <typename... CreateArguments>
    struct dispatcher : boost::static_visitor<AbstractProduct>, dispatcher_impl<std::tuple<CreateArguments...>, ProductCreators...>
    {
        dispatcher(CreateArguments &&... args) : dispatcher_impl<std::tuple<CreateArguments...>, ProductCreators...>(std::forward<CreateArguments>(args)...) {}

        using dispatcher_impl<std::tuple<CreateArguments...>, ProductCreators...>::operator();
    };

public:
    template <typename ProductCreator>
    bool Register(IdentifierType id, ProductCreator &&creator) {
        return associations_.emplace(id, std::forward<ProductCreator>(creator)).second;
    }

    bool Unregister(const IdentifierType& id) {
        return associations_.erase(id) == 1;
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
    Unary() {} // Also has nullary ctor.
    Unary(int) {}
};


int main(void)
{
    multifactory<Arity*, int, Arity*(), Arity*(const int&)> factory;
    factory.Register(0, boost::function<Arity*()>( boost::factory<Nullary*>() ));
    factory.Register(1, boost::function<Arity*(const int&)>(boost::factory<Unary*>()) );
    auto a = factory.CreateObject(0);
    assert(a);
    assert(typeid(*a) == typeid(Nullary));
    auto b = factory.CreateObject(1, 2);
    assert(b);
    assert(typeid(*b) == typeid(Unary));
}
