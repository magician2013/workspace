/*
 * basic_op.c
 *
 *  Created on: 2015�~3��23��
 *      Author: ite01527
 */
#include "type_def.h"
#include "basic_op.h"

UWord32 uexp_table[32];
UWord32 udiv_table[64];
UWord32 ursqrt_table[96];
UWord32 udiv_128_64_tab[128] = { 255, 251, 247, 243, 239, 235, 232, 228, 225,
		221, 218, 214, 211, 208, 204, 201, 198, 195, 192, 189, 186, 183, 180,
		177, 174, 171, 169, 166, 163, 161, 158, 155, 153, 150, 148, 145, 143,
		140, 138, 136, 133, 131, 129, 127, 124, 122, 120, 118, 116, 114, 112,
		110, 108, 106, 104, 102, 100, 98, 96, 94, 92, 90, 89, 87, 85, 83, 81,
		80, 78, 76, 75, 73, 71, 70, 68, 67, 65, 63, 62, 60, 59, 57, 56, 54, 53,
		51, 50, 49, 47, 46, 44, 43, 42, 40, 39, 38, 36, 35, 34, 33, 31, 30, 29,
		28, 26, 25, 24, 23, 22, 20, 19, 18, 17, 16, 15, 14, 13, 11, 10, 9, 8, 7,
		6, 5, 4, 3, 2, 1 };

Word64 MAC_HI_LO = 0;

void init_uexp() {
	Word32 i;
	for (i = 0; i < 32; i += 1) {
		uexp_table[i] = (Word64) (powf(2.0, 31.0) * powf(2.0, (i + 0.5) / 32));
	}
}

void init_udiv() {
	Word32 i;
	for (i = 0; i < 64; i += 1) {
		udiv_table[i] = powf(2.0, 14.0) / (i + 64);
	}
}

void init_ursqrt() {
	Word32 i;
	for (i = 0; i < 96; i += 1) {
		ursqrt_table[i] = 256 / sqrtf((i + 32.3) / 128.0) - 256;
	}
}

void circshift(Word32 a[], Word32 m, Word32 size) { // size < 32
	Word32 temp[32];
	Word32 i;
	memmove(temp, a, sizeof(Word32) * size);

	for (i = 0; i < size; i += 1) {
		a[i] = (i - m < 0) ? temp[i - m + size] : temp[i - m];
	}
}

void swap_data(Complex16_t *a, Complex16_t *b) {
#if 0
	if(a[0].real != b[0].real) {
		a[0].real = a[0].real ^ b[0].real;
		b[0].real = a[0].real ^ b[0].real;
		a[0].real = b[0].real ^ a[0].real;
	}
	if(a[0].imag != b[0].imag) {
		a[0].imag = a[0].imag ^ b[0].imag;
		b[0].imag = a[0].imag ^ b[0].imag;
		a[0].imag = b[0].imag ^ a[0].imag;
	}
#else
	ComplexInt16 temp;

	temp = b[0];
	b[0] = a[0];
	a[0] = temp;

#endif
}

Word32 ulog2(UWord64 a) {
	Word32 se = 0;
	se = CLZ((a >> 32) & 0xffffffff);
	se += (se == 32) ? CLZ(a & 0xffffffff) : 0;
	return ((a == 0) ? 0 : 63 - se);
}

UWord32 udiv_32_32(UWord32 d, UWord32 n) { // 2^32 * d^-1, Q32

#if 0
#if 1
	UWord32 q = d;
	UWord32 r = n; // (n/d) * 2^32
	Word32 s = 0;
	Word32 m;
	Word32 a;
	Word32 t;

	s = CLZ(q);
	a = q << s;
	a = udiv_table[((UWord32) a >> 25) - 64];
	s -= 7L;// (d * 2^s * 2^-25)^-1 * 2^14
	m = -q;
	if (s >= 0) {
		q = a << s; // 2^32 / d
		a = (Word32) q * m;// q0 * (-d) = 2^32 - q0 * d
		q += ((Word64) a * (q >> 16) >> 16);// (2^32-q0*d)*q0/2^32
		if (m != m >> 1) { // d == 0 || d == 1 ??
			a = (Word32) q * m;// 2nd round
			s = 0;// (2^32-q0*d)
			q += (Word64) a * q >> 32;
			q = (UWord64) r * q >> 32;
			r += m;// r -= d
			r += (Word32) q * m;// r -= (q+1) * d
			t = r - d;// t = r - (q+2)
			if (t < -d) {
				r += d;
			} else if ((-d <= t) && (t < 0)) { // (r >= 0)
				q += 1;
			} else {
				r -= 2 * d;
				q += 2;
			}
			return (q);
		} else {
			if (d == 1) {
				return (r);
			} else {
				return (-1);
			}
		}
	} else {
		a -= 4;
		s = 0 - s; // (2^39 * 2^-s * d^-1 - 4) * 2^(s - 7) = 2^32 * d^-1
		q = (UWord32) a >> s;
		q = ((Word64) r * q + 0x80000000) >> 32;// n * q >> 32 = n / d
		r += (Word32) q * m;// n -= q0 * d
		if (r / 2 >= d) {
			r -= 2 * d;
			q += 2;
		}
		if (r >= d) {
			r -= d;
			q += 1;
		}
		return (q);
	}
#else
	UWord32 q;

	asm volatile("l.divu %0, %1, %2" :"=r"(q) :"r"(n), "r"(d));
	return (q);
#endif

#else
	return ((d == 0) ? 0x7fffffff : n / d);
#endif
}

