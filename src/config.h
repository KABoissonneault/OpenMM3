#pragma once

#include <string_view>
#include <iosfwd>

namespace omm::config
{
	using config_callback = void(void* obj, std::string_view section, std::string_view key, std::string_view value);
	void visit_config(std::istream& i, void* obj, config_callback visitor);
}
