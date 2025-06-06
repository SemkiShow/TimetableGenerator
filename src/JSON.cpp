#include "JSON.hpp"

int indentationLevel = 0;
#define INDENTATION std::string(indentationLevel, '\t')
std::string JSONToString(JSONObject jsonObject)
{
    std::string output = "";
    output += (jsonObject.type == JSON_OBJECT ? "{" : "[");
    if (jsonObject.format == JSON_NEWLINE) output += '\n';
    indentationLevel++;
    if (jsonObject.type == JSON_LIST)
    {
        for (int i = 0; i < jsonObject.strings.size(); i++)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += '"' + jsonObject.strings[i] + '"';
            if (i < jsonObject.strings.size()-1 ||
                jsonObject.ints.size() > 0 ||
                jsonObject.bools.size() > 0 ||
                jsonObject.objects.size() > 0) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
        }
        for (int i = 0; i < jsonObject.ints.size(); i++)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += std::to_string(jsonObject.ints[i]);
            if (i < jsonObject.ints.size()-1 ||
                jsonObject.bools.size() > 0 ||
                jsonObject.objects.size() > 0) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
        }
        for (int i = 0; i < jsonObject.bools.size(); i++)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += (jsonObject.bools[i] ? "true" : "false");
            if (i < jsonObject.bools.size()-1 ||
            jsonObject.objects.size() > 0) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
        }
        for (int i = 0; i < jsonObject.objects.size(); i++)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += JSONToString(jsonObject.objects[i]);
            if (i < jsonObject.objects.size()-1) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
        }
    }
    if (jsonObject.type == JSON_OBJECT)
    {
        int i = 0;
        for (auto pair: jsonObject.stringPairs)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += '"' + pair.first + "\": " + '"' + pair.second + '"';
            if (i < jsonObject.stringPairs.size()-1 ||
                jsonObject.intPairs.size() > 0 ||
                jsonObject.boolPairs.size() > 0 ||
                jsonObject.objectPairs.size() > 0) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
            i++;
        }
        i = 0;
        for (auto pair: jsonObject.intPairs)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += '"' + pair.first + "\": " + std::to_string(pair.second);
            if (i < jsonObject.intPairs.size()-1 ||
                jsonObject.boolPairs.size() > 0 ||
                jsonObject.objectPairs.size() > 0) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
            i++;
        }
        i = 0;
        for (auto pair: jsonObject.boolPairs)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += '"' + pair.first + "\": " + (pair.second ? "true" : "false");
            if (i < jsonObject.boolPairs.size()-1 ||
                jsonObject.objectPairs.size() > 0) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
            i++;
        }
        i = 0;
        for (auto pair: jsonObject.objectPairs)
        {
            if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
            output += '"' + pair.first + "\": " + JSONToString(pair.second);
            if (i < jsonObject.objectPairs.size()-1) output += ", ";
            if (jsonObject.format == JSON_NEWLINE) output += '\n';
            i++;
        }
    }
    indentationLevel--;
    if (jsonObject.format == JSON_NEWLINE) output += INDENTATION;
    output += (jsonObject.type == JSON_OBJECT ? "}" : "]");
    return output;
}

void SaveJSON(std::string path, JSONObject* jsonObject)
{
    std::ofstream timetableFile(path);
    indentationLevel = 0;
    timetableFile << JSONToString(*jsonObject);
    timetableFile.close();
    std::cout << "Saved " << path << "\n";
}

std::string TrimSpaces(const std::string& input)
{
    auto first = input.find_first_not_of(" \t\n\r\f\v");
    auto last  = input.find_last_not_of (" \t\n\r\f\v");
    return (first == input.npos) ? "" : input.substr(first, last-first+1);
}

std::string TrimQuotes(const std::string& input)
{
    auto first = input.find_first_not_of("\"\t\n\r\f\v");
    auto last  = input.find_last_not_of ("\"\t\n\r\f\v");
    return (first == input.npos) ? "" : input.substr(first, last-first+1);
}

