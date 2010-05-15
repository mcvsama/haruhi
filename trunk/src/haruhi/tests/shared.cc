#include <utility/memory.h>

int main()
{
	Shared<int> a = new int(5);
	Shared<int> b = a;
	return 0;
}

