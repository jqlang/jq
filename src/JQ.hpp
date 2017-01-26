#include "compile.h"
#include "jv.h"
#include "jq.h"
#include "jv_alloc.h"
#include "util.h"
#include <string>

class JQ
{
public:
	JQ(const std::string& json = "", const std::string& filter = "", bool dbg = false);
	~JQ();

	void setJSON(const std::string& json);
	const std::string& getJSON() const;

	void setFilter(const std::string& filter);
	const std::string& getFilter() const;

	bool process(const std::string& json, const std::string& filter, bool dbg = false);
	const std::string& result(int flags = 0);

	const std::string& getError() const;

private:
	void clear();
	void cleanup();
	void cleanup(jv& j, const std::string& msg = "");

	jq_state*           _jq;
	std::string         _json;
	std::string         _filter;
	std::string         _filtered_json;
	jv                  _input;
	jv                  _result;
	bool                _processed;
	mutable std::string _error;
};

inline void JQ::setJSON(const std::string& json)
{
	_json = json;
}

inline const std::string& JQ::getJSON() const
{
	return _json;
}

inline void JQ::setFilter(const std::string& filter)
{
	_filter = filter;
}

inline const std::string& JQ::getFilter() const
{
	return _filter;
}

inline const std::string& JQ::getError() const
{
	return _error;
}
