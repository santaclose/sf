// from https://stackoverflow.com/questions/4200224/random-noise-functions-for-glsl

// A single iteration of Bob Jenkins' One-At-A-Time hashing algorithm.
uint _hash( uint x ) {
	x += ( x << 10u );
	x ^= ( x >>  6u );
	x += ( x <<  3u );
	x ^= ( x >> 11u );
	x += ( x << 15u );
	return x;
}

// Compound versions of the hashing algorithm I whipped together.
uint _hash( uvec2 v ) { return _hash( v.x ^ _hash(v.y)                         ); }
uint _hash( uvec3 v ) { return _hash( v.x ^ _hash(v.y) ^ _hash(v.z)             ); }
uint _hash( uvec4 v ) { return _hash( v.x ^ _hash(v.y) ^ _hash(v.z) ^ _hash(v.w) ); }

// Construct a float with half-open range [0:1] using low 23 bits.
// All zeroes yields 0.0, all ones yields the next smallest representable value below 1.0.
float _floatConstruct( uint m ) {
	const uint ieeeMantissa = 0x007FFFFFu; // binary32 mantissa bitmask
	const uint ieeeOne      = 0x3F800000u; // 1.0 in IEEE binary32

	m &= ieeeMantissa;                     // Keep only mantissa bits (fractional part)
	m |= ieeeOne;                          // Add fractional part to 1.0

	float  f = uintBitsToFloat( m );       // Range [1:2]
	return f - 1.0;                        // Range [0:1]
}

// Pseudo-random value in half-open range [0:1].
float Random( float x ) { return _floatConstruct(_hash(floatBitsToUint(x))); }
float Random( vec2  v ) { return _floatConstruct(_hash(floatBitsToUint(v))); }
float Random( vec3  v ) { return _floatConstruct(_hash(floatBitsToUint(v))); }
float Random( vec4  v ) { return _floatConstruct(_hash(floatBitsToUint(v))); }