Word32 sdiv_32_32(Word32 d, Word32 n) {
#if 1
	Word32 q = d;
	Word32 r = n;
	Word32 sign;

	sign = d & (1ul << 31);
	if (d < 0) {
		d = 0 - d;
	}
	sign ^= r >> 31; // 0x80000000 ^ 0xffffffff = 7fffffff
	if (r < 0) {
		r = 0 - r;
	}
	q = udiv_32_32(d, r);
	if (sign & 0x80000000) {
		q = 0 - q;
	}
	return (q);
#else
	Word64 q;

	asm volatile("l.div %0, %1, %2" :"=r"(q) :"r"(n), "r"(d));
	return (q);
#endif
}

UWord64 udiv_64_64(UWord64 d, UWord64 n) {
#if 0
	UWord64 q = d; // (1/d) * n
	UWord64 r = n;
	UWRD128 temp;
	Word64 a;
	Word64 m;
	Word64 t;
	Word32 a1, a0;
	Word32 s;

	a0 = q & 0xffffffff;
	a1 = (q >> 32) & 0xffffffff;
	s = CLZ(a1);
	s += (s == 32) ? CLZ(a0) : 0;
	a = q << s;// norm
	a = udiv_table[((UWord64) a >> 57) - 64];// 2^14 / (i+64)
	s -= 7;
	m = -q;// m = 2^64 - d;
	if (s >= 0) {
		q = a << s; // 2^64 / d = xn
		a = (Word64) q * m;// (-d * q0), (2^64 - d * q0) * q0 * 2^-64 = q0 - d * q0^2 * 2^-64
		temp = smul_64_128(a, q >> 32);// 32 * 64
		q += (((UWord64) temp.a2 << 32) + temp.a1);
		if (m != m >> 1) {
			a = (Word64) q * m;
			temp = smul_64_128(a, q);
			q += (((UWord64) temp.a3 << 32) + temp.a2);
			temp = umul_64_128(r, q);
			q = ((UWord64) temp.a3 << 32) + temp.a2;
			q += (temp.a1 & 0x80000000) ? 1 : 0;
			r += m; // r = n-d
			r += (Word64) q * m;// r = n - (q+1)*d;
			t = r - d;
			if (t < -d) { // n - (q+1)*d < 0
				r -= m;// n - q*d
			} else if (-d <= t && t < 0) {
				q += 1;
			} else {
				q += 2;
				r += 2 * m;
			}
		} else { // udiv_by_0_or_1
			if (m == 0) {
				q = -1;
			} else {
				q = r;
			}
		}
	} else { // large d
		a -= 4;// (2^s * d * 2^-57)^-1 * 2^14 * 2^(s-7) = 2^64/d
		s = -s;
		q = a >> s;
		temp = umul_64_128(r, q);
		q = ((UWord64) temp.a3 << 32) + temp.a2;
		r += (Word64) q * m;// r = n-q*d
		if (r / 2 >= d) {
			r += 2 * m;
			q += 2;
		}
		if (r >= d) {
			r += m;
			q += 1;
		}
	}
	return (q);
#else
	return ((d == 0) ? LLONG_MAX : (n / d));
#endif
}

Word64 sdiv_64_64(Word64 d, Word64 n) {
	Word64 q = d;
	Word64 r = n;
	Word64 sign;
	sign = d & (1ull << 63);
	d = (d < 0) ? -d : d;
	sign ^= r >> 63;
	r = (r < 0) ? -r : r;
	q = udiv_64_64(d, r);
	q = (sign & (UWord64) 0x8000000000000000) ? -q : q;
	return (q);
}

