#pragma once
#include <iostream>
#include <string>


template <class T>
struct v2d_generic
{
	T x = 0;
	T y = 0;
	v2d_generic() : x(0), y(0) {}
	v2d_generic(T _x, T _y) : x(_x), y(_y) {}
	v2d_generic(const v2d_generic& v) : x(v.x), y(v.y) {}
	v2d_generic& operator=(const v2d_generic& v) = default;

	T mag() { return T(std::sqrt(x * x + y * y)); }
	T mag2() { return x * x + y * y; }
	v2d_generic  norm() {
		if (mag() == 0) return v2d_generic(0, 0);
		T r = 1 / mag();
		return v2d_generic(x * r, y * r);
	}

	v2d_generic  perp() { return v2d_generic(-y, x); }
	T dot(const v2d_generic& rhs) { return this->x * rhs.x + this->y * rhs.y; }
	T cross(const v2d_generic& rhs) { return this->x * rhs.y - this->y * rhs.x; }
	v2d_generic  operator +  (const v2d_generic& rhs) const { return v2d_generic(this->x + rhs.x, this->y + rhs.y); }
	v2d_generic  operator -  (const v2d_generic& rhs) const { return v2d_generic(this->x - rhs.x, this->y - rhs.y); }
	v2d_generic  operator *  (const T& rhs)           const { return v2d_generic(this->x * rhs, this->y * rhs); }
	v2d_generic  operator *  (const v2d_generic& rhs) const { return v2d_generic(this->x * rhs.x, this->y * rhs.y); }
	v2d_generic  operator /  (const T& rhs)           const { return v2d_generic(this->x / rhs, this->y / rhs); }
	v2d_generic  operator /  (const v2d_generic& rhs) const { return v2d_generic(this->x / rhs.x, this->y / rhs.y); }
	v2d_generic& operator += (const v2d_generic& rhs) { this->x += rhs.x; this->y += rhs.y; return *this; }
	v2d_generic& operator -= (const v2d_generic& rhs) { this->x -= rhs.x; this->y -= rhs.y; return *this; }
	v2d_generic& operator *= (const T& rhs) { this->x *= rhs; this->y *= rhs; return *this; }
	v2d_generic& operator /= (const T& rhs) { this->x /= rhs; this->y /= rhs; return *this; }
	v2d_generic  operator +  () const { return { +x, +y }; }
	v2d_generic  operator -  () const { return { -x, -y }; }
	bool operator == (const v2d_generic& rhs) const { return (this->x == rhs.x && this->y == rhs.y); }
	bool operator != (const v2d_generic& rhs) const { return (this->x != rhs.x || this->y != rhs.y); }
	const std::string str() const { return std::string("(") + std::to_string(this->x) + "," + std::to_string(this->y) + ")"; }
	friend std::ostream& operator << (std::ostream& os, const v2d_generic& rhs) { os << rhs.str(); return os; }
	operator v2d_generic<int32_t>() const { return { static_cast<int32_t>(this->x), static_cast<int32_t>(this->y) }; }
	operator v2d_generic<float>() const { return { static_cast<float>(this->x), static_cast<float>(this->y) }; }
	operator v2d_generic<double>() const { return { static_cast<double>(this->x), static_cast<double>(this->y) }; }
};

// Note: joshinils has some good suggestions here, but they are complicated to implement at this moment, 
// however they will appear in a future version of PGE
template<class T> inline v2d_generic<T> operator * (const float& lhs, const v2d_generic<T>& rhs)
{
	return v2d_generic<T>((T)(lhs * (float)rhs.x), (T)(lhs * (float)rhs.y));
}
template<class T> inline v2d_generic<T> operator * (const double& lhs, const v2d_generic<T>& rhs)
{
	return v2d_generic<T>((T)(lhs * (double)rhs.x), (T)(lhs * (double)rhs.y));
}
template<class T> inline v2d_generic<T> operator * (const int& lhs, const v2d_generic<T>& rhs)
{
	return v2d_generic<T>((T)(lhs * (int)rhs.x), (T)(lhs * (int)rhs.y));
}
template<class T> inline v2d_generic<T> operator / (const float& lhs, const v2d_generic<T>& rhs)
{
	return v2d_generic<T>((T)(lhs / (float)rhs.x), (T)(lhs / (float)rhs.y));
}
template<class T> inline v2d_generic<T> operator / (const double& lhs, const v2d_generic<T>& rhs)
{
	return v2d_generic<T>((T)(lhs / (double)rhs.x), (T)(lhs / (double)rhs.y));
}
template<class T> inline v2d_generic<T> operator / (const int& lhs, const v2d_generic<T>& rhs)
{
	return v2d_generic<T>((T)(lhs / (int)rhs.x), (T)(lhs / (int)rhs.y));
}

typedef v2d_generic<int32_t> vi2d;
typedef v2d_generic<uint32_t> vu2d;
typedef v2d_generic<float> vf2d;
typedef v2d_generic<double> vd2d;

