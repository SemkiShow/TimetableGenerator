// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Json.hpp"
#include <charconv>
#include <climits>
#include <fstream>
#include <iostream>

void Indentation(std::string& buf, size_t level) { buf.append(4 * level, ' '); }

void StdToString(std::string& buf, int val)
{
    char tmp[64];
    auto [ptr, ec] = std::to_chars(tmp, tmp + 64, val);
    buf.append(tmp, ptr - tmp);
}

void StdToString(std::string& buf, double val)
{
    char tmp[64];
    auto [ptr, ec] = std::to_chars(tmp, tmp + 64, val);
    buf.append(tmp, ptr - tmp);
}

int Stoi(std::string_view s)
{
    int val;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
    if (ec == std::errc()) return val;
    throw std::runtime_error("Failed to convert " + std::string(s));
}

double Stod(std::string_view s)
{
    double val;
    auto [ptr, ec] = std::from_chars(s.data(), s.data() + s.size(), val);
    if (ec == std::errc()) return val;
    throw std::runtime_error("Failed to convert " + std::string(s));
}

void Json::SkipWhitespace(const std::string& s, size_t& idx)
{
    while (idx < s.size() && isspace(s[idx])) ++idx;
}

Json Json::ParseValue(const std::string& s, size_t& idx)
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
        return Json(true);
    }
    if (s.compare(idx, 5, "false") == 0)
    {
        idx += 5;
        return Json(false);
    }
    if (s.compare(idx, 4, "null") == 0)
    {
        idx += 4;
        return Json(nullptr);
    }

    throw std::runtime_error(std::string("Unexpected token: ") + s[idx]);
}

Json Json::ParseObject(const std::string& s, size_t& idx)
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
        obj[std::move(key)] = ParseValue(s, idx);
        SkipWhitespace(s, idx);
        if (s[idx] == '}')
        {
            ++idx;
            break;
        }
        if (s[idx] != ',') throw std::runtime_error("Expected ',' or '}'");
        ++idx;
    }
    return Json(obj);
}

Json Json::ParseArray(const std::string& s, size_t& idx)
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
        arr.push_back(ParseValue(s, idx));
        SkipWhitespace(s, idx);
        if (s[idx] == ']')
        {
            ++idx;
            break;
        }
        if (s[idx] != ',') throw std::runtime_error("Expected ',' or ']'");
        ++idx;
    }
    return Json(arr);
}

Json Json::ParseString(const std::string& s, size_t& idx)
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
    return Json(str);
}

Json Json::ParseNumber(std::string_view s, size_t& idx)
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
            while (idx < s.size() && isdigit(s[idx])) ++idx;
        }
        else
        {
            break;
        }
    }

    std::string_view numStr = s.substr(start, idx - start);

    try
    {
        if (isDouble)
        {
            // If it has decimal point or exponent - store as double
            double d = Stod(numStr);
            return Json(d);
        }
        else
        {
            // Try parsing as int first
            int n = Stoi(numStr);
            if (n >= INT_MIN && n <= INT_MAX)
                return Json(n); // store as int if fits
            else
                return Json(static_cast<double>(n)); // store as double if too big
        }
    }
    catch (const std::exception& e)
    {
        throw std::runtime_error(std::string("Invalid number: ") + e.what());
    }
}

void Json::EscapeString(std::string& out, const std::string& s)
{
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
}

void Json::ToString(std::string& buf, size_t level) const
{
    if (IsNull())
        buf += "null";
    else if (IsBool())
        buf += (std::get<bool>(value) ? "true" : "false");
    else if (IsInt())
        StdToString(buf, std::get<int>(value));
    else if (IsDouble())
        StdToString(buf, std::get<double>(value));
    else if (IsString())
    {
        buf += "\"";
        EscapeString(buf, std::get<std::string>(value));
        buf += "\"";
    }
    else if (IsArray())
    {
        const auto& arr = std::get<array_t>(value);
        buf += "[";
        if (format == JsonFormat::Newline) buf += '\n';
        for (size_t i = 0; i < arr.size(); ++i)
        {
            if (format == JsonFormat::Newline) Indentation(buf, level + 1);
            arr[i].ToString(buf, level + 1);
            if (i < arr.size() - 1) buf += ", ";
            if (format == JsonFormat::Newline) buf += '\n';
        }
        if (format == JsonFormat::Newline) Indentation(buf, level);
        buf += "]";
    }
    else if (IsObject())
    {
        const auto& obj = std::get<object_t>(value);
        buf += "{";
        if (format == JsonFormat::Newline) buf += '\n';
        size_t count = 0;
        for (const auto& [k, v]: obj)
        {
            if (format == JsonFormat::Newline) Indentation(buf, level + 1);
            buf += "\"";
            EscapeString(buf, k);
            buf += "\": ";
            v.ToString(buf, level + 1);
            if (count++ < obj.size() - 1) buf += ", ";
            if (format == JsonFormat::Newline) buf += '\n';
        }
        if (format == JsonFormat::Newline) Indentation(buf, level);
        buf += "}";
    }
}

std::string Json::ToString(size_t level) const
{
    std::string str;
    str.reserve(4 * 1024 * 1024);
    ToString(str, level);
    return str;
}

void Json::Save(const std::filesystem::path& path)
{
    std::ofstream f(path);
    if (!f) throw std::runtime_error("Cannot open file: " + path.string());
    f << ToString();
    f.close();
}

Json Json::Load(const std::filesystem::path& path)
{
    std::ifstream f(path, std::ios::binary);
    if (!f) throw std::runtime_error("Cannot open file: " + path.string());

    std::streamsize size = std::filesystem::file_size(path);
    if (size <= 0) return Json();

    std::string s(size, '\0');
    if (f.read(s.data(), size))
    {
        return Parse(s);
    }

    throw std::runtime_error("Failed to read file: " + path.string());
}

Json Json::Parse(const std::string& s)
{
    size_t idx = 0;
    return ParseValue(s, idx);
}
