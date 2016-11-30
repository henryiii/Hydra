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
 * Function.h
 *
 *  Created on: 05/05/2016
 *      Author: Antonio Augusto Alves Junior
 */

/**
 * \file
 * \ingroup functor
 */


#ifndef FUNCTION_H_
#define FUNCTION_H_

#include <hydra/detail/Config.h>
#include <hydra/Types.h>
#include <hydra/detail/Print.h>
#include <hydra/Parameter.h>
#include <hydra/detail/utility/Utility_Tuple.h>
#include <hydra/detail/FunctorTraits.h>
#include <thrust/iterator/detail/tuple_of_iterator_references.h>
#include <thrust/iterator/zip_iterator.h>
#include <thrust/tuple.h>
#include <array>
#include <initializer_list>
#include <memory>


namespace hydra
{

/*
template<typename Functor, typename ReturnType, size_t NPARAM>
struct BaseFunctor
{
	//tag
    typedef void hydra_functor_tag;

	typedef   ReturnType return_type;
	typedef   std::true_type is_functor;
    static const size_t parameter_count =NPARAM;

	__host__ __device__
	BaseFunctor():
	fCacheIndex(-1),
	fCached(0),
	fParamRegistered(0)
	//fNorm(1.0)
	{	}

	__host__ __device__
	BaseFunctor(BaseFunctor<Functor,ReturnType, NPARAM> const& other):
	fCacheIndex( other.GetCacheIndex() ),
	fCached( other.IsCached() ),
	fParamRegistered(0)
	{
		for(int i=0;i<NPARAM;i++){
			fParameters[i]=other.GetParameter( i);
		}

	}

	__host__ __device__
	BaseFunctor<Functor,ReturnType, NPARAM>&
	operator=(BaseFunctor<Functor, ReturnType, NPARAM> const & other )
	{
		if(this != &other)
		{
			this->fCacheIndex     = other.GetCacheIndex();
			this->fCached         = other.IsCached();
			this->fParamRegistered =0;
			for(int i=0;i<NPARAM;i++){
				this->fParameters[i]=other.GetParameter( i);
					}
         }
		return *this;
	}

	__host__ __device__
	~BaseFunctor()
	{ }

	__host__ __device__ inline
	Functor& GetFunctor() {return *static_cast<Functor*>(this);}

	__host__ __device__ inline
	int GetCacheIndex() const { return this->fCacheIndex; }

	__host__ __device__ inline
	void SetCacheIndex(int index) {fCacheIndex = index;}

	__host__ __device__ inline
	bool IsCached() const
	{ return this->fCached;}

	__host__ __device__ inline
	void SetCached(bool cached=true)
	{ fCached = cached; }


	void PrintRegisteredParameters()
	{

		HYDRA_CALLER ;
		HYDRA_MSG <<HYDRA_ENDL;
		HYDRA_MSG << "Registered parameters begin:" << HYDRA_ENDL;
		for(size_t i=0; i<parameter_count; i++ )
		HYDRA_MSG <<"  >> Parameter " << i <<") "<< fParameters[i] << HYDRA_ENDL;
		HYDRA_MSG <<"Registered parameters end." << HYDRA_ENDL;
		HYDRA_MSG <<HYDRA_ENDL;
		return;
	}

	__host__ inline
	void SetParameters(const std::vector<double>& parameters){

		if(fCached) return;

		for(size_t i=0; i< parameter_count; i++){
			fParameters[i].Reset(parameters );
		}

		if (INFO >= hydra::Print::Level()  )
		{
			std::ostringstream stringStream;
			for(size_t i=0; i< parameter_count; i++){
			     stringStream << "Parameter["<< fParameters[i].GetIndex() <<"] :  " << parameters[fParameters[i].GetIndex() ]
			                    << "  " << fParameters[i] << "\n";
			}
			HYDRA_LOG(INFO, stringStream.str().c_str() )
		}

		return;
	}


	__host__  __device__ inline
	 const Parameter GetParameter(size_t i) const {
		return fParameters[i];
	}


	__host__ __device__  inline
	Parameter& GetParameter(size_t i) {
		return fParameters[i];
	}


	__host__ __device__  inline
		void SetParameter(size_t i, Parameter const& par) {
			 fParameters[i]=par;
		}

	__host__ __device__
	GReal_t operaror[](size_t 1)
	{
		return fParameters[i];
	}

	template<typename T  >
	__host__ __device__ inline
	typename thrust::detail::enable_if< detail::is_homogeneous<
	typename thrust::tuple_element<0, typename std::remove_reference<T>::type>::type, typename std::remove_reference<T>::type>::value, return_type>::type
	interface(T&& x)
	{
		typedef typename std::remove_reference<T>::type Tprime;
		typedef typename std::remove_reference<typename thrust::tuple_element<0, Tprime>::type>::type first_type;
		constexpr size_t N = thrust::tuple_size< Tprime >::value;

		first_type Array[ N ];

		detail::tupleToArray(x, &Array[0] );

		return static_cast<Functor*>(this)->Evaluate(&Array[0]);


	}

	template<typename T >
	__host__ __device__ inline
	typename thrust::detail::enable_if<!detail::is_homogeneous<
	typename thrust::tuple_element<0, typename std::remove_reference<T>::type>::type, typename std::remove_reference<T>::type>::value, return_type>::type
	interface(T&& x)
	{

		return static_cast<Functor*>(this)->Evaluate(x);


	}



	template<typename T>
	__host__  __device__ inline
	return_type operator()( T* x, T* p=0  )
	{return static_cast<Functor*>(this)->Evaluate(x); }


	template<typename T>
	__host__ __device__ inline
	return_type operator()( T&&  x )
	{return interface< T>(std::forward< T >(x));}


	template<typename T1, typename T2>
	__host__ __device__  inline
	return_type operator()( T1&& x, T2 && cache)
	{

		return fCached ?\
				detail::extract<return_type, T2 >(fCacheIndex, std::forward<T2>(cache)):\
				operator()<T1>( std::forward<T1>(x) );
	}


private:

	int  fCacheIndex;
	bool fCached;
	bool fParamRegistered;
	Parameter fParameters[NPARAM];


};
*/

template<typename Functor, typename ReturnType, size_t NPARAM>
struct BaseFunctor
{
	//tag
    typedef void hydra_functor_tag;

