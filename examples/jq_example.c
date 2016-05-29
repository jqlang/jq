#include "../src/jq.h"
#include <stdio.h>

/*
 * Example usage:
 * 
 * $./jq_example "{\"foo\": \"bar\"}" "{baz: .foo}"
 * {"baz":"bar"}
 * 
 */
 
int main(int argc, char* argv[])
{
	if(argc < 2)
	{
		printf("Usage: jq_example JSON [filter]\n");
		return 1;
	}

	const char* json = argv[1];
	const char* filter = (argc > 2) ? argv[2] : ".";
	jq_state* jq = jq_init();
	if(jq)
	{
		if(!jq_compile(jq, filter))
		{
			printf("Filter compilation error.\n");
			jq_teardown(&jq);
			return 2;
		}

		jv input = jv_parse(json);
		if (!jv_is_valid(input))
		{
			printf("Input is invalid.\n");
			jv_free(input);
			jq_teardown(&jq);
			return 3;
		}

		jq_start(jq, input, 0);
		jv result = jq_next(jq);
		if (!jv_is_valid(result))
		{
			printf("Result is invalid.\n");
			jv_free(result);
			jq_teardown(&jq);
			return 3;
		}

		jv_dumpf(result, stdout, 0);
		printf("\n");

		jq_teardown(&jq);
	}
	else
	{
		printf("Can't create jq.\n");
		return -1;
	}

	return 0;
}