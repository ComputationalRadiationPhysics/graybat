<%#
    This is an ERB [1] template file used to generate the
    <boost/hana/detail/struct_macros.hpp> header. The maximum
    number of members that can be handled by the macros can
    be controlled with the 'MAX_NUMBER_OF_MEMBERS' variable,
    which can be set when calling ERB to generate the header:

        export MAX_NUMBER_OF_MEMBERS=55; erb struct_macros.erb.hpp

    'MAX_NUMBER_OF_MEMBERS' must be <= 62, otherwise an error is triggered.
    In case 'MAX_NUMBER_OF_MEMBERS' is not specified, it defaults to 40.

    [1]: http://en.wikipedia.org/wiki/ERuby
%>

<%
    MAX_NUMBER_OF_MEMBERS = (ENV["MAX_NUMBER_OF_MEMBERS"] || 40).to_i
    raise "MAX_NUMBER_OF_MEMBERS must be <= 62" if MAX_NUMBER_OF_MEMBERS > 62
%>

/*!
@file
Defines the `BOOST_HANA_DEFINE_STRUCT`, `BOOST_HANA_ADAPT_STRUCT`, and
`BOOST_HANA_ADAPT_ADT` macros.

@copyright Louis Dionne 2015
Distributed under the Boost Software License, Version 1.0.
(See accompanying file LICENSE.md or copy at http://boost.org/LICENSE_1_0.txt)
 */

//////////////////////////////////////////////////////////////////////////////
// THIS FILE IS GENERATED FROM THE <boost/hana/detail/struct_macros.erb.hpp>
// ERB TEMPLATE. DO NOT EDIT THIS FILE DIRECTLY.
//
// THE ERB TEMPLATE CONTAINS INFORMATION ABOUT HOW TO REGENERATE THIS FILE.
//////////////////////////////////////////////////////////////////////////////

#ifndef BOOST_HANA_DETAIL_STRUCT_MACROS_HPP
#define BOOST_HANA_DETAIL_STRUCT_MACROS_HPP

#include <boost/hana/detail/preprocessor.hpp>
#include <boost/hana/pair.hpp>
#include <boost/hana/string.hpp>
#include <boost/hana/tuple.hpp>

#include <cstddef>
#include <utility>


namespace boost { namespace hana { namespace struct_detail {
    template <typename Memptr, Memptr ptr>
    struct member_ptr {
        template <typename T>
        constexpr decltype(auto) operator()(T&& t) const
        { return static_cast<T&&>(t).*ptr; }
    };

    constexpr std::size_t strlen(char const* s) {
        std::size_t n = 0;
        while (*s++ != '\0')
            ++n;
        return n;
    }

    template <std::size_t n, typename Names, std::size_t ...i>
    constexpr auto prepare_member_name_impl(std::index_sequence<i...>) {
        return hana::string_c<hana::at_c<n>(Names::get())[i]...>;
    }

    template <std::size_t n, typename Names>
    constexpr auto prepare_member_name() {
        constexpr std::size_t len = strlen(hana::at_c<n>(Names::get()));
        return prepare_member_name_impl<n, Names>(std::make_index_sequence<len>{});
    }
}}}

//////////////////////////////////////////////////////////////////////////////
// BOOST_HANA_ADAPT_STRUCT
//////////////////////////////////////////////////////////////////////////////
template <typename ...>
struct BOOST_HANA_ADAPT_STRUCT_must_be_called_in_the_global_namespace;

#define BOOST_HANA_ADAPT_STRUCT(...)                                        \
  template <>                                                               \
  struct BOOST_HANA_ADAPT_STRUCT_must_be_called_in_the_global_namespace<>;  \
  BOOST_HANA_ADAPT_STRUCT_IMPL(BOOST_HANA_PP_NARG(__VA_ARGS__), __VA_ARGS__)\
  static_assert(true, "force the usage of a trailing semicolon")            \
/**/

#define BOOST_HANA_ADAPT_STRUCT_IMPL(N, ...) \
  BOOST_HANA_PP_CONCAT(BOOST_HANA_ADAPT_STRUCT_IMPL_, N)(__VA_ARGS__)

<% (0..MAX_NUMBER_OF_MEMBERS).each do |n| %>
#define BOOST_HANA_ADAPT_STRUCT_IMPL_<%= n+1 %>(TYPE <%= (1..n).map { |i| ", m#{i}" }.join %>)    \
    namespace boost { namespace hana {                                                            \
        template <>                                                                               \
        struct accessors_impl<TYPE> {                                                             \
            static constexpr auto apply() {                                                       \
                struct member_names {                                                             \
                  static constexpr auto get() {                                                   \
                      return ::boost::hana::make_tuple(                                           \
                          <%= (1..n).map { |i| "BOOST_HANA_PP_STRINGIZE(m#{i})" }.join(', ') %>   \
                      );                                                                          \
                  }                                                                               \
                };                                                                                \
                return ::boost::hana::make_tuple(                                                 \
                    <%= (1..n).map { |i| "::boost::hana::make_pair(::boost::hana::struct_detail::prepare_member_name<#{i-1}, member_names>(), ::boost::hana::struct_detail::member_ptr<decltype(&TYPE::m#{i}), &TYPE::m#{i}>{})" }.join(', ') %>\
                );                                                                                \
            }                                                                                     \
        };                                                                                        \
    }}                                                                                            \
