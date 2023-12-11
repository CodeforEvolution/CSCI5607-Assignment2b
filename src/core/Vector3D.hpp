// Assignment 2b - Learning About Viewing, Projection and Viewport Transformations via a First-Person 3D Walkthrough
// Work by Jacob Secunda
#ifndef VECTOR3D_H
#define VECTOR3D_H

#include <cmath>
#include <iostream>

template<typename T>
struct Vector3D {
	T dx;
	T dy;
	T dz;

public:
	Vector3D() = default;

	Vector3D(T dx, T dy, T dz)
			:
			dx(dx),
			dy(dy),
			dz(dz)
	{
	}

	// Description: Calculates the length of this vector.
	[[nodiscard]] inline T
	Length() const {
		return sqrt((dx * dx) + (dy * dy) + (dz * dz));
	}

	// Description: Converts this vector into a unit vector.
	// 	- This is a vector with length 1, though with the same direction.
	void
	NormalizeSelf() {
		T vectorLength = Length();
		// We can't divide by zero!!!
		if (vectorLength == 0) {
			// This isn't ideal...but let's not crash at least!
			return;
		}

		dx /= vectorLength;
		dy /= vectorLength;
		dz /= vectorLength;
	}

	// Description: Calculates the unit vector of this vector and returns it.
	// 	- This is a vector with length 1, though with the same direction.
	[[nodiscard]] Vector3D<T>
	Normalize() const {
		Vector3D<T> norm = *this;
		norm.NormalizeSelf();
		return norm;
	}

	// Description: Obtain angle between directions of this vector and 'v'.
	// 	- Note: Vectors are Orthogonal when Dot Product == 0.
	[[nodiscard]] T
	DotProduct(const Vector3D& v) const {
		return (dx * v.dx) + (dy * v.dy) + (dz * v.dz);
	}

	// Description: Finds the vector normal/perpendicular to this Vector 3D and 'v'.
	[[nodiscard]] Vector3D<T>
	CrossProduct(const Vector3D<T>& v) const {
		Vector3D normal{};

		normal.dx = (this->dy * v.dz) - (this->dz * v.dy);
		normal.dy = -1 * ((this->dx * v.dz) - (this->dz * v.dx));
		normal.dz = (this->dx * v.dy) - (this->dy * v.dx);

		return normal;
	}

	// Description: Add vectors together to create a vector connecting this vector and 'vector' together.
	constexpr Vector3D<T>&
	operator+=(const Vector3D<T>& vector) {
		dx += vector.dx;
		dy += vector.dy;
		dz += vector.dz;

		return *this;
	}

	constexpr Vector3D<T>&
	operator-=(const Vector3D<T>& vector) {
		dx -= vector.dx;
		dy -= vector.dy;
		dz -= vector.dz;

		return *this;
	}

	// Description: Lengthen/shorten this vector by 'scalar' amount.
	constexpr Vector3D<T>&
	operator*=(const T& scalar) {
		dx *= scalar;
		dy *= scalar;
		dz *= scalar;

		return *this;
	}

	// Description: Shorten/lengthen this vector by 'scalar' amount.
	constexpr Vector3D<T>&
	operator/=(const T& scalar) {
		dx /= scalar;
		dy /= scalar;
		dz /= scalar;

		return *this;
	}

	auto operator<=>(const Vector3D<T>& other) const = default;
};

/** Types */
using Vector3Di = Vector3D<int>;
using Vector3Df = Vector3D<float>;


/** Extra Operators */

template<typename T> static Vector3D<T>
operator*(const Vector3D<T>& vector, const T& scalar)
{
	Vector3D scaledVector = vector;
	scaledVector *= scalar;
	return scaledVector;
}

template<typename T> static Vector3D<T>
operator+(const Vector3D<T>& vectorA, const Vector3D<T>& vectorB)
{
	Vector3D<T> vectorC{};

	vectorC.dx = vectorA.dx + vectorB.dx;
	vectorC.dy = vectorA.dy + vectorB.dy;
	vectorC.dz = vectorA.dz + vectorB.dz;

	return vectorC;
}

template<typename T> static Vector3D<T>
operator-(const Vector3D<T>& vectorA, const Vector3D<T>& vectorB)
{
	Vector3D vectorC = vectorA;
	vectorC.dx -= vectorB.dx;
	vectorC.dy -= vectorB.dy;
	vectorC.dz -= vectorB.dz;
	return vectorC;
}


template<typename T> static std::ostream&
operator<<(std::ostream& out, const Vector3D<T>& vector) {
	out << "(dx: " << std::to_string(vector.dx) << ", dy: " << std::to_string(vector.dy) << ", dz: "
		<< std::to_string(vector.dz) << ")";
	return out;
}

#endif // VECTOR3D_H
