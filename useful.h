#pragma once

#include <regex>
#include <climits>
#include <string>
#include <vector>
#include <stdexcept>
#include <ostream>

static std::regex operator ""_r(const char* pattern, std::size_t length)
{
    return std::regex(pattern, length);
}

static int operator ""_sys(const char* call, std::size_t length)
{
    return system(NULL) != 0 ? system(call) : INT_MAX;
}

static std::string operator ""_env(const char* variable, std::size_t length)
{
    char* char_result = getenv(variable);
    return char_result == NULL ? "" : std::string(char_result);
}

static std::runtime_error operator ""_error (const char* msg, std::size_t length)
{
    return std::runtime_error(msg);
}

class Formatter
{
private:
    std::string format;
    std::vector<std::string> arguments;
    bool checked;
    
    std::string create_from_fields()
    {
        if(checked)
            return format;
        
        std::string result = "";
        int next_position = 0;
        int reading_position = -1;
        bool has_position;
        
        for(int i = 0; i < int(format.size()); ++i)
        {
            switch(format[i])
            {
                default:
                    result.push_back(format[i]);
                    break;
                case '{':
                    if(i == int(format.size()) - 1)
                    {
                        result.push_back('{');
                    }
                    else if(format[i + 1] == '{')
                    {
                        result.push_back('{');
                        ++i;
                    }
                    else
                    {
                        has_position = false;
                        reading_position = 0;
                        
                        int j = i + 1;
                        while(true)
                        {
                            if(j >= int(format.size()))
                            {
                                for(; i < j; ++i)
                                    result.push_back(format[i]);
                                return result;
                            }
                            
                            switch(format[j])
                            {
                                case '{':
                                    throw "Encountered '{' inside braces."_error;
                                    break;
                                case '}':
                                    goto success;
                                    break;
                                case '0':
                                case '1':
                                case '2':
                                case '3':
                                case '4':
                                case '5':
                                case '6':
                                case '7':
                                case '8':
                                case '9':
                                    has_position = true;
                                    reading_position = 10*reading_position + (format[j] - '0');
                                    break;
                                default:
                                    throw "Unexpected characters in braces."_error;
                                    break;
                            }
                            ++j;
                        }
                        success: i = j;
                        int position = has_position ? reading_position : next_position;
                        if(!has_position)
                            ++next_position;
                        if(position < 0 || position >= int(arguments.size()))
                            throw std::runtime_error("Invalid position read from arguments: " + std::to_string(position));
                        else
                            result += arguments[position];
                    }
                    break;
                case '}':
                    throw "Unmatched '}' encountered."_error;
                    break;
            }
                
        }
        checked = true;
        format = result;
        arguments.clear();
        arguments.shrink_to_fit();
        
        return result;
    }
    
public:
    Formatter(const char* format) : checked(false), format(format), arguments(std::vector<std::string>())
    {
    }
    
    template <typename T>
    Formatter& operator%(const T& other)
    {
        arguments.push_back(std::to_string(other));
        return *this;
    }
    
    operator std::string()
    {
        return create_from_fields();
    }
};

std::ostream& operator<<(std::ostream &out, Formatter& obj)
{
    return out << std::string(obj);
}

static Formatter operator ""_format(const char* format_string, std::size_t l)
{
    return Formatter(format_string);
}

