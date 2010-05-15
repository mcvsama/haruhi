#include <iostream>
#include <utility/memory.h>
#include <utility/signal.h>

class Receiver: public Signal::Receiver
{
	void
	receive()
	{
		std::clog << "Signal received\n";
	}
};


int main()
{
	Receiver recv;
	Signal::Signal0 sig;
	sig.connect (&recv, Receiver::receive);
	sig();
	return 0;
}