bool IsNumber(std::string input)
{
    try
    {
        int a = stoi(input);
        return true;
    }
    catch (const std::exception&)
    {
        return false;
    }
}

std::vector<std::string> Split(std::string input, char delimiter = ' ', int limit = -1)
{
    std::vector<std::string> output;
    output.push_back("");
    int index = 0;
    for (int i = 0; i < input.size(); i++)
    {
        if (input[i] == delimiter && (index < limit && limit != -1))
        {
            index++;
            output.push_back("");
            continue;
        }
        output[index] += input[i];
    }
    return output;
}

void ParseJSON(std::string json, JSONObject* jsonObject)
{
    int bracketLevel = 0;
    int startIndex = 0;
    std::vector<std::string> values;
    for (int i = 0; i < json.size(); i++)
    {
        if (json[i] == '{' || json[i] == '[')
        {
            bracketLevel++;
            if (bracketLevel == 1)
            {
                jsonObject->type = (json[i] == '{' ? JSON_OBJECT : JSON_LIST);
                startIndex = i+1;
            }
        }
        if (json[i] == '}' || json[i] == ']')
        {
            if (bracketLevel == 1 && json.size() > 2) values.push_back(json.substr(startIndex, i - startIndex));
            bracketLevel--;
            if (bracketLevel <= 0) break;
        }
        if (json[i] == ',' && bracketLevel == 1)
        {
            values.push_back(json.substr(startIndex, i - startIndex));
            startIndex = i+1;
        }
    }
    for (int i = 0; i < values.size(); i++)
    {
        values[i] = TrimSpaces(values[i]);
        if (jsonObject->type == JSON_LIST)
        {
            if (values[i][0] == '{' || values[i][0] == '[')
            {
                jsonObject->objects.push_back(JSONObject());
                ParseJSON(values[i], &jsonObject->objects[jsonObject->objects.size()-1]);
            }
            else if (values[i] == "true" || values[i] == "false") jsonObject->bools.push_back(values[i] == "true");
            else if (IsNumber(values[i])) jsonObject->ints.push_back(stoi(values[i]));
            else
                jsonObject->strings.push_back(TrimQuotes(values[i]));
        }
        if (jsonObject->type == JSON_OBJECT)
        {
            std::vector<std::string> keyValuePair = Split(values[i], ':', 2);
            for (int i = 0; i < keyValuePair.size(); i++)
                keyValuePair[i] = TrimSpaces(keyValuePair[i]);
            if (keyValuePair[0].size() == 0 || keyValuePair[1].size() == 0) return;

            if (keyValuePair[1][0] == '{' || keyValuePair[1][0] == '[')
            {
                jsonObject->objectPairs[TrimQuotes(keyValuePair[0])] = JSONObject();
                ParseJSON(values[i], &jsonObject->objectPairs[TrimQuotes(keyValuePair[0])]);
            }
            else if (keyValuePair[1] == "true" || keyValuePair[1] == "false")
                jsonObject->boolPairs[TrimQuotes(keyValuePair[0])] = keyValuePair[1] == "true";
            else if (IsNumber(keyValuePair[1]))
                jsonObject->intPairs[TrimQuotes(keyValuePair[0])] = stoi(keyValuePair[1]);
            else
                jsonObject->stringPairs[TrimQuotes(keyValuePair[0])] = TrimQuotes(keyValuePair[1]);
        }
    }
}

void LoadJSON(std::string path, JSONObject* jsonObject)
{
    std::ifstream timetableFile(path);
    std::string buf, json;
    while (std::getline(timetableFile, buf))
        for (int i = 0; i < buf.size(); i++) if (buf[i] != '\t') json += buf[i];
    timetableFile.close();

    ParseJSON(json, jsonObject);

    std::cout << "Loaded " << path << "\n";
}
