#pragma once

#include <regex>
#include <climits>
#include <string>
#include <vector>
#include <stdexcept>
#include <ostream>
#include <cctype>
#include <unordered_map>
#include <map>

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

template <typename T>
class Keyword
{
public:
    std::string name;
    T value;
    
    Keyword(std::string name, T value) : name(name), value(value)
    {
    }
};

class Formatter
{
private:
    std::string format;
    std::vector<std::string> arguments;
    std::unordered_map<std::string, std::string> keywords;
    
    std::string create_from_fields()
    {   
        std::string result = "";
        int next_position = 0;
        
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
                        int reading_position = 0;
                        std::string reading_keyword = "";
                        
                        bool has_position = false;
                        bool has_keyword = false;
                        bool read_nonblank_bits = false;
                        bool read_blank_bits = false;
                        
                        int j = i + 1;
                        while(true)
                        {
                            if(j >= int(format.size()))
                            {
                                for(; i < j; ++i)
                                    result.push_back(format[i]);
                                return result;
                            }
                            
                            if(!read_nonblank_bits)
                            {
                                if(format[j] == '}')
                                {
                                    break;
                                }
                                else if(isdigit(format[j]))
                                {
                                    has_position = true;
                                    read_nonblank_bits = true;
                                    reading_position = 10*reading_position + (format[j] - '0');
                                }
                                else if(isalpha(format[j]))
                                {
                                    has_keyword = true;
                                    read_nonblank_bits = true;
                                    reading_keyword.push_back(format[j]);
                                }
                                else if(isblank(format[j]))
                                {
                                }
                                else if(format[j] == '{')
                                {
                                    throw "Encountered '{' inside braces."_error;
                                }
                                else
                                {
                                    throw "Unexpected characters in braces."_error;
                                }
                            }
                            else
                            {
                                if(format[j] == '}')
                                {
                                    break;
                                }
                                else if(has_position && isdigit(format[j]))
                                {
                                    if(read_blank_bits)
                                        throw "Invalid spacing inside braces"_error;
                                    reading_position = 10*reading_position + (format[j] - '0');
                                }
                                else if(has_keyword && isalnum(format[j]))
                                {
                                    if(read_blank_bits)
                                        throw "Invalid spacing inside braces"_error;
                                    reading_keyword.push_back(format[j]);
                                }
                                else if(isblank(format[j]))
                                {
                                    read_blank_bits = true;
                                }
                                else if(format[j] == '{')
                                {
                                    throw "Encountered '{' inside braces."_error;
                                }
                                else
                                {
                                    throw "Unexpected characters in braces."_error;
                                }
                            }
                            
                            ++j;
                        }
                        i = j;
                        
                        if(has_keyword)
                        {
                            if(keywords.count(reading_keyword) == 0)
                                throw std::runtime_error("Invalid keyword read from format: \"" + reading_keyword + "\"");
                            else
                                result += keywords[reading_keyword];
                        }
                        else
                        {
                            int position = has_position ? reading_position : next_position++;
                            if(position < 0 || position >= int(arguments.size()))
                                throw std::runtime_error("Invalid position read from format: \"" + std::to_string(position) + "\"");
                            else
                                result += arguments[position];
                        }
                        
                    }
                    break;
                case '}':
                    throw "Unmatched '}' encountered."_error;
                    break;
            }
                
        }        
        return result;
    }
    
public:
    Formatter(const char* format) : format(format), arguments(std::vector<std::string>()), keywords(std::unordered_map<std::string, std::string>())
    {
    }
    
    template <typename T>
    Formatter& operator%(const T& other)
    {
        arguments.push_back(std::to_string(other));
        return *this;
    }
    
    Formatter& operator%(const std::string& other)
    {
        arguments.push_back(other);
        return *this;
    }
    
    template <typename T>
    Formatter& operator%(const Keyword<T>& other)
    {
        if(keywords.count(other.name) == 1)
            throw std::runtime_error("Duplicate keywords read from format: \"" + other.name + "\"");
        else
            keywords[other.name] = std::to_string(other.value);
        return *this;
    }
    
    Formatter& operator%(const Keyword<std::string>& other)
    {
        if(keywords.count(other.name) == 1)
            throw std::runtime_error("Duplicate keywords read from format: \"" + other.name + "\"");
        else
            keywords[other.name] = other.value;
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

