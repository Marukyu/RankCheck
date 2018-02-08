#ifndef SRC_SHARED_CONFIG_DATATYPES_HPP_
#define SRC_SHARED_CONFIG_DATATYPES_HPP_

#include <SFML/Config.hpp>
#include <Shared/Utils/StaticKey.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <algorithm>
#include <cstddef>
#include <functional>
#include <string>
#include <unordered_map>

namespace cfg
{

struct Value
{
	enum class Type
	{
		Missing,
		Null,
		String,
		Bool,
		Int,
		Float
	};

	Value() :
		type(Type::Missing),
		content()
	{
	}

	Value(Type type, std::string content) :
		type(type),
		content(content)
	{
	}

	Type type;
	std::string content;
};

class StringKey : public StaticKey<StringKey>
{
public:
	using DataType = std::string;

	inline static DataType convertTo(Value value)
	{
		return std::move(value.content);
	}

	inline static Value convertFrom(DataType value)
	{
		return Value(Value::Type::String, std::move(value));
	}

	StringKey(std::string key) :
		StaticKey<StringKey>(std::move(key))
	{
	}
};

class BoolKey : public StaticKey<BoolKey>
{
public:
	using DataType = bool;

	static constexpr const char * TRUE_VALUE = "true";
	static constexpr const char * FALSE_VALUE = "false";

	inline static DataType convertTo(Value value)
	{
		if (value.content == TRUE_VALUE)
		{
			return true;
		}
		else if (value.content.empty() || value.content == FALSE_VALUE)
		{
			return false;
		}
		else
		{
			return cStoI(value.content) != 0;
		}
	}

	inline static Value convertFrom(DataType value)
	{
		return Value(Value::Type::Bool, value ? TRUE_VALUE : FALSE_VALUE);
	}

	BoolKey(std::string key) :
		StaticKey<BoolKey>(std::move(key))
	{
	}
};

class IntKey : public StaticKey<IntKey>
{
public:
	using DataType = sf::Int64;

	inline static DataType convertTo(Value value)
	{
		return cStoN<DataType>(value.content);
	}

	inline static Value convertFrom(DataType value)
	{
		return Value(Value::Type::Int, cNtoS(value));
	}

	IntKey(std::string key) :
		StaticKey<IntKey>(std::move(key))
	{
	}
};

class FloatKey : public StaticKey<FloatKey>
{
public:
	using DataType = float;

	inline static DataType convertTo(Value value)
	{
		return cStoN<DataType>(value.content);
	}

	inline static Value convertFrom(DataType value)
	{
		return Value(Value::Type::Float, cNtoS(value));
	}

	FloatKey(std::string key) :
		StaticKey<FloatKey>(std::move(key))
	{
	}
};

}

#endif
