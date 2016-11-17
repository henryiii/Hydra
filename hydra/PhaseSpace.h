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

/*-
 * PhaseSpace.h
 *
 * Created on : Feb 25, 2016
 *      Author: Antonio Augusto Alves Junior
 */


/**
 * \file
 * \ingroup phsp
 */


#ifndef PHASESPACE_H_
#define PHASESPACE_H_

#include <array>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <ostream>
#include <algorithm>
#include <time.h>
#include <stdio.h>
//#include <math.h>

#if(THRUST_HOST_SYSTEM==THRUST_HOST_SYSTEM_OMP || THRUST_DEVICE_SYSTEM==THRUST_DEVICE_SYSTEM_OMP)
#include <omp.h>
#endif

#include <hydra/detail/Config.h>
#include <hydra/Types.h>
#include <hydra/Containers.h>
#include <hydra/Vector3R.h>
#include <hydra/Vector4R.h>
#include <hydra/Events.h>
#include <hydra/detail/functors/DecayMother.h>
#include <hydra/detail/functors/DecayMothers.h>
#include <hydra/detail/functors/FlagAcceptReject.h>
#include <hydra/detail/functors/IsAccepted.h>
#include <hydra/detail/utility/Generic.h>
#include <hydra/strided_iterator.h>
#include <hydra/detail/launch_decayers.inl>

#include <thrust/iterator/zip_iterator.h>
#include <thrust/iterator/counting_iterator.h>
#include <thrust/tuple.h>
#include <thrust/extrema.h>
#include <thrust/random.h>
#include <thrust/distance.h>


#include <thrust/system/omp/execution_policy.h>


using namespace std;

namespace hydra {


template <size_t N, typename GRND=thrust::random::default_random_engine>
class PhaseSpace {

public:

	/**
	 * PhaseSpace ctor. Constructor of the phase-space generator takes as input parameters:
	 * - _MotherMass: the mass of the mother particle in Gev/c*c
	 * - _Masses: STL vector with the mass of the daughter particles.
	 */
	PhaseSpace(GReal_t _MotherMass, vector<GReal_t> _Masses) :
		fNDaughters(_Masses.size()),
		fSeed(1)

{

		fMasses.resize(_Masses.size());
		thrust::copy(_Masses.begin(), _Masses.end(), fMasses.begin());


		GReal_t fTeCmTm = 0.0;

		fTeCmTm = _MotherMass; // total energy in C.M. minus the sum of the masses

		for (size_t n = 0; n < fNDaughters; n++) {
			fTeCmTm -= _Masses[n];
		}
		if (fTeCmTm < 0.0) {
			cout << "Not enough energy for this decay. Exit." << endl;
			exit(1);
		}

}    //decay


	~PhaseSpace() {}

	template<typename Iterator>
	void Generate(Vector4R const& mother, Iterator begin, Iterator end)
	{
		/**
		 * Run the generator and calculate the maximum weight. It takes as input the fourvector of the mother particle
		 * in any system of reference. The daughters will be generated in this system.
		*/

#if(THRUST_DEVICE_SYSTEM==THRUST_DEVICE_BACKEND_CUDA && (BACKEND==device))
	cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);
#endif


	detail::DecayMother<N, detail::IteratorTraits<Iterator>::type::backend,GRND> decayer(mother,fMasses, fNDaughters, fSeed);
			detail::launch_decayer(begin, end, decayer );

	}

	template<typename Iterator1, typename Iterator2>
	void Generate( Iterator1 begin, Iterator1 end, Iterator1 mothers_begin)
	{
		/**
		 * Run the generator and calculate the maximum weight. It takes as input the device vector with the four-vectors of the mother particle
		 * in any system of reference. The daughters will be generated in this system.
		 */

#if(THRUST_DEVICE_SYSTEM==THRUST_DEVICE_BACKEND_CUDA && (BACKEND==device))
	cudaDeviceSetCacheConfig(cudaFuncCachePreferL1);
#endif


	detail::DecayMothers<N, detail::IteratorTraits<Iterator1>::type::backend,GRND> decayer(fMasses, fNDaughters, fSeed);
		detail::launch_decayer(begin, end, mothers_begin, decayer );



	}

	inline GInt_t GetSeed() const	{
			return fSeed;
		}

	inline void SetSeed(GInt_t _seed) 	{
				fSeed=_seed;
			}


	/**
	 * PDK function
	 */
	inline GReal_t PDK(const GReal_t a, const GReal_t b, const GReal_t c) const {
		//the PDK function
		GReal_t x = (a - b - c) * (a + b + c) * (a - b + c) * (a + b - c);
		x = sqrt(x) / (2 * a);
		return x;
	}

private:


	GInt_t  fNDaughters;///< Number of daughters.
	GInt_t  fSeed;///< seed.
	vector<GReal_t> fMasses;




};

}//namespace hydra
#include <hydra/detail/PhaseSpace.inl>

#endif /* PHASESPACE_H_ */
