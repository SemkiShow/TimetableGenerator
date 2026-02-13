// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

enum class JsonFormat
{
    Newline,
    Inline
};

// If commented out, functions like Get*() return 0 instead of throwing an error on a null object
// #define JSON_STRICT_ERRORS

class Json
{
  public:
    using array_t = std::vector<Json>;
    using object_t = std::unordered_map<std::string, Json>;
    using value_t = std::variant<std::nullptr_t, bool, int, double, std::string, array_t, object_t>;

  private:
    value_t value;

  public:
    Json() : value(nullptr) {}
    Json(std::nullptr_t) : value(nullptr) {}
    Json(bool b) : value(b) {}
    Json(int n) : value(n) {}
    Json(double n) : value(n) {}
    Json(const char* s) : value(std::string(s)) {}
    Json(const std::string& s) : value(s) {}
    Json(const array_t& a) : value(a) {}
    Json(const object_t& o) : value(o) {}
    Json(const JsonFormat format) : value(nullptr), format(format) {}

    JsonFormat format = JsonFormat::Newline;

    // Type queries
    bool IsNull() const { return std::holds_alternative<std::nullptr_t>(value); }
    bool IsBool() const { return std::holds_alternative<bool>(value); }
    bool IsInt() const { return std::holds_alternative<int>(value); }
    bool IsDouble() const { return std::holds_alternative<double>(value); }
    bool IsString() const { return std::holds_alternative<std::string>(value); }
    bool IsArray() const { return std::holds_alternative<array_t>(value); }
    bool IsObject() const { return std::holds_alternative<object_t>(value); }

    // Queries
    bool GetBool() const
    {
#ifndef JSON_STRICT_ERRORS
        if (IsNull()) return 0;
#endif
        if (!IsBool()) throw std::runtime_error("JSONValue is not a bool");
        return std::get<bool>(value);
    }

    int GetInt() const
    {
        if (IsInt()) return std::get<int>(value);
        if (IsDouble())
        {
            double d = std::get<double>(value);
            if (d == static_cast<int>(d)) return static_cast<int>(d);
        }
#ifndef JSON_STRICT_ERRORS
        if (IsNull()) return 0;
#endif
        throw std::runtime_error("JSONValue is not an int");
    }

    double GetDouble() const
    {
        if (IsInt()) return GetInt();
#ifndef JSON_STRICT_ERRORS
        if (IsNull()) return 0;
#endif
        if (!IsDouble()) throw std::runtime_error("JSONValue is not a double");
        return std::get<double>(value);
    }

    std::string GetString() const
    {
#ifndef JSON_STRICT_ERRORS
        if (IsNull()) return "";
#endif
        if (!IsString()) throw std::runtime_error("JSONValue is not a string");
        return std::get<std::string>(value);
    }

    array_t& GetArray()
    {
        if (!IsArray()) throw std::runtime_error("JSONValue is not an array");
        return std::get<array_t>(value);
    }

    object_t& GetObject()
    {
        if (!IsObject()) throw std::runtime_error("JSONValue is not an object");
        return std::get<object_t>(value);
    }

    const array_t& GetArray() const
    {
        if (!IsArray()) throw std::runtime_error("JSONValue is not an array");
        return std::get<array_t>(value);
    }

    const object_t& GetObject() const
    {
        if (!IsObject()) throw std::runtime_error("JSONValue is not an object");
        return std::get<object_t>(value);
    }

    // Access
    void push_back(const Json& element)
    {
        if (IsNull()) value = array_t{};
        if (!IsArray()) throw std::runtime_error("Cannot push_back to non-array Json");
        GetArray().push_back(element);
    }

    void emplace_back(const Json& element)
    {
        if (IsNull()) value = array_t{};
        if (!IsArray()) throw std::runtime_error("Cannot emplace_back to non-array Json");
        GetArray().emplace_back(element);
    }

    Json& back()
    {
        if (IsNull()) value = array_t{};
        if (!IsArray()) throw std::runtime_error("Cannot push_back to non-array Json");
        return GetArray().back();
    }

    size_t size()
    {
        if (!IsArray() && !IsObject()) return 0;
        return IsArray() ? GetArray().size() : GetObject().size();
    }

    Json& operator[](const std::string& key)
    {
        if (!IsObject()) value = object_t{};
        return std::get<object_t>(value)[key];
    }

    const Json& operator[](const std::string& key) const
    {
        if (!IsObject()) throw std::runtime_error("Not an object");
        const auto& obj = std::get<object_t>(value);
        auto it = obj.find(key);
        if (it == obj.end()) throw std::out_of_range("Key not found");
        return it->second;
    }

    Json& operator[](size_t index)
    {
        if (!IsArray()) value = array_t{};
        auto& arr = std::get<array_t>(value);
        if (index >= arr.size()) arr.resize(index + 1);
        return arr[index];
    }

    const Json& operator[](size_t index) const
    {
        if (!IsArray()) throw std::runtime_error("Not an array");
        const auto& arr = std::get<array_t>(value);
        if (index >= arr.size()) throw std::out_of_range("Index out of range");
        return arr[index];
    }

    // Assignment
    Json& operator=(std::nullptr_t)
    {
        value = nullptr;
        return *this;
    }

    Json& operator=(bool b)
    {
        value = b;
        return *this;
    }

    Json& operator=(int n)
    {
        value = n;
        return *this;
    }

    Json& operator=(double n)
    {
        value = n;
        return *this;
    }

    Json& operator=(const char* s)
    {
        value = std::string(s);
        return *this;
    }

    Json& operator=(const std::string& s)
    {
        value = s;
        return *this;
    }

    Json& operator=(const array_t& a)
    {
        value = a;
        return *this;
    }

    Json& operator=(const object_t& o)
    {
        value = o;
        return *this;
    }

    void Save(const std::filesystem::path& path);
    static Json Load(const std::filesystem::path& path);
    static Json Parse(const std::string& json);
    std::string ToString(size_t level = 0) const;

  private:
    static void SkipWhitespace(const std::string& s, size_t& idx);
    static Json ParseValue(const std::string& s, size_t& idx);
    static Json ParseObject(const std::string& s, size_t& idx);
    static Json ParseArray(const std::string& s, size_t& idx);
    static Json ParseString(const std::string& s, size_t& idx);
    static Json ParseNumber(std::string_view s, size_t& idx);
    static void EscapeString(std::string& out, const std::string& s);
    void ToString(std::string& buf, size_t level = 0) const;
};
