#include "vl.h"

namespace ofv
{
	namespace vl
	{
		vec::vec()
		{
			x = 0;
			y = 0;
			z = 0;
		}
		vec::vec(const float x, const float y, const float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		vec vec::Cross(const vec& other)
		{
			// i j k
			// x y z
			// o p q

			// iyq + kxp + jzo - izp - jxq - kyo

			vec res;
			res.x = y * other.z - z * other.y;
			res.y = z * other.x - x * other.z;
			res.z = x * other.y - y * other.x;
			return res;
		}
		float vec::Dot(const vec& other)
		{
			return x * other.x + y * other.y + z * other.z;
		}
		float vec::Magnitude()
		{
			return sqrt(x * x + y * y + z * z);
		}
		vec vec::Normalized()
		{
			vec res;
			float mag = Magnitude();
			res.x = x / mag;
			res.y = y / mag;
			res.z = z / mag;
			return res;
		}

		void vec::Normalize()
		{
			float mag = Magnitude();
			x /= mag;
			y /= mag;
			z /= mag;
		}

		const vec vec::forward = vec(0, 0, -1);
		const vec vec::up = vec(0, 1, 0);
		const vec vec::right = vec(1, 0, 0);
		const vec vec::zero = vec(0, 0, 0);

		vec operator+(const vec& a, const vec& b)
		{
			vec res(a.x + b.x, a.y + b.y, a.z + b.z);
			return res;
		}

		void operator+=(vec& a, const vec& b)
		{
			a.x += b.x;
			a.y += b.y;
			a.z += b.z;
		}

		vec operator-(const vec& a, const vec& b)
		{
			vec res(a.x - b.x, a.y - b.y, a.z - b.z);
			return res;
		}
		void operator-=(vec& a, const vec& b)
		{
			a.x -= b.x;
			a.y -= b.y;
			a.z -= b.z;
		}

		vec operator*(const vec& a, const float& factor)
		{
			vec res(a.x * factor, a.y * factor, a.z * factor);
			return res;
		}

		vec operator*(const vec& a, const vec& b)
		{
			vec res(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
			return res;
		}

		vec operator-(const vec& theV)
		{
			vec res(theV.x * -1, theV.y * -1, theV.z * -1);
			return res;
		}
	}
}