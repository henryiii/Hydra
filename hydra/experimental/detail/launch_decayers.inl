/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016 Antonio Augusto Alves Junior
 *
 *   This file is part of Hydra Data Analysis Framework.
 *
 *   Hydra is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Hydra is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Hydra.  If not, see <http://www.gnu.org/licenses/>.
 *
 *---------------------------------------------------------------------------*/

/*
 * launch_decayer
 *
 *  Created on: Jun 16, 2016
 *      Author: Antonio Augusto Alves Junior
 */

/**
 * \file
 * \ingroup phsp
 */


#ifndef LAUNCH_DECAYER_INC
#define LAUNCH_DECAYER_INC

#include <hydra/detail/Config.h>
#include <hydra/detail/BackendPolicy.h>
#include <hydra/Types.h>
#include <hydra/Containers.h>
#include <hydra/experimental/Events.h>
#include <hydra/experimental/detail/functors/DecayMother.h>
#include <hydra/experimental/detail/functors/DecayMothers.h>
#include <hydra/experimental/detail/functors/EvalMother.h>
#include <hydra/experimental/detail/functors/EvalMothers.h>

#include <hydra/detail/utility/Utility_Tuple.h>

#include <thrust/iterator/counting_iterator.h>
#include <thrust/sequence.h>
#include <thrust/tuple.h>
#include <thrust/transform.h>
#include <thrust/transform_reduce.h>

namespace hydra {
namespace experimental {
	namespace detail {

	template<size_t N, hydra::detail::Backend BACKEND, typename FUNCTOR, typename GRND, typename Iterator>
	inline StatsPHSP launch_evaluator(hydra::detail::BackendPolicy<BACKEND>const&,
			Iterator begin, Iterator end,	detail::EvalMother<N, GRND,FUNCTOR> const& evaluator)
	{
		typedef hydra::detail::BackendPolicy<BACKEND> system_t;

		StatsPHSP init = StatsPHSP();

		StatsPHSP result = thrust::transform_reduce(system_t() , begin, end,
				evaluator, init,detail::AddStatsPHSP() );

		return result;
	}


	template<size_t N, hydra::detail::Backend BACKEND, typename FUNCTOR, typename GRND, typename IteratorMother>
	inline StatsPHSP launch_evaluator(hydra::detail::BackendPolicy<BACKEND>const&,
			 IteratorMother begin, IteratorMother end, detail::EvalMothers<N, GRND,FUNCTOR> const& evaluator)
	{
		typedef hydra::detail::BackendPolicy<BACKEND> system_t;

		size_t nevents = thrust::distance(begin, end);
		thrust::counting_iterator<GLong_t> first(0);
		thrust::counting_iterator<GLong_t> last = first + nevents;

		StatsPHSP init = StatsPHSP();

		StatsPHSP result = thrust::transform_reduce(system_t(),
				thrust::make_zip_iterator(first, begin),
				thrust::make_zip_iterator(last, end),
				evaluator, init,detail::AddStatsPHSP() );

		return result;
	}



	template<size_t N, typename  BACKEND, typename GRND, typename Iterator>
    inline void launch_decayer(Iterator begin, Iterator end, DecayMother<N, BACKEND, GRND> const& decayer)
	{

		size_t nevents = thrust::distance(begin, end);
		thrust::counting_iterator<GLong_t> first(0);
		thrust::counting_iterator<GLong_t> last = first + nevents;

		auto begin_weights = thrust::get<0>(begin.get_iterator_tuple());

		auto begin_temp = hydra::detail::dropFirst( begin.get_iterator_tuple() );

		auto begin_particles = thrust::make_zip_iterator(begin_temp);

		thrust::transform(first, last, begin_particles, begin_weights, decayer);

		return;
	}


	template<size_t N, typename  BACKEND, typename GRND,	typename IteratorMother, typename IteratorDaughter>
	inline	void launch_decayer(IteratorMother begin, IteratorMother end
			, IteratorDaughter begin_daugters, DecayMothers<N, BACKEND,GRND> const& decayer)
	{

		size_t nevents = thrust::distance(begin, end);
		thrust::counting_iterator<GLong_t> first(0);
		thrust::counting_iterator<GLong_t> last = first + nevents;

		auto begin_weights = thrust::get<0>(begin_daugters.get_iterator_tuple());

		auto begin_temp = hydra::detail::changeFirst(  begin, begin_daugters.get_iterator_tuple() );

		auto begin_particles = thrust::make_zip_iterator(begin_temp);

		thrust::transform(first, last, begin_particles, begin_weights, decayer);

		return;
	}



	}
}  // namespace experimental

}// namespace hydra



#endif /* LAUNCH_DECAYER_INC */
