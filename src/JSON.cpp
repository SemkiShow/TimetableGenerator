// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "JSON.hpp"
#include <climits>
#include <fstream>
#include <iostream>
#include <sstream>

inline std::string Indentation(size_t level) { return std::string(level, '\t'); }

void JSON::SkipWhitespace(const std::string& s, size_t& idx)
{
    while (idx < s.size() && isspace(s[idx]))
        ++idx;
}

JSON JSON::ParseValue(const std::string& s, size_t& idx)
{
    SkipWhitespace(s, idx);
    if (idx >= s.size()) throw std::runtime_error("Unexpected end of input");

    if (s[idx] == '{') return ParseObject(s, idx);
    if (s[idx] == '[') return ParseArray(s, idx);
    if (s[idx] == '"') return ParseString(s, idx);
    if (isdigit(s[idx]) || s[idx] == '-' || s[idx] == '+') return ParseNumber(s, idx);
    if (s.compare(idx, 4, "true") == 0)
    {
        idx += 4;
        return JSON(true);
    }
    if (s.compare(idx, 5, "false") == 0)
    {
        idx += 5;
        return JSON(false);
    }
    if (s.compare(idx, 4, "null") == 0)
    {
        idx += 4;
        return JSON(nullptr);
    }

    throw std::runtime_error(std::string("Unexpected token: ") + s[idx]);
}

JSON JSON::ParseObject(const std::string& s, size_t& idx)
{
    ++idx; // skip '{'
    object_t obj;
    SkipWhitespace(s, idx);
    if (idx < s.size() && s[idx] == '}')
    {
        ++idx;
        return obj;
    }

    while (true)
    {
        SkipWhitespace(s, idx);
        if (s[idx] != '"') throw std::runtime_error("Expected string key");
        std::string key = ParseString(s, idx).GetString();
        SkipWhitespace(s, idx);
        if (s[idx] != ':') throw std::runtime_error("Expected ':' after key");
        ++idx;
        JSON val = ParseValue(s, idx);
        obj[key] = val;
        SkipWhitespace(s, idx);
        if (s[idx] == '}')
        {
            ++idx;
            break;
        }
        if (s[idx] != ',') throw std::runtime_error("Expected ',' or '}'");
        ++idx;
    }
    return JSON(obj);
}

JSON JSON::ParseArray(const std::string& s, size_t& idx)
{
    ++idx; // skip '['
    array_t arr;
    SkipWhitespace(s, idx);
    if (idx < s.size() && s[idx] == ']')
    {
        ++idx;
        return arr;
    }

    while (true)
    {
        JSON val = ParseValue(s, idx);
        arr.push_back(val);
        SkipWhitespace(s, idx);
        if (s[idx] == ']')
        {
            ++idx;
            break;
        }
        if (s[idx] != ',') throw std::runtime_error("Expected ',' or ']'");
        ++idx;
    }
    return JSON(arr);
}

JSON JSON::ParseString(const std::string& s, size_t& idx)
{
    ++idx; // skip '"'
    std::string str;
    while (idx < s.size())
    {
        if (s[idx] == '"')
        {
            ++idx;
            break;
        }
        if (s[idx] == '\\')
        {
            ++idx;
            if (idx >= s.size()) throw std::runtime_error("Invalid escape sequence");
            switch (s[idx])
            {
            case '"':
                str.push_back('"');
                break;
            case '\\':
                str.push_back('\\');
                break;
            case '/':
                str.push_back('/');
                break;
            case 'b':
                str.push_back('\b');
                break;
            case 'f':
                str.push_back('\f');
                break;
            case 'n':
                str.push_back('\n');
                break;
            case 'r':
                str.push_back('\r');
                break;
            case 't':
                str.push_back('\t');
                break;
            default:
                throw std::runtime_error("Unknown escape character");
            }
        }
        else
            str.push_back(s[idx]);
        ++idx;
    }
    return JSON(str);
}

