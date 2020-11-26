# Promise++
This project copies the Promise API from JavaScript in c++.

## Integration
`Promise.h` is all you need.
Simply copy the file to your project and include it.
```cpp
#include <PromisePP/Promise.h>

using PromisePP::Promise;
```

## Example
```cpp

PromisePP::Promise<int, std::string> p = PromisePP::Promise<int, std::string>([](std::function<void(int)> resolve, std::function<void(std::string)> reject) 
		{
			int result = 5 + 5;

			if (result > 10 ) resolve(result);
			else reject("This will be caught in an .error()");
		});

	// Currently then<void> is not supported.
	p.then<int>([](int result)
		{
			// Handle the result.
			std::cout << "Got the result " << result << std::endl;
			return 0;
		});

	p.error<int>([](std::string error) 
		{
			std::cout << "Error: \t" << error << std::endl;
			return 0;
		});
```
