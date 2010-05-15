#include <iostream>
#include <vector>
#include <dsp/envelope.h>

int main()
{
	using DSP::Envelope;

	Envelope e;
	e.points().push_back (Envelope::Point (0.0, 10));
	e.points().push_back (Envelope::Point (1.0, 5));
	e.points().push_back (Envelope::Point (0.5, 5));
	e.points().push_back (Envelope::Point (0.3, 5));
	e.set_release_point (3);
	e.prepare();
	float k;
	for (int i = 0; i < 50; ++i)
	{
		e.fill (&k, &k + 1);
		if (i == 5)
			e.release();
	}
}

