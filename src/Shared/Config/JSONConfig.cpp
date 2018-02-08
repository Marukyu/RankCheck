#include <Shared/Config/DataTypes.hpp>
#include <Shared/Config/JSONConfig.hpp>
#include <Shared/External/RapidJSON/error/en.h>
#include <Shared/External/RapidJSON/error/error.h>
#include <Shared/External/RapidJSON/prettywriter.h>
#include <Shared/External/RapidJSON/rapidjson.h>
#include <Shared/External/RapidJSON/reader.h>
#include <Shared/External/RapidJSON/stringbuffer.h>
#include <Shared/External/RapidJSON/writer.h>
#include <Shared/Utils/Error.hpp>
#include <Shared/Utils/MakeUnique.hpp>
#include <Shared/Utils/StrNumCon.hpp>
#include <Shared/Utils/Utilities.hpp>
#include <cassert>
#include <stdexcept>

namespace cfg
{

const std::string JSONConfig::LENGTH_NODE = "length";
const std::string JSONConfig::LENGTH_SUFFIX = ".length";

JSONConfig::JSONConfig()
{
	document = makeUnique<rapidjson::Document>();
	document->SetObject();
}

JSONConfig::~JSONConfig()
{
}

void JSONConfig::loadFromMemory(const char* data, std::size_t size)
{
	// TODO: Optimize this.
	// Need to create a string to ensure trailing '\0'.
	loadFromString(std::string(data, data + size));
}

void JSONConfig::loadFromString(const std::string & json)
{
	document = makeUnique<rapidjson::Document>();
	document->SetObject();
	rapidjson::ParseResult result = document->Parse<rapidjson::kParseTrailingCommasFlag |
		rapidjson::kParseCommentsFlag>(json.c_str());

	if (result.IsError())
	{
		throw std::runtime_error(
			std::string("Error parsing JSON: ") + rapidjson::GetParseError_En(result.Code())
				+ " [Error location: character " + cNtoS(result.Offset()) + "]");
	}
}

std::string JSONConfig::saveToString(Style style) const
{
	if (document == nullptr)
	{
		return "";
	}

	rapidjson::StringBuffer buffer;

	switch (style)
	{
	case Style::Compact:
	default:
	{
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		document->Accept(writer);
	}
		break;
	case Style::Pretty:
	{
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
		document->Accept(writer);
	}
		break;
	}
	return buffer.GetString();
}

Value JSONConfig::readValue(std::string key) const
{
	std::size_t arrayLength;
	if (tryGetArrayLength(key, arrayLength))
	{
		return Value(Value::Type::Int, cNtoS(arrayLength));
	}

	const rapidjson::Value * node = findKey(key);

	if (node != nullptr)
	{
		return getNodeValue(*node);
	}
	else
	{
		return Value();
	}
}

void JSONConfig::writeValue(std::string key, Value value)
{
	if (document == nullptr)
	{
		return;
	}

	if (value.type == Value::Type::Int && trySetArrayLength(key, cStoUL(value.content)))
	{
		return;
	}

	rapidjson::Value & node = createOrFindKey(key);
	setNodeValue(node, std::move(value));
}

std::vector<JSONConfig::PathComponent> JSONConfig::getPathComponents(const std::string& key) const
{
	std::vector<PathComponent> components;

	std::vector<std::string> pathStrings;
	splitString(key, ".", pathStrings);

	for (std::string & pathString : pathStrings)
	{
		if (pathString.empty())
		{
			throw Error("Malformed config path \"" + key + "\" (expected key name)");
		}

		std::size_t openBracketIndex = pathString.find_first_of('[');

		if (openBracketIndex != std::string::npos)
		{
			std::string pathSubStr = pathString.substr(0, openBracketIndex);
			checkPathString(pathSubStr);
			components.emplace_back(std::move(pathSubStr));

			while (openBracketIndex != std::string::npos)
			{
				std::size_t closedBracketIndex = pathString.find_first_of(']', openBracketIndex + 1);

				if (closedBracketIndex == std::string::npos)
				{
					throw Error("Malformed config path \"" + key + "\" (expected ']')");
				}

				std::string bracketStr = pathString.substr(openBracketIndex + 1,
					closedBracketIndex - openBracketIndex - 1);
				unsigned int arrayIndex = 0;

				if (bracketStr.empty() || !str2Num(arrayIndex, bracketStr))
				{
					throw Error("Malformed config path \"" + key + "\" (expected array index)");
				}

				components.emplace_back(arrayIndex);
				openBracketIndex = pathString.find_first_of('[', closedBracketIndex);
			}
		}
		else
		{
			checkPathString(pathString);
			components.emplace_back(std::move(pathString));
		}
	}

	return components;
}

void JSONConfig::checkPathString(const std::string& pathString) const
{
	for (char ch : pathString)
	{
		if ((ch < 'a' || ch > 'z') && (ch < 'A' || ch > 'Z') && ch != '_')
		{
			throw Error(
				"Malformed config node name \"" + pathString + "\" (only A-Z, a-z and _ are allowed)");
		}
	}
}

const rapidjson::Value* JSONConfig::findKey(const std::string & key) const
{
	if (document == nullptr || !document->IsObject())
	{
		return nullptr;
	}

	const rapidjson::Value * currentNode = document.get();

	std::vector<PathComponent> pathComponents = getPathComponents(key);

	for (const PathComponent & component : pathComponents)
	{
		if (component.isArrayIndex())
		{
			if (!currentNode->IsArray() || currentNode->Size() <= component.getArrayIndex())
			{
				return nullptr;
			}

			currentNode = &((*currentNode)[component.getArrayIndex()]);
		}
		else
		{
			if (!currentNode->IsObject())
			{
				return nullptr;
			}

			auto result = currentNode->FindMember(component.getName().c_str());

			if (result == currentNode->MemberEnd())
			{
				return nullptr;
			}

			currentNode = &result->value;
		}
	}

	return currentNode;
}

rapidjson::Value & JSONConfig::createOrFindKey(const std::string & key)
{
	rapidjson::Value * currentNode = document.get();

	std::vector<PathComponent> pathComponents = getPathComponents(key);

	for (const PathComponent & component : pathComponents)
	{
		if (component.isArrayIndex())
		{
			if (!currentNode->IsArray())
			{
				currentNode->SetArray();
			}

			if (currentNode->Capacity() <= component.getArrayIndex())
			{
				currentNode->Reserve(component.getArrayIndex() + 1, document->GetAllocator());
			}

			while (currentNode->Size() <= component.getArrayIndex())
			{
				currentNode->PushBack(rapidjson::Value(), document->GetAllocator());
			}

			currentNode = &((*currentNode)[component.getArrayIndex()]);
		}
		else
		{
			if (!currentNode->IsObject())
			{
				currentNode->SetObject();
			}

			auto result = currentNode->FindMember(component.getName().c_str());

			if (result == currentNode->MemberEnd())
			{
				// Member does not exist, create it.
				currentNode->AddMember(rapidjson::Value(component.getName().c_str(), document->GetAllocator()),
					rapidjson::Value(rapidjson::kObjectType), document->GetAllocator());

				result = currentNode->FindMember(component.getName().c_str());

				assert(result != currentNode->MemberEnd());
			}

			currentNode = &result->value;
		}
	}

	return *currentNode;
}

bool JSONConfig::isArrayLengthKey(const std::string& key) const
{
	return key.size() >= LENGTH_SUFFIX.length()
		&& key.compare(key.length() - LENGTH_SUFFIX.length(), LENGTH_SUFFIX.length(), LENGTH_SUFFIX) == 0;
}

bool JSONConfig::tryGetArrayLength(const std::string & key, std::size_t& arrayLength) const
{
	if (isArrayLengthKey(key))
	{
		std::size_t lastDot = key.find_last_of('.');

		if (lastDot != std::string::npos)
		{
			const rapidjson::Value * node = findKey(key.substr(0, lastDot));

			if (node != nullptr && node->IsArray())
			{
				arrayLength = node->Size();
				return true;
			}
		}

	}

	return false;
}

bool JSONConfig::trySetArrayLength(const std::string & key, std::size_t arrayLength)
{
	if (isArrayLengthKey(key))
	{
		std::size_t lastDot = key.find_last_of('.');

		if (lastDot != std::string::npos)
		{
			rapidjson::Value & node = createOrFindKey(key.substr(0, lastDot));

			if (node.IsArray())
			{
				while (node.Size() > arrayLength)
				{
					node.PopBack();
				}

				node.Reserve(arrayLength, document->GetAllocator());

				while (node.Size() < arrayLength)
				{
					node.PushBack(rapidjson::Value(), document->GetAllocator());
				}

				return true;
			}
		}

	}

	return false;
}

void JSONConfig::setNodeValue(rapidjson::Value& node, Value value)
{
	switch (value.type)
	{
	case Value::Type::Bool:
		node.SetBool(value.content == "true");
		break;
	case Value::Type::Int:
		node.SetInt64(cStoL(value.content));
		break;
	case Value::Type::String:
		node.SetString(value.content.c_str(), document->GetAllocator());
		break;
	case Value::Type::Float:
		node.SetDouble(cStoD(value.content));
		break;
	case Value::Type::Missing:
	case Value::Type::Null:
	default:
		node.SetNull();
		break;
	}
}

Value JSONConfig::getNodeValue(const rapidjson::Value& node) const
{
	if (node.IsTrue())
	{
		return Value(Value::Type::Bool, "true");
	}
	else if (node.IsFalse())
	{
		return Value(Value::Type::Bool, "false");
	}
	else if (node.IsDouble())
	{
		return Value(Value::Type::Float, cNtoS(node.GetDouble()));
	}
	else if (node.IsInt64())
	{
		return Value(Value::Type::Int, cNtoS(node.GetInt64()));
	}
	else if (node.IsUint64())
	{
		return Value(Value::Type::Int, cNtoS(node.GetUint64()));
	}
	else if (node.IsString())
	{
		return Value(Value::Type::String, node.GetString());
	}

	return Value();
}

}
