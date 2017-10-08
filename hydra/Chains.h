/*----------------------------------------------------------------------------
 *
 *   Copyright (C) 2016-2017 Antonio Augusto Alves Junior
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
 * Chains.h
 *
 *  Created on: 07/10/2017
 *      Author: Antonio Augusto Alves Junior
 */

#ifndef CHAINS_H_
#define CHAINS_H_


#include <utility>
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <cassert>

#include <hydra/detail/Config.h>
#include <hydra/detail/BackendPolicy.h>
#include <hydra/Types.h>
#include <hydra/Containers.h>
#include <hydra/Decays.h>
#include <hydra/detail/utility/Utility_Tuple.h>
#include <hydra/detail/utility/Generic.h>
#include <hydra/detail/functors/FlagAcceptReject.h>
//thrust
#include <hydra/detail/external/thrust/tuple.h>
#include <hydra/detail/external/thrust/iterator/zip_iterator.h>
#include <hydra/detail/external/thrust/distance.h>

namespace hydra {


template<typename ...Decays>
struct Chains;

/**
 * @ingroup phsp
 * @brief Memory storage for a chain of decays.
 *
 * Allocate memory for a chain of decays. Each decay needs be generated by a corresponding instance of hydra::PhaseSpace.
 * @tparam N number of particles in final states of each decay.
 * @tparam BACKEND
 */
template<size_t ...N, hydra::detail::Backend BACKEND>
class Chains< Decays<N,hydra::detail::BackendPolicy<BACKEND> >...>
{


	typedef hydra::detail::BackendPolicy<BACKEND> system_t;



	typedef HYDRA_EXTERNAL_NS::thrust::tuple<typename
			Decays<N,hydra::detail::BackendPolicy<BACKEND> >::iterator...> iterator_tuple;
	typedef HYDRA_EXTERNAL_NS::thrust::tuple<typename
			Decays<N,hydra::detail::BackendPolicy<BACKEND> >::const_iterator...> const_iterator_tuple;

	typedef typename system_t::template container<GReal_t>  vector_real;
	typedef typename system_t::template container<GReal_t>::iterator vector_real_iterator;
	typedef typename system_t::template container<GReal_t>::const_iterator vector_real_const_iterator;

	typedef typename system_t::template container<GBool_t>  vector_bool;
	typedef typename system_t::template container<GBool_t>::iterator  vector_bool_iterator;
	typedef typename system_t::template container<GBool_t>::const_iterator  vector_bool_const_iterator;

	//zipped iterators
	typedef HYDRA_EXTERNAL_NS::thrust::zip_iterator<
			decltype(HYDRA_EXTERNAL_NS::thrust:: tuple_cat(HYDRA_EXTERNAL_NS::thrust::tuple<vector_real_iterator>(), iterator_tuple()))>  iterator;
	typedef HYDRA_EXTERNAL_NS::thrust::zip_iterator<
			decltype(HYDRA_EXTERNAL_NS::thrust:: tuple_cat(HYDRA_EXTERNAL_NS::thrust::tuple<vector_real_const_iterator>(), const_iterator_tuple()))>  const_iterator;

	typedef   typename iterator::value_type value_type;
	typedef   typename iterator::reference  reference_type;

	typedef decltype(hydra::detail::make_index_sequence<sizeof...(N)> { }) indexing_type;

public:

	typedef HYDRA_EXTERNAL_NS::thrust::tuple<	Decays<N,hydra::detail::BackendPolicy<BACKEND> >...> event_tuple;
	/**
	 * @brief default constructor
	 */
	Chains(){};

	/**
	 * @brief Constructor allocating memory for a given number of events.
	 * @param nevents number of events
	 */
	Chains(size_t nevents);

	/**
	 * @brief Copy constructor
	 * @param other chain container defined at same back-end.
	 */
	Chains(Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND> >...>const& other);

	/**
	 * @brief Copy constructor
	 * @param other chain container defined in other back-end.
	 */
	template<hydra::detail::Backend BACKEND2>
	Chains(Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND2> >...>const& other);


/**
 *@brief  Move constructor
 * @param other
 */
	Chains(Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND> >...>&& other);


/**
 * @brief Assignment operator for chain container allocated in the same back-end
 * @param other
 * @return
 */
	Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND> >...>&
	operator=(Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND> >...> const& other);

	/**
	 * @brief Assignment operator for chain container allocated in a different back-end.
	 * @param other
	 * @return
	 */
	template<hydra::detail::Backend BACKEND2>
	Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND> >...>&
	operator=(Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND2> >...>const& other);

	/**
	 * @brief Move-assignment operator for chain container allocated in a same back-end.
	 * @param other
	 * @return
	 */
	Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND> >...>&
	operator=(Chains<Decays<N,hydra::detail::BackendPolicy<BACKEND> >...>&& other);

	/**
	 * @brief Constructor from a list of decay containers.
	 * @param events
	 */
	Chains(Decays<N,hydra::detail::BackendPolicy<BACKEND> > const& ...events);


	/**
 	 * @brief Get the number of events.
 	 * @return
 	 */
	size_t GetNDecays() const {return fSize;}

