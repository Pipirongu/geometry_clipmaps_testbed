#pragma once

#include <cmath> 

/**
@class Vector2

Vector2 class
*/
class Vector2
{
public:
	Vector2(void);
	/// A Vector of size 2 with x, y values
	Vector2(float x, float y);
	Vector2(float s);
	~Vector2(void);

	/// Set x, y values of the Vector
	void Insert(float x, float y);

	/// Vector2+=Vector2. Addition is affecting the left Vector
	void operator+=(const Vector2& rhs);
	/// Vector2+Vector2
	Vector2 operator+(const Vector2& rhs)const;

	/// Vector2-=Vector2. Substraction is affecting the left Vector
	void operator-=(const Vector2& rhs);
	/// Vector2-Vector2
	Vector2 operator-(const Vector2& rhs)const;

	/// Vector2*=number. Multiplication is affecting the left Vector
	void operator*=(const float& number);
	/// Vector2*number
	Vector2 operator*(const float& number)const;

	/// Vector2/=number. Division is affecting the left Vector
	void operator/=(const float& number);
	/// Vector2/number
	Vector2 operator/(const float& number)const;

	//bitwise operators
	Vector2 operator/(const Vector2& b)const;
	Vector2 operator*(const Vector2& b)const;
	void operator*=(const Vector2& b);

	/// number*Vector2
	friend Vector2 operator*(const float& number, const Vector2& rhs);

	/// Compares two vectors if they are the same
	bool operator==(const Vector2& rhs)const;

	/// Returns the dot product of this vector
	static float Dot(const Vector2& lhs, const Vector2& rhs);
	/// Returns the length of this vector
	float Magnitude();
	/// Normalize this vector
	void Normalize();
	/// Return the normalized vector
	Vector2 Normalized();
	static Vector2 vec_fract(const Vector2& vec);
	static Vector2 vec_floor(const Vector2& vec);

	/// Overloaded access operator for Vector
	float& operator[](unsigned int index);
	const float& operator[](unsigned int index) const;

	union
	{
		struct
		{
			float x, y;
		};
		// Allow us to use vec2, vec3 and vec4 directly in uniform buffers.
		// GLSL using std140 packing demands that packing of vectors is four floats.
		// The rules on packing with arrays can be slightly complicated howewer.
		float data[2];
	};
};