UWord32 udiv_64_32(UWord32 d, UWord32 n) {
#if 0
	UWRD128 tmp;
	UWord64 temp;
	UWord64 t;
	UWord32 q;
	UWord32 a;
	Word32 s;
	if (n < d) {
		s = CLZ(d);
		n <<= s;
		d <<= s;
		q = udiv_128_64_tab[(d >> 24) - 128];
		q += 256; // max = 2^9
		a = q * q;// Q16, q0.^2, max = 2^18
		temp = (UWord64) d * a;// Q32 * Q16, 2^18 * 2^32 = 2^50
		a = temp >> 32 & 0xffffffff;// Q16, max = 2^18
		q = (q << 9) - a;// Q16, 2^18-2^18
		t = (UWord64) q * q;// Q32, 2^36
		t >>= 1;// Q31
		tmp = umul_64_128(t, d);// 2^35 * 2^32
		temp = tmp.a1 + ((UWord64) tmp.a2 << 32);// 2^67/2^32
		t = ((UWord64) q << 16) - temp;// Q31
		temp = n * t >> 32;// Q31 * Q32
		q = temp >> 16;
		return (q);
	} else {
		return (0x7fff);
	}
#else
	return ((n < d) ? ((Word64) n << 15) / d : 0x7fff);

#endif
}

UWord64 udiv_128_64(UWord64 d, UWord64 n) { // return Q15 format of n*(d)^-1
	UWRD128 temp;
	UWord64 t;
	UWord64 q;
	Word64 a;
	Word64 s;
	if (n < d) { // n <= d
		s = CLZ(d >> 32 & 0xffffffff); // upper-case
		s += (s == 32) ? CLZ(d & 0xffffffff) : 0;
		n <<= s; // r <<= s
		d <<= s; // norm(d), 0x8000 ~ 0xffff, 2^64-1 ~ 2^63, 1 ~ 0.5, Q64
		q = udiv_128_64_tab[(d >> 56) - 128]; // table[a] = round(256.0/c) - 256
		q += 256; // x0 = Q8 format approximation, c = (128.5+a)*2^-8
		a = (Word32) q * (Word32) q; // Q16, (128+a)*2^-8<d<=(129+a)*2^-8
		temp = umul_64_128(d, a); // Q80, q0^2 * d
		a = temp.a2 + ((UWord64) temp.a3 << 32); // Q16
		q = (q << 9) - a; // Q16
		t = q * q; // Q32
		t >>= 1; // Q31
		temp = umul_64_128(d, t); // Q31 * Q64
		a = temp.a2 + ((UWord64) temp.a3 << 32); // Q31
		q = (q << 16) - a; // Q31, 1/d
		temp = umul_64_128(n, q); // Q31 * Q64
		q = (temp.a2 + ((UWord64) temp.a3 << 32)) >> 16; // Q15
		return (q);
	} else {
		return (0x7fff);
	}
}

UWRD128 umul_64_128(UWord64 b, UWord64 c) {
	UWord32 c1, c0;
	UWord32 b1, b0;
	UWord32 carry;
	UWord64 temp;
	UWRD128 a;

	b0 = b & 0xffffffff;
	b1 = b >> 32 & 0xffffffff;
	c0 = c & 0xffffffff;
	c1 = c >> 32 & 0xffffffff;
	temp = (UWord64) b0 * c0;
	a.a0 = temp & 0xffffffff;
	a.a1 = temp >> 32 & 0xffffffff;
	temp = (UWord64) b0 * c1;
	a.a2 = temp & 0xffffffff;
	a.a3 = temp >> 32 & 0xffffffff;
	temp = (UWord64) b1 * c1;
	c1 = temp & 0xffffffff;
	b0 = temp >> 32 & 0xffffffff;
	temp = (UWord64) a.a1 + a.a2;
	a.a1 = temp & 0xffffffff;
	carry = temp >> 32;
	temp = (UWord64) a.a3 + c1 + carry;
	a.a2 = temp & 0xffffffff;
	carry = temp >> 32;
	a.a3 = b0 + carry;
	temp = (UWord64) b1 * c0;
	c0 = temp & 0xffffffff;
	b0 = temp >> 32;
	temp = (UWord64) a.a1 + c0;
	a.a1 = temp & 0xffffffff;
	carry = temp >> 32;
	temp = (UWord64) a.a2 + b0 + carry;
	a.a2 = temp & 0xffffffff;
	carry = temp >> 32;
	a.a3 += carry;

	return (a);
}