	/**
	 * @brief Iterator to begin of accept/reject flags range.
	 * @return iterator
	 */
	vector_bool_iterator FlagsBegin() {return fFlags.begin();}

	/**
	 * @brief Iterator to end of accept/reject flags range.
	 * @return iterator
	 */
	vector_bool_iterator FlagsEnd() {return fFlags.end();}

	/**
	 * @brief Iterator to begin of weights range.
	 * @return iterator
	 */
	vector_real_iterator WeightsBegin() {return fWeights.begin();}

	/**
	 * @brief Iterator to end of weights range.
	 * @return iterator
	 */
	vector_real_iterator WeightsEnd() {return fWeights.end();}

	/*
	 * constant access iterators
	 */
	vector_bool_const_iterator FlagsBegin() const{return fFlags.cbegin();}

	vector_bool_const_iterator FlagsEnd() const{return fFlags.cend();}

	vector_real_const_iterator WeightsBegin() const{ return fWeights.cbegin();	}

	vector_real_const_iterator WeightsEnd() const { return fWeights.cend(); }

	template<unsigned int I>
	auto GetDecay() const
	-> typename HYDRA_EXTERNAL_NS::thrust::tuple_element<I, event_tuple>::type&
	{ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage);	}

	template<unsigned int I>
	auto GetDecay()
	-> typename HYDRA_EXTERNAL_NS::thrust::tuple_element<I, event_tuple>::type&
	{ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage);	}

	/**
	 * @brief Iterator to begin of container range.
	 * @return iterator
	 */
	iterator begin(){ return fBegin; }

	/**
	 * @brief Iterator to end of container range.
	 * @return iterator
	 */
	iterator  end(){ return fEnd; }

	/*
	 * constant access iterators
	 */
	const_iterator  begin() const{ return fConstBegin; }
	const_iterator    end() const{ return fConstEnd; }
	const_iterator cbegin() const{ return fConstBegin; }
	const_iterator   cend() const{ return fConstEnd; }

	/**
	 * @brief  Iterator to begin of decay container range.
	 * @return
	 */
	template<unsigned int I>
	iterator begin(){ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage).begin(); }

	/**
	 * @brief  Iterator to end of decay container range.
	 * @return
	 */
	template<unsigned int I>
	iterator  end(){ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage).end() ; }

	/*
	 * constant access iterators
	 */
	template<unsigned int I>
	const_iterator begin() const{ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage).cbegin() ; }

	template<unsigned int I>
	const_iterator  end() const{ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage).cend() ; }

	template<unsigned int I>
	const_iterator cbegin() const{ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage).cbegin() ; }

	template<unsigned int I>
	const_iterator  cend() const{ return HYDRA_EXTERNAL_NS::thrust::get<I>(fStorage).cend() ; }

	/**
	 * @brief capacity.
	 * @return capacity in number of elements
	 */
	size_t capacity() const  { return fFlags.capacity(); }

	/**
	 * @brief resize container to N.
	 */
	void resize(size_t n);

	/**
	 * @brief size of the container in number of elements.
	 * @return
	 */
	size_t size() const { return fSize; }

	/**
	 * @brief subscript operator.
	 * @param i
	 * @return reference to chain i
	 */
	reference_type operator[](size_t i)
	{ return fBegin[i];	}


	const reference_type operator[](size_t i) const
	{ return fConstBegin[i]; }

	event_tuple const& CopyStorage() const { return fStorage;}


private:

	event_tuple MoveStorage(){ return std::move(fStorage);}

	size_t	CheckSizes(std::initializer_list<size_t> sizes)
	{
		assert(std::adjacent_find( sizes.begin(), sizes.end(),
				std::not_equal_to<size_t>() ) == sizes.end());
		size_t s=*sizes.begin();
		return	s;
	}

	event_tuple fStorage;
	vector_bool fFlags; ///< Vector of flags. Accepted events are flagged 1 and rejected 0.
	vector_real fWeights; ///< Vector of event weights.
	iterator fBegin;
	iterator fEnd;
	const_iterator fConstBegin;
	const_iterator fConstEnd;
	size_t   fSize;


};

template<size_t ...N,hydra::detail::Backend BACKEND >
Chains< Decays<N, hydra::detail::BackendPolicy<BACKEND> >...>
make_chain( hydra::detail::BackendPolicy<BACKEND> const& policy, size_t entries )
{
	return std::move( Chains<Decays<N, hydra::detail::BackendPolicy<BACKEND> >...>(entries) );
}


template<size_t I, hydra::detail::Backend BACKEND, size_t ...N>
auto get_decay(Chains<Decays<N, hydra::detail::BackendPolicy<BACKEND> >...>& chain )
-> typename HYDRA_EXTERNAL_NS::thrust::tuple_element<I,
      typename Chains<Decays<N, hydra::detail::BackendPolicy<BACKEND> >...>::event_tuple>::type&
{
	return chain.template GetDecay<I>();
}


}  // namespace hydra

#include <hydra/detail/Chains.inl>




#endif /* CHAINS_H_ */
