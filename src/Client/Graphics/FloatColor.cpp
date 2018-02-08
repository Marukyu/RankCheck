#include <Client/Graphics/FloatColor.hpp>
#include <SFML/Graphics/Color.hpp>
#include <Shared/Utils/MiscMath.hpp>

FloatColor::FloatColor() :
	r(1),
	g(1),
	b(1),
	a(1)
{
}

FloatColor::FloatColor(sf::Color color) :
	r(color.r / 255.f),
	g(color.g / 255.f),
	b(color.b / 255.f),
	a(color.a / 255.f)
{
}

FloatColor::FloatColor(float r, float g, float b, float a) :
	r(r),
	g(g),
	b(b),
	a(a)
{
}

FloatColor::operator sf::Color() const
{
	int cr = clamp<int>(0, r * 255, 255);
	int cg = clamp<int>(0, g * 255, 255);
	int cb = clamp<int>(0, b * 255, 255);
	int ca = clamp<int>(0, a * 255, 255);
	return sf::Color(cr, cg, cb, ca);
}

FloatColor FloatColor::interpolate(const FloatColor& color1, const FloatColor& color2, float factor)
{
	return color1 * (1.f - factor) + color2 * factor;
}

FloatColor operator +(const FloatColor& color1, const FloatColor& color2)
{
	return FloatColor(color1.r + color2.r, color1.g + color2.g, color1.b + color2.b, color1.a + color2.a);
}

FloatColor operator -(const FloatColor& color1, const FloatColor& color2)
{
	return FloatColor(color1.r - color2.r, color1.g - color2.g, color1.b - color2.b, color1.a - color2.a);
}

FloatColor operator *(const FloatColor& color1, const FloatColor& color2)
{
	return FloatColor(color1.r * color2.r, color1.g * color2.g, color1.b * color2.b, color1.a * color2.a);
}

FloatColor operator *(const FloatColor& color, float scalar)
{
	return FloatColor(color.r * scalar, color.g * scalar, color.b * scalar, color.a * scalar);
}

FloatColor operator /(const FloatColor& color, float scalar)
{
	return FloatColor(color.r / scalar, color.g / scalar, color.b / scalar, color.a / scalar);
}

FloatColor& operator +=(FloatColor& color1, const FloatColor& color2)
{
	return color1 = color1 + color2;
}

FloatColor& operator -=(FloatColor& color1, const FloatColor& color2)
{
	return color1 = color1 - color2;
}

FloatColor& operator *=(FloatColor& color1, const FloatColor& color2)
{
	return color1 = color1 * color2;
}

FloatColor& operator *=(FloatColor& color, float scalar)
{
	return color = color * scalar;
}

FloatColor& operator /=(FloatColor& color, float scalar)
{
	return color = color / scalar;
}