	typedef   ReturnType return_type;
	typedef   std::true_type is_functor;
    static const size_t parameter_count =NPARAM;

	__host__ __device__
	BaseFunctor():
	fCacheIndex(-1),
	fCached(0),
	fParamResgistered(0)
	//fNorm(1.0)
	{	}

	__host__ __device__
	BaseFunctor(BaseFunctor<Functor,ReturnType, NPARAM> const& other):
	fCacheIndex( other.GetCacheIndex() ),
	fCached( other.IsCached() ),
	fParamResgistered(0)
	//fNorm(other.GetNorm())
	{ }

	__host__ __device__ inline
	BaseFunctor<Functor,ReturnType, NPARAM>&
	operator=(BaseFunctor<Functor, ReturnType, NPARAM> const & other )
	{
		if(this != &other)
		{
			this->fCacheIndex     = other.GetCacheIndex();
			this->fCached         = other.IsCached();
			//this->fParameterIndex = other.GetParameterIndex();
			//this->fNorm = other.GetNorm();
			this->fParamResgistered =0;

         }
		return *this;
	}

	__host__ __device__
	~BaseFunctor()
	{ }

	__host__ __device__ inline
	Functor& GetFunctor() {return *static_cast<Functor*>(this);}

	__host__ __device__ inline
	int GetCacheIndex() const { return this->fCacheIndex; }

	__host__ __device__ inline
	void SetCacheIndex(int index) {fCacheIndex = index;}

	__host__ __device__ inline
	bool IsCached() const
	{ return this->fCached;}

	__host__ __device__ inline
	void SetCached(bool cached=true)
	{ fCached = cached; }

