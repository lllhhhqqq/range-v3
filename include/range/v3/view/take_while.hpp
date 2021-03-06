/// \file
// Range v3 library
//
//  Copyright Eric Niebler 2013-2014
//
//  Use, modification and distribution is subject to the
//  Boost Software License, Version 1.0. (See accompanying
//  file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
// Project home: https://github.com/ericniebler/range-v3
//

#ifndef RANGES_V3_VIEW_TAKE_WHILE_HPP
#define RANGES_V3_VIEW_TAKE_WHILE_HPP

#include <utility>
#include <functional>
#include <type_traits>
#include <meta/meta.hpp>
#include <range/v3/range_fwd.hpp>
#include <range/v3/range_concepts.hpp>
#include <range/v3/range_adaptor.hpp>
#include <range/v3/utility/functional.hpp>
#include <range/v3/utility/semiregular.hpp>
#include <range/v3/utility/iterator_concepts.hpp>
#include <range/v3/utility/static_const.hpp>
#include <range/v3/view/view.hpp>

namespace ranges
{
    inline namespace v3
    {
        /// \addtogroup group-views
        /// @{
        template<typename Rng, typename Pred, bool Inf /*= is_infinite<Rng>::value*/>
        struct take_while_view
          : range_adaptor<take_while_view<Rng, Pred, Inf>, Rng, Inf>
        {
        private:
            friend range_access;
            semiregular_t<invokable_t<Pred>> pred_;

            template<bool IsConst>
            struct sentinel_adaptor
              : adaptor_base
            {
            private:
                semiregular_ref_or_val_t<invokable_t<Pred>, IsConst> pred_;
            public:
                sentinel_adaptor() = default;
                sentinel_adaptor(semiregular_ref_or_val_t<invokable_t<Pred>, IsConst> pred)
                  : pred_(std::move(pred))
                {}
                bool empty(range_iterator_t<Rng> it, range_sentinel_t<Rng> end) const
                {
                    return it == end || !pred_(*it);
                }
            };

            sentinel_adaptor<false> end_adaptor()
            {
                return {pred_};
            }
            CONCEPT_REQUIRES(Invokable<Pred const, range_common_reference_t<Rng>>())
            sentinel_adaptor<true> end_adaptor() const
            {
                return {pred_};
            }
        public:
            take_while_view() = default;
            take_while_view(Rng rng, Pred pred)
              : range_adaptor_t<take_while_view>{std::move(rng)}
              , pred_(invokable(std::move(pred)))
            {}
        };

        namespace view
        {
            struct take_while_fn
            {
            private:
                friend view_access;
                template<typename Pred>
                static auto bind(take_while_fn take_while, Pred pred)
                RANGES_DECLTYPE_AUTO_RETURN
                (
                    make_pipeable(std::bind(take_while, std::placeholders::_1,
                        protect(std::move(pred))))
                )
            public:
                template<typename Rng, typename Pred>
                using Concept = meta::and_<
                    InputIterable<Rng>,
                    IndirectInvokablePredicate<Pred, range_iterator_t<Rng>>>;

                template<typename Rng, typename Pred,
                    CONCEPT_REQUIRES_(Concept<Rng, Pred>())>
                take_while_view<all_t<Rng>, Pred> operator()(Rng && rng, Pred pred) const
                {
                    return {all(std::forward<Rng>(rng)), std::move(pred)};
                }
            #ifndef RANGES_DOXYGEN_INVOKED
                template<typename Rng, typename Pred,
                    CONCEPT_REQUIRES_(!Concept<Rng, Pred>())>
                void operator()(Rng &&, Pred) const
                {
                    CONCEPT_ASSERT_MSG(InputIterable<Rng>(),
                        "The object on which view::take_while operates must be a model of the "
                        "InputIterable concept.");
                    CONCEPT_ASSERT_MSG(IndirectInvokablePredicate<Pred, range_iterator_t<Rng>>(),
                        "The function passed to view::take_while must be callable with objects of "
                        "the range's common reference type, and its result type must be "
                        "convertible to bool.");
                }
            #endif
            };

            /// \relates take_while_fn
            /// \ingroup group-views
            namespace
            {
                constexpr auto&& take_while = static_const<view<take_while_fn>>::value;
            }
        }
        /// @}
    }
}

#endif
