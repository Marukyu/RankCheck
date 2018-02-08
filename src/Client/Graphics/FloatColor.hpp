#ifndef SRC_CLIENT_GRAPHICS_FLOATCOLOR_HPP_
#define SRC_CLIENT_GRAPHICS_FLOATCOLOR_HPP_

namespace sf
{
class Color;
}

class FloatColor
{
public:

	FloatColor();
	FloatColor(float r, float g, float b, float a = 1.f);
	FloatColor(sf::Color color);

	operator sf::Color() const;

	static FloatColor interpolate(const FloatColor & color1, const FloatColor & color2, float factor);

	float r;
	float g;
	float b;
	float a;
};

FloatColor operator+(const FloatColor & color1, const FloatColor & color2);
FloatColor operator-(const FloatColor & color1, const FloatColor & color2);
FloatColor operator*(const FloatColor & color1, const FloatColor & color2);

FloatColor operator*(const FloatColor & color, float scalar);
FloatColor operator/(const FloatColor & color, float scalar);

FloatColor & operator*=(FloatColor & color, float scalar);
FloatColor & operator/=(FloatColor & color, float scalar);

FloatColor & operator+=(FloatColor & color1, const FloatColor & color2);
FloatColor & operator-=(FloatColor & color1, const FloatColor & color2);
FloatColor & operator*=(FloatColor & color1, const FloatColor & color2);

#endif
