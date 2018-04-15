#include <Client/GUI3/Rendering/Style.hpp>
#include <cassert>
#include <utility>

namespace gui3
{

Style::Style(std::string name) :
	myKey(std::move(name))
{
}

CombinedStyle::ConstIterator::ConstIterator() :
	myParent(nullptr)
{
}

const Style & CombinedStyle::ConstIterator::operator*() const
{
	return myStyle;
}

const Style * CombinedStyle::ConstIterator::operator->() const
{
	return &myStyle;
}

CombinedStyle::ConstIterator & CombinedStyle::ConstIterator::operator++()
{
	for (std::size_t i = myStyle.getIndex() + 1; i < myParent->myStyles.size(); ++i)
	{
		if (myParent->myStyles[i])
		{
			myStyle = Style::fromIndex(i);
			return *this;
		}
	}

	myParent = nullptr;
	return *this;
}

CombinedStyle::ConstIterator CombinedStyle::ConstIterator::operator++(int)
{
	CombinedStyle::ConstIterator copy = *this;
	++(*this);
	return copy;
}

bool CombinedStyle::ConstIterator::operator==(const ConstIterator & it) const
{
	return myParent == it.myParent && myStyle.getIndex() == it.myStyle.getIndex();
}

bool CombinedStyle::ConstIterator::operator!=(const ConstIterator & it) const
{
	return !(*this == it);
}

CombinedStyle::ConstIterator::ConstIterator(const CombinedStyle * parent, std::size_t index) :
	myParent(parent)
{
	select(index);
}

void CombinedStyle::ConstIterator::select(std::size_t index)
{
	myStyle = Style::fromIndex(index);
}

CombinedStyle::CombinedStyle(CombinedStyle && style) :
	myStyles(std::move(style.myStyles))
{
}

CombinedStyle::CombinedStyle(const CombinedStyle & style) :
	myStyles(style.myStyles)
{
}

CombinedStyle & CombinedStyle::operator=(CombinedStyle && style)
{
	myStyles = style.myStyles;
	return *this;
}

CombinedStyle & CombinedStyle::operator=(const CombinedStyle & style)
{
	myStyles = style.myStyles;
	return *this;
}

CombinedStyle::CombinedStyle(Style style)
{
	add(style);
}

CombinedStyle::CombinedStyle(std::initializer_list<Style> styles)
{
	add(styles);
}

void CombinedStyle::add(Style style)
{
	assert(style.getIndex() < myStyles.size());
	myStyles.set(style.getIndex());
}

void CombinedStyle::add(CombinedStyle style)
{
	myStyles |= style.myStyles;
}

CombinedStyle::ConstIterator CombinedStyle::begin() const
{
	ConstIterator it(this, 0);

	if (!myStyles.test(0))
	{
		++it;
	}

	return it;
}

CombinedStyle::ConstIterator CombinedStyle::end() const
{
	return ConstIterator();
}

CombinedStyle operator+(Style style1, Style style2)
{
	return CombinedStyle { style1, style2 };
}

CombinedStyle operator+(Style style1, CombinedStyle style2)
{
	CombinedStyle resultStyle(style1);
	resultStyle.add(std::move(style2));
	return resultStyle;
}

CombinedStyle operator+(CombinedStyle style1, Style style2)
{
	style1.add(style2);
	return style1;
}

CombinedStyle operator+(CombinedStyle style1, CombinedStyle style2)
{
	style1.add(std::move(style2));
	return style1;
}

CombinedStyle & operator+=(CombinedStyle & style1, Style style2)
{
	return style1 = style1 + std::move(style2);
}

CombinedStyle & operator+=(CombinedStyle & style1, CombinedStyle style2)
{
	return style1 = style1 + std::move(style2);
}

}