/**/
<% end %>

//////////////////////////////////////////////////////////////////////////////
// BOOST_HANA_ADAPT_ADT
//////////////////////////////////////////////////////////////////////////////
template <typename ...>
struct BOOST_HANA_ADAPT_ADT_must_be_called_in_the_global_namespace;

#define BOOST_HANA_ADAPT_ADT(...)                                           \
  template <>                                                               \
  struct BOOST_HANA_ADAPT_ADT_must_be_called_in_the_global_namespace<>;     \
  BOOST_HANA_ADAPT_ADT_IMPL(BOOST_HANA_PP_NARG(__VA_ARGS__), __VA_ARGS__)   \
  static_assert(true, "force the usage of a trailing semicolon")            \
/**/

#define BOOST_HANA_ADAPT_ADT_IMPL(N, ...) \
  BOOST_HANA_PP_CONCAT(BOOST_HANA_ADAPT_ADT_IMPL_, N)(__VA_ARGS__)

<% (0..MAX_NUMBER_OF_MEMBERS).each do |n| %>
#define BOOST_HANA_ADAPT_ADT_IMPL_<%= n+1 %>(TYPE <%= (1..n).map { |i| ", m#{i}" }.join %>)             \
    namespace boost { namespace hana {                                                                  \
        template <>                                                                                     \
        struct accessors_impl<TYPE> {                                                                   \
            template <typename ...>                                                                     \
            static constexpr auto apply() {                                                             \
                struct member_names {                                                                   \
                  static constexpr auto get() {                                                         \
                      return ::boost::hana::make_tuple(                                                 \
                        <%= (1..n).map { |i| "BOOST_HANA_PP_STRINGIZE(BOOST_HANA_PP_FRONT m#{i})" }.join(', ') %>\
                      );                                                                                \
                  }                                                                                     \
                };                                                                                      \
                return ::boost::hana::make_tuple(                                                       \
                    <%= (1..n).map { |i| "::boost::hana::make_pair(::boost::hana::struct_detail::prepare_member_name<#{i-1}, member_names>(), BOOST_HANA_PP_DROP_FRONT m#{i})" }.join(', ') %>\
                );                                                                                      \
            }                                                                                           \
        };                                                                                              \
    }}                                                                                                  \
/**/
<% end %>

//////////////////////////////////////////////////////////////////////////////
// BOOST_HANA_DEFINE_STRUCT
//////////////////////////////////////////////////////////////////////////////
#define BOOST_HANA_DEFINE_STRUCT(...) \
    BOOST_HANA_DEFINE_STRUCT_IMPL(BOOST_HANA_PP_NARG(__VA_ARGS__), __VA_ARGS__)

#define BOOST_HANA_DEFINE_STRUCT_IMPL(N, ...) \
    BOOST_HANA_PP_CONCAT(BOOST_HANA_DEFINE_STRUCT_IMPL_, N)(__VA_ARGS__)

<% (0..MAX_NUMBER_OF_MEMBERS).each do |n| %>
#define BOOST_HANA_DEFINE_STRUCT_IMPL_<%= n+1 %>(TYPE <%= (1..n).map { |i| ", m#{i}" }.join %>)       \
  <%= (1..n).map { |i| "BOOST_HANA_PP_DROP_BACK m#{i} BOOST_HANA_PP_BACK m#{i};" }.join(' ') %>       \
                                                                                                      \
  struct hana_accessors_impl {                                                                        \
    static constexpr auto apply() {                                                                   \
      struct member_names {                                                                           \
        static constexpr auto get() {                                                                 \
            return ::boost::hana::make_tuple(                                                         \
              <%= (1..n).map { |i| "BOOST_HANA_PP_STRINGIZE(BOOST_HANA_PP_BACK m#{i})" }.join(', ') %>\
            );                                                                                        \
        }                                                                                             \
      };                                                                                              \
      return ::boost::hana::make_tuple(                                                               \
        <%= (1..n).map { |i| "::boost::hana::make_pair(::boost::hana::struct_detail::prepare_member_name<#{i-1}, member_names>(), ::boost::hana::struct_detail::member_ptr<decltype(&TYPE::BOOST_HANA_PP_BACK m#{i}), &TYPE::BOOST_HANA_PP_BACK m#{i}>{})" }.join(', ') %>\
      );                                                                                              \
    }                                                                                                 \
  }                                                                                                   \
/**/
<% end %>

#endif // !BOOST_HANA_DETAIL_STRUCT_MACROS_HPP
