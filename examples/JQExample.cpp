#include "../src/JQ.hpp"
#include <iostream>
#include <exception>

/*
 * Example usage:
 * 
 * $./JQExample "{\"foo\": \"bar\"}" "{baz: .foo}"
 * {"baz":"bar"}
 * 
 */

using namespace std;

int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Usage: JQExample JSON [filter]\n");
		return 1;
	}

	string json = argv[1];
	string filter = (argc > 2) ? argv[2] : ".";
	try
	{
		JQ jq(json, filter);
		if(!jq.getError().size())
		{
			cout << "Input:\t" << json << endl;
			cout << "Filter:\t" << filter << endl;
			cout << "Result:\t" << jq.result() << endl;
		}
		else
		{
			cerr << jq.getError() << endl;
		}
	}
	catch(exception& exc)
	{
		cerr << exc.what() << endl;
		return 1;
	}

	return 0;
}
