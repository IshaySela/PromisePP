#include <exception>
#include <iostream>
#include "Promise.h"

int doOperation(int x ) { return 5 + x; }

int main()
{	
	PromisePP::Promise<int, std::exception> p = PromisePP::Promise<int, std::exception>([](std::function<void(int)> resolve, std::function<void(std::exception)> reject) 
		{
			int result = doOperation(5);

			if (result >31 ) resolve(result);
			else reject(std::exception("This will be caught in an .error()"));
		});

	// Currently then<void> is not supported.
	p.then<int>([](int result)
		{
			// Handle the result.
			std::cout << "Got the result " << result << std::endl;
			return 0;
		});

	return 0;
}