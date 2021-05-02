#include "config.h"

#include <string>
#include <iostream>
#include <stdexcept>

#include <fmt/format.h>

namespace omm::config
{
	void visit_config(std::istream& i, void* obj, config_callback visitor)
	{
        std::string section;
        std::string line;
        int line_count = 0;
        while (std::getline(i, line))
        {
            ++line_count;

            auto line_view = std::string_view(line);
            
            while (!line_view.empty() && std::isspace(line_view.front()))
                line_view = line_view.substr(1);
            
            if (line_view.empty() || line_view[0] == ';')
            {
                continue;
            }
            else if (line_view[0] == '[')
            {
                size_t const end_pos = line_view.find(']');
                if(end_pos == std::string_view::npos)
                    throw std::runtime_error(fmt::format("Error on line {}: expected section to end with ']'", line_count));

                section = line_view.substr(1, end_pos - 1);
            }
            else
            {
                std::string_view key;
                auto const key_end_pos = line_view.find('=');
                if (key_end_pos == std::string_view::npos)
                {
                    key = line_view;
                }
                else
                {
                    key = line_view.substr(0, key_end_pos);
                }

                while (std::isspace(key.back()))
                {
                    key = key.substr(0, key.size() - 1);
                }

                std::string_view value;
                if (key_end_pos != std::string_view::npos && line_view.size() > key_end_pos + 1)
                {
                    line_view = line_view.substr(key_end_pos + 1, line_view.size() - key_end_pos + 1);
                    while (!line_view.empty() && std::isspace(line_view.front()))
                        line_view = line_view.substr(1);

                    while (!line_view.empty() && std::isspace(line_view.back()))
                        line_view = line_view.substr(0, line_view.size() - 1);

                    if (!line_view.empty())
                    {
                        if (line_view.front() == '"')
                        {
                            if (line_view.size() == 1 || line_view.find('"', 1) != line_view.size() - 1)
                                throw std::runtime_error("Error on line {}: mismatched quotes on key");

                            line_view = line_view.substr(1, line_view.size() - 2);
                        }

                        value = line_view;
                    }
                }

                visitor(obj, section, key, value);
            }
        }
	}
}
