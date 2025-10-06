// SPDX-FileCopyrightText: 2025 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#pragma once

#include <map>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

enum class JSONFormat
{
    Newline,
    Inline
};

class JSON
{
  public:
    using array_t = std::vector<JSON>;
    using object_t = std::map<std::string, JSON>;
    using value_t = std::variant<std::nullptr_t, bool, int, double, std::string, array_t, object_t>;

  private:
    value_t value;

  public:
    JSON() : value(nullptr) {}
    JSON(std::nullptr_t) : value(nullptr) {}
    JSON(bool b) : value(b) {}
    JSON(int n) : value(n) {}
    JSON(double n) : value(n) {}
    JSON(const char* s) : value(std::string(s)) {}
    JSON(const std::string& s) : value(s) {}
    JSON(const array_t& a) : value(a) {}
    JSON(const object_t& o) : value(o) {}
    JSON(const JSONFormat format) : format(format) {}

    JSONFormat format = JSONFormat::Newline;

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
        throw std::runtime_error("JSONValue is not an int");
    }

    double GetDouble() const
    {
        if (!IsDouble()) throw std::runtime_error("JSONValue is not a double");
        return std::get<double>(value);
    }

    std::string GetString() const
    {
        if (!IsString()) throw std::runtime_error("JSONValue is not a string");
        return std::get<std::string>(value);
    }

    array_t GetArray() const
    {
        if (IsNull()) return array_t{};
        if (!IsArray()) throw std::runtime_error("JSONValue is not an array");
        return std::get<array_t>(value);
    }

    object_t GetObject() const
    {
        if (IsNull()) return object_t{};
        if (!IsObject()) throw std::runtime_error("JSONValue is not an object");
        return std::get<object_t>(value);
    }

    // Access
    void push_back(const JSON& element)
    {
        if (!IsArray()) value = array_t{};
        std::get<array_t>(value).push_back(element);
    }

    size_t size()
    {
        if (!IsArray() && !IsObject())
            throw std::runtime_error("JSONValue is not an array and is not an object");
        return IsArray() ? GetArray().size() : GetObject().size();
    }

    JSON& operator[](const std::string& key)
    {
        if (!IsObject()) value = object_t{};
        return std::get<object_t>(value)[key];
    }

    const JSON& operator[](const std::string& key) const
    {
        if (!IsObject()) throw std::runtime_error("Not an object");
        const auto& obj = std::get<object_t>(value);
        auto it = obj.find(key);
        if (it == obj.end()) throw std::out_of_range("Key not found");
        return it->second;
    }

    JSON& operator[](const char* key) { return (*this)[std::string(key)]; }

    const JSON& operator[](const char* key) const { return (*this)[std::string(key)]; }

    JSON& operator[](size_t index)
    {
        if (!IsArray()) value = array_t{};
        auto& arr = std::get<array_t>(value);
        if (index >= arr.size()) arr.resize(index + 1);
        return arr[index];
    }

    const JSON& operator[](size_t index) const
    {
        if (!IsArray()) throw std::runtime_error("Not an array");
        const auto& arr = std::get<array_t>(value);
        if (index >= arr.size()) throw std::out_of_range("Index out of range");
        return arr[index];
    }

    // Assignment
    JSON& operator=(std::nullptr_t)
    {
        value = nullptr;
        return *this;
    }

    JSON& operator=(bool b)
    {
        value = b;
        return *this;
    }

    JSON& operator=(int n)
    {
        value = n;
        return *this;
    }

    JSON& operator=(double n)
    {
        value = n;
        return *this;
    }

    JSON& operator=(const char* s)
    {
        value = std::string(s);
        return *this;
    }

    JSON& operator=(const std::string& s)
    {
        value = s;
        return *this;
    }

    JSON& operator=(const array_t& a)
    {
        value = a;
        return *this;
    }

    JSON& operator=(const object_t& o)
    {
        value = o;
        return *this;
    }

    void Save(const std::string& path);
    static JSON Load(const std::string& path);
    static JSON Parse(const std::string& json);
    std::string ToString(size_t level = 0) const;

  private:
    static void SkipWhitespace(const std::string& s, size_t& idx);
    static JSON ParseValue(const std::string& s, size_t& idx);
    static JSON ParseObject(const std::string& s, size_t& idx);
    static JSON ParseArray(const std::string& s, size_t& idx);
    static JSON ParseString(const std::string& s, size_t& idx);
    static JSON ParseNumber(const std::string& s, size_t& idx);
    static std::string EscapeString(const std::string& s);
};