UWRD128 smul_64_128(Word64 b, Word64 c) {
	Word32 c1, b1;
	UWord32 c0, b0; // 0~2^32-1 * 0~2^32-1 ~= 2^64-2^33+1
	UWord32 carry;
	UWord64 acc;
	UWord64 temp;
	UWRD128 a;
	b0 = b & 0xffffffff; // {c1, c0} * {b1, b0}
	b1 = b >> 32 & 0xffffffff; // unsigned int & long long
	c0 = c & 0xffffffff;
	c1 = c >> 32 & 0xffffffff;
// even negative equals unsigned mult
	temp = (Word64) b0 * c0; // word64->uword64, ull_max+1
	a.a0 = temp & 0xffffffff; // long long & unsigned int
	a.a1 = (temp >> 32) & 0xffffffff;
	acc = a.a1; // unsigned->Word64
	temp = ((Word32) b0) * (UWord64) c1; // usigned * signed
	acc += temp; // UWord64 + UWord64
	a.a1 = acc & 0xffffffff;
	a.a2 = (acc >> 32) & 0xffffffff; // unsigned long long

	if (b0 & 0x80000000) {
		a.a2 += c1; // Word32
	}
	acc = a.a1;
	temp = (Word64) b1 * ((Word32) c0);
	acc += temp;
	a.a1 = acc & 0xffffffff;
	a.a3 = (acc >> 32) & 0xffffffff;

	if (c0 & 0x80000000) {
		a.a3 += b1;
	}
	b0 = (Word32) a.a2 >> 31;
	temp = (UWord64) a.a2 + a.a3;
	a.a2 = temp & 0xffffffff;
	carry = (temp >> 32) & 0xffffffff;
	temp = b0 + ((Word32) a.a3 >> 31) + carry;
	acc = (Word64) a.a2 + ((Word64) temp << 32);
	acc += (UWord64) b1 * c1;
	a.a2 = acc & 0xffffffff;
	a.a3 = (acc >> 32) & 0xffffffff;
	return (a);
}

Word64 sat_128_64(UWRD128 a) {
	UWord64 b0;
	Word64 b1;
	Word64 c0, c1;
	b0 = a.a0 + ((UWord64) a.a1 << 32); // low 64 bits
	b1 = a.a2 + ((UWord64) a.a3 << 32); // high 64 bits, 64'b1 or 64'b0

	c0 = ((UWord64) b0) >> 63;
	c0 ^= ((b1 & 0x7fffffffffffffffLL) << 1);
	c1 = b1 >> 63;

	if (c1 != c0) {
		return ((!c1) ? LLONG_MAX : LLONG_MIN);
	} else {
		return (b0);
	}
}

UWRD128 uadd_128(UWRD128 a, UWRD128 b) {
	UWord32 carry;
	UWord64 temp;
	temp = (UWord64) a.a0 + b.a0;
	a.a0 = temp & 0xffffffff;
	carry = temp >> 32;
	temp = (UWord64) a.a1 + b.a1 + carry;
	a.a1 = temp & 0xffffffff;
	carry = temp >> 32;
	temp = (UWord64) a.a2 + b.a2 + carry;
	a.a2 = temp & 0xffffffff;
	carry = temp >> 32;
	a.a3 += b.a3 + carry;
	return (a);
}

UWRD128 usub_128(UWRD128 a, UWRD128 b) {
	UWord64 c0, c1;
	UWRD128 temp;

	c0 = b.a0 + ((UWord64) b.a1 << 32);
	c1 = b.a2 + ((UWord64) b.a3 << 32);
//c0 = ~c0;
//c1 = ~c1;

	b.a0 = ~c0 & 0xffffffff;
	b.a1 = ~c0 >> 32 & 0xffffffff;
	b.a2 = ~c1 & 0xffffffff;
	b.a3 = ~c1 >> 32 & 0xffffffff;

	temp.a0 = 1;
	temp.a1 = 0;
	temp.a2 = 0;
	temp.a3 = 0;
	temp = uadd_128(temp, b);
	temp = uadd_128(temp, a);

	return (temp);
}