	__host__ __device__ inline
	void RegistryParameters( std::initializer_list<Parameter*> var_list){

#ifndef __CUDA_ARCH__
		int i=0;
		for(Parameter* var: var_list)
		{
			//var->SetIndex(fParameterIndex +i+fParameterIndex);
			fParameters[i]=var;
			i++;
		}
		fParamResgistered=1;
#endif

	}


	void PrintRegisteredParameters()
	{
		if(!fParamResgistered){
			HYDRA_LOG(WARNING, "Parameters not registered, check client implementation. Nothing to dump. Exiting..." )
		return;
		}

		HYDRA_CALLER ;
		HYDRA_MSG <<HYDRA_ENDL;
		HYDRA_MSG << "Registered parameters begin:" << HYDRA_ENDL;
		for(size_t i=0; i<parameter_count; i++ )
		HYDRA_MSG <<"  >> Parameter " << i <<") "<< *fParameters[i] << HYDRA_ENDL;
		HYDRA_MSG <<"Registered parameters end." << HYDRA_ENDL;
		HYDRA_MSG <<HYDRA_ENDL;
		return;
	}

	__host__ inline
	void SetParameters(const std::vector<double>& parameters){

		if(fCached) return;
		if(!fParamResgistered){
					HYDRA_LOG(WARNING, "Parameters not registered, check client implementation. Nothing to dump. Exiting..." )
		            return;
				}
		for(size_t i=0; i< parameter_count; i++){
			*(fParameters[i])= parameters[fParameters[i]->GetIndex()];
		}

		if (INFO >= hydra::Print::Level()  )
		{
			std::ostringstream stringStream;
			for(size_t i=0; i< parameter_count; i++){
			     stringStream << "Parameter["<< fParameters[i]->GetIndex() <<"] :  " << parameters[fParameters[i]->GetIndex() ] << "  " << *fParameters[i] << "\n";
			}
			HYDRA_LOG(INFO, stringStream.str().c_str() )
		}

		return;
	}


	__host__  inline
	 const Parameter* GetParameter(size_t i) const {
		return fParameters[i];
	}


	__host__  inline
	Parameter* GetParameter(size_t i) {
		return fParameters[i];
	}



	template<typename T  >
	__host__ __device__ inline
	typename thrust::detail::enable_if< detail::is_homogeneous<
	typename thrust::tuple_element<0, typename std::remove_reference<T>::type>::type, typename std::remove_reference<T>::type>::value, return_type>::type
	interface(T&& x)
	{
		typedef typename std::remove_reference<T>::type Tprime;
		typedef typename std::remove_reference<typename thrust::tuple_element<0, Tprime>::type>::type first_type;
		constexpr size_t N = thrust::tuple_size< Tprime >::value;

		first_type Array[ N ];

		detail::tupleToArray(x, &Array[0] );

		return static_cast<Functor*>(this)->Evaluate(&Array[0]);


	}

	template<typename T >
	__host__ __device__ inline
	typename thrust::detail::enable_if<!detail::is_homogeneous<
	typename thrust::tuple_element<0, typename std::remove_reference<T>::type>::type, typename std::remove_reference<T>::type>::value, return_type>::type
	interface(T&& x)
	{

		return static_cast<Functor*>(this)->Evaluate(x);


	}



	template<typename T>
	__host__  __device__ inline
	return_type operator()( T* x, T* p=0  )
	{return static_cast<Functor*>(this)->Evaluate(x); }


	template<typename T>
	__host__ __device__ inline
	return_type operator()( T&&  x )
	{return interface< T>(std::forward< T >(x));}


	template<typename T1, typename T2>
	__host__ __device__  inline
	return_type operator()( T1&& x, T2 && cache)
	{

		return fCached ?\
				detail::extract<return_type, T2 >(fCacheIndex, std::forward<T2>(cache)):\
				operator()<T1>( std::forward<T1>(x) );
	}

protected:

   // mutable GReal_t fNorm;
	int  fCacheIndex;
	bool fCached;
	bool fParamResgistered;
	Parameter* fParameters[NPARAM];


};



}//namespace hydra


#endif /* FUNCTION_H_ */
