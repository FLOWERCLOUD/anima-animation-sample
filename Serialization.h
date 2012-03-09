#pragma once
#include "stdafx.h" 

namespace Serialization
{
#ifdef DEBUG
	static const int STREAM_VERSION = 101;
#else
	static const int STREAM_VERSION = 100;
#endif

	static const int STREAM_MAGIC = 0xDEADBEEF;

	struct StreamSerializable
	{
		virtual bool ToStream( std::ostream& stream ) = 0;
		virtual bool FromStream( std::istream& stream ) = 0;
	};

	template< typename T >
	void ToStream( const std::vector<T>& data, std::ostream& stream )
	{
		size_t elementCount = data.size();
		stream.write( reinterpret_cast<const char*>( &elementCount ), sizeof(elementCount) );
		stream.write( reinterpret_cast<const char*>( &data[0] ), data.size() * sizeof(T) );
	}

	template< typename T >
	void FromStream( std::istream& stream, std::vector<T>& data )
	{
		size_t elementCount;
		stream.read( reinterpret_cast<char*>( &elementCount ), sizeof(size_t) );
		
		data.resize( elementCount );
		stream.read( reinterpret_cast<char*>( &data[0] ), data.size() * sizeof(T) );
	}

	template< typename T >
	void ToStream( const T& data, std::ostream& stream )
	{
		stream.write( reinterpret_cast<const char*>( &data ), sizeof(T) );
	}

	template< typename T >
	void FromStream( std::istream& stream, T& data  )
	{
		stream.read( reinterpret_cast<char*>( &data ), sizeof(T) );
	}

	template< typename T>
	T FromStream( std::istream& stream )
	{
		T value;
		FromStream( stream, value );
		return value;
	}
}