JSON JSON::ParseNumber(const std::string& s, size_t& idx)
{
    size_t start = idx;
    if (s[idx] == '-' || s[idx] == '+') ++idx;

    bool isDouble = false;

    while (idx < s.size())
    {
        char c = s[idx];
        if (isdigit(c))
        {
            ++idx;
        }
        else if (c == '.')
        {
            if (isDouble) throw std::runtime_error("Invalid number: multiple decimals");
            isDouble = true;
            ++idx;
        }
        else if (c == 'e' || c == 'E')
        {
            isDouble = true;
            ++idx;
            if (idx < s.size() && (s[idx] == '+' || s[idx] == '-')) ++idx;
            if (idx >= s.size() || !isdigit(s[idx]))
                throw std::runtime_error("Invalid number: exponent missing digits");
            while (idx < s.size() && isdigit(s[idx]))
                ++idx;
        }
        else
        {
            break;
        }
    }

    std::string numStr = s.substr(start, idx - start);

    try
    {
        if (isDouble)
        {
            // If it has decimal point or exponent → store as double
            double d = std::stod(numStr);
            return JSON(d);
        }
        else
        {
            // Try parsing as int first
            long long n = std::stoll(numStr);
            if (n >= INT_MIN && n <= INT_MAX)
                return JSON(static_cast<int>(n)); // store as int if fits
            else
                return JSON(static_cast<double>(n)); // store as double if too big
        }
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Invalid number: ") + e.what());
    }
}

std::string JSON::EscapeString(const std::string& s)
{
    std::string out;
    for (char c: s)
    {
        switch (c)
        {
        case '"':
            out += "\\\"";
            break;
        case '\\':
            out += "\\\\";
            break;
        case '\b':
            out += "\\b";
            break;
        case '\f':
            out += "\\f";
            break;
        case '\n':
            out += "\\n";
            break;
        case '\r':
            out += "\\r";
            break;
        case '\t':
            out += "\\t";
            break;
        default:
            out += c;
            break;
        }
    }
    return out;
}

std::string JSON::ToString(size_t level) const
{
    std::ostringstream oss;
    if (IsNull())
        oss << "null";
    else if (IsBool())
        oss << (std::get<bool>(value) ? "true" : "false");
    else if (IsInt())
        oss << std::get<int>(value);
    else if (IsDouble())
        oss << std::get<double>(value);
    else if (IsString())
        oss << "\"" << EscapeString(std::get<std::string>(value)) << "\"";
    else if (IsArray())
    {
        const auto& arr = std::get<array_t>(value);
        oss << "[";
        if (format == JSONFormat::Newline) oss << '\n';
        for (size_t i = 0; i < arr.size(); ++i)
        {
            if (format == JSONFormat::Newline) oss << Indentation(level + 1);
            oss << arr[i].ToString(level + 1);
            if (i < arr.size() - 1) oss << ", ";
            if (format == JSONFormat::Newline) oss << '\n';
        }
        if (format == JSONFormat::Newline) oss << Indentation(level);
        oss << "]";
    }
    else if (IsObject())
    {
        const auto& obj = std::get<object_t>(value);
        oss << "{";
        if (format == JSONFormat::Newline) oss << '\n';
        size_t count = 0;
        for (const auto& [k, v]: obj)
        {
            if (format == JSONFormat::Newline) oss << Indentation(level + 1);
            oss << "\"" << EscapeString(k) << "\": " << v.ToString(level + 1);
            if (count++ < obj.size() - 1) oss << ", ";
            if (format == JSONFormat::Newline) oss << '\n';
        }
        if (format == JSONFormat::Newline) oss << Indentation(level);
        oss << "}";
    }
    return oss.str();
}

void JSON::Save(const std::string& path)
{
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Cannot open file: " + path);
    f << ToString();
    f.close();
}

JSON JSON::Load(const std::string& path)
{
    std::ifstream f(path);
    if (!f) throw std::runtime_error("Cannot open file: " + path);
    std::stringstream ss;
    ss << f.rdbuf();
    f.close();
    return Parse(ss.str());
}

JSON JSON::Parse(const std::string& s)
{
    size_t idx = 0;
    return ParseValue(s, idx);
}
