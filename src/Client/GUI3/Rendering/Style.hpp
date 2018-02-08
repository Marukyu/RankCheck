#ifndef SRC_CLIENT_GUI3_RENDERING_STYLE_HPP_
#define SRC_CLIENT_GUI3_RENDERING_STYLE_HPP_

#include <Shared/Utils/StaticKey.hpp>
#include <bitset>
#include <initializer_list>
#include <string>

namespace gui3
{

class Style
{
public:
	Style() = default;
	Style(std::string name);
	~Style() = default;

	std::string getName() const
	{
		return myKey.getName();
	}

	std::size_t getIndex() const
	{
		return myKey.getIndex();
	}

	static Style fromIndex(std::size_t index)
	{
		return Style(StaticKey<Style>::fromIndex(index));
	}

private:

	Style(StaticKey<Style> key) :
		myKey(std::move(key))
	{
	}

	StaticKey<Style> myKey;
};

class CombinedStyle
{
private:

	using Storage = std::bitset<64>;

public:

	class ConstIterator
	{
	public:
		ConstIterator();

		ConstIterator(ConstIterator && it) = default;
		ConstIterator(const ConstIterator & it) = default;

		ConstIterator & operator=(ConstIterator && it) = default;
		ConstIterator & operator=(const ConstIterator & it) = default;

		const Style & operator*() const;
		const Style * operator->() const;
		ConstIterator & operator++();
		ConstIterator operator++(int);
		bool operator==(const ConstIterator & it) const;
		bool operator!=(const ConstIterator & it) const;

	private:
		ConstIterator(const CombinedStyle * parent, std::size_t index);

		void select(std::size_t index);

		const CombinedStyle * myParent;
		Style myStyle;

		friend class CombinedStyle;
	};

	CombinedStyle() = default;

	CombinedStyle(CombinedStyle && style);
	CombinedStyle(const CombinedStyle & style);

	CombinedStyle & operator=(CombinedStyle && style);
	CombinedStyle & operator=(const CombinedStyle & style);

	CombinedStyle(Style style);
	CombinedStyle(std::initializer_list<Style> styles);

	~CombinedStyle() = default;

	void add(Style style);
	void add(CombinedStyle style);

	template<typename Iterator>
	void add(Iterator first, Iterator last)
	{
		for (auto it = first; it != last; ++it)
		{
			add(*it);
		}
	}

	ConstIterator begin() const;
	ConstIterator end() const;

private:

	Storage myStyles;
};

CombinedStyle operator+(Style style1, Style style2);
CombinedStyle operator+(Style style1, CombinedStyle style2);
CombinedStyle operator+(CombinedStyle style1, Style style2);
CombinedStyle operator+(CombinedStyle style1, CombinedStyle style2);

CombinedStyle & operator+=(CombinedStyle & style1, Style style2);
CombinedStyle & operator+=(CombinedStyle & style1, CombinedStyle style2);

}

#endif
