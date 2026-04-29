// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Web.hpp"
#include "Logging.hpp"
#include "Settings.hpp"
#include <cstddef>
#include <cstdio>
#include <curl/curl.h>
#include <curl/easy.h>
#include <filesystem>
#include <string>
#include <utility>
#include <vector>

static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    ((std::string*)stream)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

Response PerformGet(GetRequest request)
{
    LogInfo("Performing a GET request at %s", request.url.c_str());

    CURL* curl = curl_easy_init();
    Response response;
    if (curl == nullptr)
    {
        LogError("Failed to initialize CURL");
        curl_easy_cleanup(curl);
        return response;
    }

    struct curl_slist* headers = nullptr;
    for (auto& header: request.headers)
    {
        headers = curl_slist_append(headers, header.c_str());
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        LogError("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        response.response = readBuffer;

        constexpr int START_SUCCESS = 200;
        constexpr int END_SUCCESS = 300;
        if (responseCode >= START_SUCCESS && responseCode < END_SUCCESS)
        {
            LogInfo("Successfully performed a GET request at %s", request.url.c_str());
            response.success = true;
        }
        else
        {
            LogError("HTTP request failed: Status %ld. Response: %s", responseCode,
                     readBuffer.c_str());
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

PostRequest::PostFields::PostFields(
    const std::vector<std::pair<std::string, std::string>>& postFields)
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        LogError("Failed to initialize CURL");
        curl_easy_cleanup(curl);
        return;
    }

    for (size_t i = 0; i < postFields.size(); i++)
    {
        const auto& field = postFields[i];
        char* fieldValueEsc =
            curl_easy_escape(curl, field.second.c_str(), static_cast<int>(field.second.size()));
        value += field.first + "=" + fieldValueEsc;
        if (i + 1 < postFields.size()) value += "&";
        curl_free(fieldValueEsc);
    }

    curl_easy_cleanup(curl);
}

Response PerformPost(PostRequest request)
{
    LogInfo("Performing a POST request at %s", request.url.c_str());

    CURL* curl = curl_easy_init();
    Response response;
    if (curl == nullptr)
    {
        LogError("Failed to initialize CURL");
        curl_easy_cleanup(curl);
        return response;
    }

    struct curl_slist* headers = nullptr;
    for (auto& header: request.headers)
    {
        headers = curl_slist_append(headers, header.c_str());
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
    if (!request.postFields.value.empty())
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.postFields.value.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        LogError("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        response.response = readBuffer;

        constexpr int START_SUCCESS = 200;
        constexpr int END_SUCCESS = 300;
        if (responseCode >= START_SUCCESS && responseCode < END_SUCCESS)
        {
            LogInfo("Successfully performed a POST request at %s", request.url.c_str());
            response.success = true;
        }
        else
        {
            LogError("HTTP request failed: Status %ld. Response: %s", responseCode,
                     readBuffer.c_str());
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

Response PerformCustom(CustomRequest request)
{
    LogInfo("Performing a %s request at %s", request.requestName.c_str(), request.url.c_str());

    CURL* curl = curl_easy_init();
    Response response;
    if (curl == nullptr)
    {
        LogError("Failed to initialize CURL");
        curl_easy_cleanup(curl);
        return response;
    }

    struct curl_slist* headers = nullptr;
    for (auto& header: request.headers)
    {
        headers = curl_slist_append(headers, header.c_str());
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.requestName.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_USERAGENT, ("TimetableGenerator/" + version).c_str());

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        LogError("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        response.response = readBuffer;

        constexpr int START_SUCCESS = 200;
        constexpr int END_SUCCESS = 300;
        if (responseCode >= START_SUCCESS && responseCode < END_SUCCESS)
        {
            LogInfo("Successfully performed a %s request at %s", request.requestName.c_str(),
                    request.url.c_str());
            response.success = true;
        }
        else
        {
            LogError("HTTP request failed: Status %ld. Response: %s", responseCode,
                     readBuffer.c_str());
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

static size_t WriteFileCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    return fwrite(ptr, size, nmemb, (FILE*)stream);
}

bool DownloadFile(const std::string& url, const std::filesystem::path& outputPath)
{
    LogInfo("Downloading the file %s", url.c_str());
    FILE* file = fopen(outputPath.string().c_str(), "wb");
    if (!file) return false;

    CURL* curl = curl_easy_init();
    if (!curl)
    {
        LogError("Failed to initialize CURL");
        fclose(file);
        return false;
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, file);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteFileCallback);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    fclose(file);
    return res == CURLE_OK;
}
