#include <string>
#include <vector>
#include <map>
#include <fstream>
#include <iostream>

enum JSONObjectType
{
    JSON_OBJECT,
    JSON_LIST
};

enum JSONObjectFormat
{
    JSON_NEWLINE,
    JSON_INLINE
};

struct JSONObject
{
    JSONObjectType type = JSON_OBJECT;
    JSONObjectFormat format = JSON_NEWLINE;
    std::vector<std::string> strings;
    std::vector<int> ints;
    std::vector<bool> bools;
    std::vector<JSONObject> objects;
    std::map<std::string, std::string> stringPairs;
    std::map<std::string, int> intPairs;
    std::map<std::string, bool> boolPairs;
    std::map<std::string, JSONObject> objectPairs;
};

void SaveJSON(std::string path, JSONObject* jsonObject);
void LoadJSON(std::string path, JSONObject* jsonObject);
