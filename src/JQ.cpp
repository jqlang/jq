#include "JQ.hpp"
#include "memstream.h"
#include <stdlib.h>
#include <stdexcept>

JQ::JQ(const std::string& json, const std::string& filter, bool dbg) :
	_jq(jq_init()), _input{0}, _result{0}, _processed(false)
{
	if(!_jq) { cleanup(); }
	process(json, filter, dbg);
}

JQ::~JQ()
{
	cleanup();
}

bool JQ::process(const std::string& json, const std::string& filter, bool dbg)
{
	cleanup(_input);
	cleanup(_result);
	clear();

	if(!_jq) { cleanup(); }
	if(!jq_compile(_jq, filter.c_str()))
	{
		_error = "Filter parsing failed.";
		return false;
	}

	_input = jv_parse/*_sized*/(json.c_str()/*, json.length()*/);
	if (!jv_is_valid(_input))
	{
		cleanup(_input, "JSON parse error.");
		return false;
	}

	jq_start(_jq, _input, dbg ? JQ_DEBUG_TRACE : 0);
	_input = jv_null(); // jq_start() freed it
	_result = jq_next(_jq);
	if (!jv_is_valid(_result))
	{
		cleanup(_result, "JQ filtering result invalid.");
		return false;
	}
	_json = json;
	_filter = filter;
	return _processed = true;
}

const std::string& JQ::result(int flags)
{
	if(_processed)
	{
		static const std::string ret;
		if(!_error.empty()) { return ret; }
		char* buf;
		size_t len;
		FILE* f = open_memstream(&buf, &len);
		if(f == NULL)
		{
			_error = "Can't open memory stream for writing.";
			return ret;
		}
		jv_dumpf(_result, f, flags);
		_result = jv_null();
		clear();
		fclose (f);
		_filtered_json.assign(buf, len);
		free (buf);
		_processed = false;
	}
	return _filtered_json;
}

void JQ::clear()
{
	_result = jv_null();
	_input = jv_null();
	_filtered_json.clear();
	_error.clear();
	_processed = false;
}

void JQ::cleanup()
{
	if(_jq)
	{
		cleanup(_input);
		cleanup(_result);
		clear();
		jq_teardown(&_jq);
		_jq = 0;
	}
	else
	{
		throw std::runtime_error("JQ handle is null.");
	}
}

void JQ::cleanup(jv& j, const std::string& msg)
{
	if(j.u.ptr)
	{
		jv_free(j);
		j = jv_null();
	}
	_error = msg;
}
