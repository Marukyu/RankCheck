#include <Shared/Config/DataTypes.hpp>
#include <Shared/Config/NullConfig.hpp>

namespace cfg
{

NullConfig::NullConfig()
{
}

NullConfig::~NullConfig()
{
}

Value NullConfig::readValue(std::string key) const
{
	return Value(Value::Type::Null, "");
}

void NullConfig::writeValue(std::string key, Value value)
{
}

}
