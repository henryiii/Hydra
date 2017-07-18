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
 * GaussKronrodAdaptiveQuadrature.h
 *
 *  Created on: 04/02/2017
 *      Author: Antonio Augusto Alves Junior
 */


#ifndef GAUSSKRONRODADAPTIVEQUADRATURE_H_
#define GAUSSKRONRODADAPTIVEQUADRATURE_H_


#include <hydra/detail/Config.h>
#include <hydra/detail/BackendPolicy.h>
#include <hydra/Types.h>
#include <hydra/GaussKronrodRules.h>
#include <hydra/detail/functors/ProcessGaussKronrodAdaptiveQuadrature.h>
#include <hydra/multivector.h>
#include <hydra/detail/Integrator.h>

#include <hydra/detail/Print.h>
#include <tuple>
#include <vector>

namespace hydra {


template<size_t NRULE, size_t NBIN, typename BACKEND>
class GaussKronrodAdaptiveQuadrature;


/**
 *  @ingroup numerical_integration
 *
 *  @brief  Self-adaptive Gauss-Kronrod Quadrature.
 *  @tparam NRULE Order of the Gauss-Kronrod rule, specified as the number of Kronrod nodes.
 *  @tparam NBIN  Maximum number of multidimensional subdivisions of the integration region.
 *  @tparam BACKEND parallel back end. Can be hydra::omp::sys , hydra::cuda::sys , hydra::tbb::sys , hydra::cpp::sys ,hydra::host::sys and hydra::device::sys
 *
[*Description mostly copied From Wikipedia*](https://en.wikipedia.org/wiki/Gauss%E2%80%93Kronrod_quadrature_formula)

###Introduction###

In numerical mathematics, the Gauss–Kronrod quadrature formula is a method for numerical integration
(calculating approximate values of integrals). Gauss–Kronrod quadrature is a variant of Gaussian quadrature,
in which the evaluation points are chosen so that an accurate approximation can be computed by re-using the
information produced by the computation of a less accurate approximation. It is an example of what is called a
nested quadrature rule: for the same set of function evaluation points, it has two quadrature rules, one higher order
and one lower order (the latter called an embedded rule). The difference between these two approximations is used to estimate
the calculational error of the integration.
These formulas are named after Alexander Kronrod, who invented them in the 1960s, and Carl Friedrich Gauss.

###Description###

The problem in numerical integration is to approximate definite integrals of the form

\f[ I = \int_a^b f(x)\,dx \f]

Such integrals can be approximated, for example, by \f$n\f$-point [Gaussian quadrature](https://en.wikipedia.org/wiki/Gaussian_quadrature)

\f[ \int_a^b f(x)\,dx \approx \sum_{i=1}^n w_i f(x_i), \f]

where \f$w_i\f$, \f$x_i\f$ are the weights and points at which to evaluate the function \f$f(x)\f$.

If the interval \f$[a,b]\f$ is subdivided, the Gauss evaluation points of the new subintervals never coincide with the previous
evaluation points (except at the midpoint for odd numbers of evaluation points), and thus the integrand must be evaluated at every point.
Gauss–Kronrod formulas are extensions of the Gauss quadrature formulas generated by adding \f$n+1\f$ points to an \f$n\f$-point
rule in such a way that the resulting rule is of order \f$2n+1\f$
[[Math. Comp. 66 (1997), 1133-1145] ](http://www.ams.org/journals/mcom/1997-66-219/S0025-5718-97-00861-2/home.html)
the corresponding Gauss rule is of order \f$2n-1\f$. These extra points are the zeros of [Stieltjes polynomials](https://en.wikipedia.org/wiki/Stieltjes_polynomials).
This allows for computing higher-order estimates while reusing the function values of a lower-order estimate.
The difference between a Gauss quadrature rule and its Kronrod extension are often used as an estimate of the approximation error.

 */
template<size_t NRULE, size_t NBIN, hydra::detail::Backend BACKEND>
class GaussKronrodAdaptiveQuadrature<NRULE,NBIN, hydra::detail::BackendPolicy<BACKEND>>:
public Integrator< GaussKronrodAdaptiveQuadrature<NRULE, NBIN, hydra::detail::BackendPolicy<BACKEND> > >
{

	typedef hydra::detail::BackendPolicy<BACKEND> system_t;

	/**
	 * nodes
	 */
	typedef thrust::tuple<
			GBool_t, // process
			GUInt_t,  // bin
			double,  // lower
			double,  // upper
			double,  // integral
			double   // error
			> node_t;

	typedef std::vector<node_t>   node_list_h;
	typedef multivector<node_list_h> node_table_h;

	/*
	 * parameters
	 */
	typedef thrust::tuple<
			GUInt_t,// bin ,
			double, // abscissa_X_P
			double, // abscissa_X_M
			double, // Jacobian
			double, // rule_GaussKronrod_Weight
			double  // rule_Gauss_Weight
			> parameters_t;

	typedef std::vector<parameters_t>   parameters_list_h;
	typedef typename system_t::template container<parameters_t> parameters_list_d;

	typedef multivector<parameters_list_h> parameters_table_h;
	typedef multivector<parameters_list_d> parameters_table_d;

	/*
	 * call results
	 */
	typedef thrust::tuple<
			GUInt_t, // bin
			double,  // gauss
			double   // kronrod
			> call_t;

	typedef std::vector<call_t>   call_list_h;
	typedef typename system_t::template container<call_t> call_list_d;

	typedef multivector<call_list_h> call_table_h;
	typedef multivector<call_list_d> call_table_d;

public:

	/**
	 * @brief Hydra integrator tag
	 */
	typedef void hydra_integrator_tag;



	/**
	 * @brief Deleted constructor for Self-adaptive Gauss-Kronrod quadrature.
	 */
	GaussKronrodAdaptiveQuadrature()=delete;

	/**
	 * @brief Self-adaptive Gauss-Kronrod quadrature constructor.
	 *
	 * Self-adaptive Gauss-Kronrod quadrature constructor taking the integration region and the tolerance as parameters.
	 * @param xlower - lower range limit
	 * @param xupper - upper range limit
	 * @param tolerance - maximum absolute error
	 */
	GaussKronrodAdaptiveQuadrature(GReal_t xlower, GReal_t xupper, GReal_t tolerance=1e-15):
		fIterationNumber(0),
		fXLower(xlower),
		fXUpper(xupper),
		fMaxRelativeError( tolerance ),
		fRule(GaussKronrodRuleSelector<NRULE>().fRule)
	{ InitNodes(); }


	/**
	 * @brief Copy constructor
	 *
	 * @param other object at same backdend
	 */
	GaussKronrodAdaptiveQuadrature( GaussKronrodAdaptiveQuadrature<NRULE,NBIN, hydra::detail::BackendPolicy<BACKEND>> const& other ):
			fIterationNumber( other.GetIterationNumber() ),
			fXLower(other.GetXLower() ),
			fXUpper(other.GetXUpper()),
			fMaxRelativeError(other.GetMaxRelativeError() ),
			fRule(other.GetRule())
		{
			InitNodes();
		}

	/**
	 * @brief Copy constructor
	 * @tparam BACKEND2 different backdend specification.
	 * @param other object at a different backdend.
	 */
	template< hydra::detail::Backend  BACKEND2>
	GaussKronrodAdaptiveQuadrature( GaussKronrodAdaptiveQuadrature<NRULE,NBIN, hydra::detail::BackendPolicy<BACKEND2>> const& other ):
				fIterationNumber( other.GetIterationNumber() ),
				fXLower(other.GetXLower() ),
				fXUpper(other.GetXUpper()),
				fMaxRelativeError(other.GetMaxRelativeError() ),
				fRule(other.GetRule())
			{
				InitNodes();
			}


    /**
     * @brief Assignment operator
     *
     * @param other: object at same backdend
     * @return
     */
	GaussKronrodAdaptiveQuadrature&  operator= ( GaussKronrodAdaptiveQuadrature<NRULE,NBIN, hydra::detail::BackendPolicy<BACKEND>> const& other )
	{
		if(this ==&other) return *this;

		this->fIterationNumber = other.GetIterationNumber() ;
		this->fXLower = other.GetXLower() ;
		this->fXUpper = other.GetXUpper();
		this->fMaxRelativeError = other.GetMaxRelativeError() ;
		this->fRule=other.GetRule();
		this->InitNodes();

		return *this;
	}

	/**
	 * @brief Assignment operator
	 *
	 * @tparam BACKEND2 different backdend specification.
	 * @param other: object in a different backdend
	 * @return
	 */
	template< hydra::detail::Backend  BACKEND2>
	GaussKronrodAdaptiveQuadrature&  operator= ( GaussKronrodAdaptiveQuadrature<NRULE,NBIN, hydra::detail::BackendPolicy<BACKEND2>> const& other )
		{
			if(this ==&other) return *this;

			this->fIterationNumber = other.GetIterationNumber() ;
			this->fXLower = other.GetXLower() ;
			this->fXUpper = other.GetXUpper();
			this->fMaxRelativeError = other.GetMaxRelativeError() ;
			this->fRule=other.GetRule();
			this->InitNodes();

			return *this;
		}

	/**
	 * @brief Integrate method
	 *
	 * @param functor
	 * @return std::pair<GReal_t, GReal_t> with value and error.
	 */
	template<typename FUNCTOR>
	std::pair<GReal_t, GReal_t> Integrate(FUNCTOR const& functor);


	/**
	 * @brief Print integration limits, list of nodes ... to std::cout.
	 */
	void Print()
	{
		size_t nNodes   =  fNodesTable.size();
		HYDRA_CALLER ;
		HYDRA_MSG << "GaussKronrodAdaptiveQuadrature begin: " << HYDRA_ENDL;
		HYDRA_MSG << "XLower: " << fXLower << HYDRA_ENDL;
		HYDRA_MSG << "XUpper: " << fXUpper << HYDRA_ENDL;
		HYDRA_MSG << "#Nodes: " << nNodes << HYDRA_ENDL;
		for(size_t i=0; i< nNodes; i++ ){
			auto node = this->fNodesTable[i];
			HYDRA_MSG <<std::setprecision(50)<< "Node ID #" << thrust::get<1>(node) <<" Interval ["
					  << thrust::get<2>(node)
					  <<", "
					  << thrust::get<3>(node)
					  << "] Result ["
					  << thrust::get<4>(node)
					  << ", "
					  << thrust::get<5>(node)
					  << "]"
					  << " Process  "
					  << thrust::get<0>(node)
					  << HYDRA_ENDL;
		}
		fRule.Print();
		HYDRA_MSG << "GaussKronrodAdaptiveQuadrature end. " << HYDRA_ENDL;
	}


	GReal_t GetMaxRelativeError() const
	{
		return fMaxRelativeError;
	}

	void SetMaxRelativeError(GReal_t maxRelativeError)
	{
		fMaxRelativeError = maxRelativeError;
	}

	GReal_t GetXLower() const
	{
		return fXLower;
	}

	void SetXLower(GReal_t xLower)
	{
		fXLower = xLower;
		InitNodes();
	}

	GReal_t GetXUpper() const
	{
		return fXUpper;
	}

	void SetXUpper(GReal_t xUpper)
	{
		fXUpper = xUpper;
		InitNodes();
	}

	const GaussKronrodRule<NRULE>& GetRule() const
	{
		return fRule;
	}

private:

	GUInt_t GetIterationNumber() const
	{
		return fIterationNumber;
	}

	std::pair<GReal_t, GReal_t> Accumulate();

	GReal_t GetError( GReal_t delta)
	{
		return  std::max(std::numeric_limits<GReal_t>::epsilon(),
				std::pow(200.0*std::fabs(delta ), 1.5));
	}


	void InitNodes()
	{
		GReal_t delta = (fXUpper - fXLower)/NBIN;
		fNodesTable.resize(NBIN);

		for(size_t i=0; i<NBIN; i++ )
		{
			auto node = this->fNodesTable[i];
			thrust::get<0>(node) = 	1;
			thrust::get<1>(node) = 	i;
			thrust::get<2>(node) = 	this->fXLower + i*delta;
			thrust::get<3>(node) = 	this->fXLower + (i+1)*delta;
			thrust::get<4>(node) = 	0.0;
			thrust::get<5>(node) = 	0.0;
		}

	}

	size_t CountNodesToProcess()
	{
		auto begin = fNodesTable.template vbegin<0>();
		auto end   = fNodesTable.template vend<0>();

	    size_t n=0;
		for(auto i = begin; i!=end; i++)
		if(*i)n++;
		return n;
	}

	void SetParametersTable( )
	{

		size_t nNodes =  CountNodesToProcess();

		fParametersTable.clear();
		fParametersTable.resize(nNodes*(NRULE+1)/2);
		parameters_table_h temp_table(nNodes*(NRULE+1)/2);

		//for(size_t i=0; i<nNodes; i++)
		size_t i=0;
		for(auto node : fNodesTable)
		{

			if(!thrust::get<0>(node)) 	continue;

			for(size_t call=0; call<(NRULE+1)/2; call++)
			{
				GReal_t abscissa_X_P = 0;
				GReal_t abscissa_X_M = 0;
				GReal_t jacobian = 0;
				GReal_t fLowerLimits= thrust::get<2>(node);
				GReal_t fUpperLimits= thrust::get<3>(node);

				thrust::tie(abscissa_X_P, abscissa_X_M, jacobian)
				= fRule.GetAbscissa(call , fLowerLimits, fUpperLimits);

				GReal_t rule_GaussKronrod_Weight   = fRule.KronrodWeight[call];
				GReal_t rule_Gauss_Weight                = fRule.GaussWeight[call];

				size_t index = call*nNodes + i;

				temp_table[index]= parameters_t(thrust::get<1>(node), abscissa_X_P, abscissa_X_M,
						jacobian, rule_GaussKronrod_Weight, rule_Gauss_Weight);
			}

			i++;
		}

	//	for(auto row: temp_table) std::cout << row << std::endl;
		thrust::copy( temp_table.begin(), temp_table.end(),
				fParametersTable.begin() );

	}

	void UpdateNodes();

	GUInt_t fIterationNumber;
	GReal_t fXLower;
	GReal_t fXUpper;
	GReal_t fMaxRelativeError;
	node_table_h  fNodesTable;
	parameters_table_d fParametersTable;
	call_table_h fCallTableHost;
	call_table_d fCallTableDevice;

	GaussKronrodRule<NRULE> fRule;

};


} // namespace hydra

#include <hydra/detail/GaussKronrodAdaptiveQuadrature.inl>

#endif /* GAUSSKRONRODADAPTIVEQUADRATURE_H_ */
