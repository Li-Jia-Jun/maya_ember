#pragma once

#include "geometry.h"

namespace ember
{
	int multiply(Point x, Plane s)
	{
		return x.x1 * s.a + x.x2 * s.b + x.x3 * s.c + x.x4 * s.d;
	}

	int sign(int value)
	{
		if (value > 0) return 1;
		else if (value < 0) return -1;
		else return 0;
	}

	int determiant3x3(
		int a, int b, int c,
		int d, int e, int f,
		int g, int h, int i)
	{
		int t1 = a * (e * i - f * h);
		int t2 = b * (d * i - f * g);
		int t3 = c * (d * h - e * g);
		return t1 - t2 + t3;
	}

	Point intersect(Plane p, Plane q, Plane r)
	{
		int x1 = determiant3x3(
			p.d, p.b, p.c,
			q.d, q.b, q.c,
			r.d, r.b, r.c);

		int x2 = determiant3x3(
			p.a, p.d, p.c,
			q.a, q.d, q.c,
			r.a, r.d, r.c);

		int x3 = determiant3x3(
			p.a, p.b, p.d,
			q.a, q.b, q.d,
			r.a, r.b, r.d);

		int x4 = determiant3x3(
			p.a, p.b, p.c,
			q.a, q.b, q.c,
			r.a, r.b, r.c);
	
		return { x1, x2, x3, x4 };
	}


	int classify(Point x, Plane s)
	{
		// TODO
		return -1;
	}
}