UWord32 uexp_32(Word32 n) { // Q26
	UWord32 d; // fract part
	UWord32 q;
	UWord32 r;

	n = (Word64) n * 0x5c551d94 >> 30;
	d = n << 6; // Q32
	q = d >> 27; // index
	d = d - (q << 27); // Q32, delta
	d = (Word64) d * 0xb17217f8 >> 32; // * ln(2)
	n >>= 26; // integer portion
	if (n < 0) {
		return (0);
	} else {
		r = (Word64) d * 0x55555555 >> 32; // r = delta/3, Q32
		r = (Word64) d * r >> 32; // delta.^2/3
		r += d; // delta + delta.^2/3
		r = (Word64) d * r >> 32; // d.^2 + d.^3/3
		q = uexp_table[q]; // Q31
		r = d + (r >> 1); // d + 1/2(d.^2) + 1/6(d.^3)
		r = (Word64) q * r >> 32; // Q31
		n = 31 - n;
		n = ((Word64) r + q) >> n;
		return (n);
	}
}

UWord32 rsqrt(UWord32 q) { // q : Q32
	Word32 s; // f(x) = d - x.^-2 = 0
	UWord32 d; // x_{n+1} = x_{n} - f(x_{n})/f'(x_{n})
	UWord32 a; // x_{n+1} = x_{n} - (d - x_{n}.^-2)/(2*x_{n}.^-3)
	UWord32 b; // x_{n+1} = 1.5 * x_{n} - 0.5 * d * x_{n}.^3 = 0.5 x_{n} * (3 - d*x_{n}.^2)

	s = CLZ(q);
	s &= ~1; // normalizations
	d = q << s; // Q32, sqrt(2^s * q)
	q = ursqrt_table[(d >> 25) - 32]; // 256 / sqrt(d/128) - 256
	if (d == 0) {
		return (0xffffffffUL);
	} else {
		q += 256; // Q8
		a = q * q; // Q16
		b = d >> 17; // Q15
		a = (Word64) a * b >> 16; // Q15
		b = q << 7; // Q15
		a = (0x3L << 15) - a; // Q15
		q = a * b; // Q31 of d = d0 * 2^32 = sqrt(d0).^-1 * 2^31
		s >>= 1; // d0 = 2^s0 * q0, sqrt(2^s0 * q0)^-1 * 2^31
		s = 15 - s; // 2^31 * 2^-(s0/2) * q0^-1/2
		q >>= s; // Q16
		return (q);
	}
}

UWord64 rsqrt_ll(UWord64 q) { // f(x) = x-d^-0.5, d-x.^-2
	Word32 s;
	UWord64 d;
	UWord64 a;
	UWord64 b;

	s = CLZ((q >> 32) & 0xffffffff);
	s += (s == 32) ? CLZ(q & 0xffffffff) : 0;
	s &= ~0x0001; // int
	d = q << s; // 2^s * q
	q = ursqrt_table[(d >> 57) - 32]; // 7 bits index
	if (d == 0) { // 2^62 < d <= 2^64-1, 0.25 < d <= 1-2^-32
		return (~((UWord64) 1)); // Q8 format
	} else {
		UWRD128 tmpno1;
		UWord64 tmp64no1;
		UWord64 tmp64no2;

		q += 256; // Q8
		a = (Word32) q * (Word32) q; // Q16
		b = d >> 17; // Q47, 64-17=47
		a = a * b >> 16; // Q47, q*q*d
		b = q << 7; // Q15
		a = ((UWord64) 3 << 47) - a; // Q47
		tmpno1 = umul_64_128(a, b); // Q63
		s >>= 1;
		s = 31 - s; // [sqrt(q0*2.^s)].^-0.5=q0.^-0.5 * 2.^-s/2
		tmp64no1 = ((UWord64) tmpno1.a3 << 32) + tmpno1.a2;
		tmp64no2 = ((UWord64) tmpno1.a1 << 32) + tmpno1.a0;
		tmp64no2 >>= s; // Q32
		tmp64no2 ^= ((tmp64no1 & (((UWord64) 1 << s) - 1)) << (64 - s)); // s bits
		return (q = tmp64no2); // Q32
	}
}

UWord32 usqrt(UWord32 q) {
	UWord32 d;
	d = rsqrt(q); // q0^(-0.5) * 2^16
	q = (Word64) d * q >> 32; // sqrt(q0 * 2^32) = q0.^0.5 * 2^16, q0 * 2^32 * q0^-0.5 * 2^16
	return (q);
}

UWord64 usqrt_ll(UWord64 q) {
	UWord64 d;
	UWord64 tmp64no1;
	UWRD128 tmpno1;

	d = rsqrt_ll(q); // sqrt(q0 * 2^64) = q0^0.5 * 2^32
	tmpno1 = umul_64_128(d, q); // Q32, q0.^(-0.5) * 2^32 * q0 * 2^64
	tmp64no1 = ((UWord64) tmpno1.a3 << 32) + tmpno1.a2;
	return (tmp64no1);
}