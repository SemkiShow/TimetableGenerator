// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only

#include "Web.hpp"
#include "Logging.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <curl/curl.h>
#include <curl/easy.h>
#include <curl/system.h>
#include <filesystem>
#include <sstream>
#include <string>
#include <utility>
#include <vector>

static const char* USER_AGENT = "TimetableGenerator";

static size_t WriteCallback(void* ptr, size_t size, size_t nmemb, void* stream)
{
    ((std::string*)stream)->append((char*)ptr, size * nmemb);
    return size * nmemb;
}

std::string EscapeUrl(const std::string& str)
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        LOG_ERROR("Failed to initialize CURL");
        curl_easy_cleanup(curl);
        return "";
    }

    char* escaped = curl_easy_escape(curl, str.c_str(), static_cast<int>(str.size()));
    std::string escapedStr = escaped;

    curl_easy_cleanup(curl);
    curl_free(escaped);

    return escapedStr;
}

static std::string SaveCookies(CURL* curl)
{
    if (curl == nullptr) return "";

    std::string cookies;
    struct curl_slist* cookiesList = nullptr;

    CURLcode res = curl_easy_getinfo(curl, CURLINFO_COOKIELIST, &cookiesList);

    if (res == CURLE_OK && cookiesList != nullptr)
    {
        struct curl_slist* each = cookiesList;
        while (each != nullptr)
        {
            cookies += std::string(each->data) + "\n";
            each = each->next;
        }
        curl_slist_free_all(cookiesList);
    }

    return cookies;
}

static void LoadCookies(CURL* curl, const std::string& cookies)
{
    if (curl == nullptr) return;
    curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "");

    if (cookies.empty()) return;

    std::stringstream ss(cookies);
    std::string line;
    while (std::getline(ss, line))
    {
        if (line.empty()) continue;
        if (line.back() == '\r') line.pop_back();
        curl_easy_setopt(curl, CURLOPT_COOKIELIST, line.c_str());
    }
}

Response PerformGet(GetRequest request, const std::string& cookies)
{
    return PerformCustom(
        {
            .url = std::move(request.url),
            .headers = std::move(request.headers),
            .requestName = "GET",
        },
        cookies);
}

PostRequest::PostFields::PostFields(
    const std::vector<std::pair<std::string, std::string>>& postFields)
{
    CURL* curl = curl_easy_init();
    if (curl == nullptr)
    {
        LOG_ERROR("Failed to initialize CURL");
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

Response PerformPost(PostRequest request, const std::string& cookies)
{
    LogInfo("Performing a POST request at %s", request.url.c_str());

    CURL* curl = curl_easy_init();
    Response response;
    if (curl == nullptr)
    {
        LOG_ERROR("Failed to initialize CURL");
        return response;
    }

    struct curl_slist* headers = nullptr;
    for (auto& header: request.headers)
    {
        headers = curl_slist_append(headers, header.c_str());
    }

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    if (!request.postFields.value.empty())
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.postFields.value.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)request.postFields.value.size());
    }
    else
    {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, 0L);
    }
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (strlen(USER_AGENT) > 0) curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    LoadCookies(curl, cookies);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        LOG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        response.code = responseCode;
        response.body = readBuffer;
        response.cookies = SaveCookies(curl);

        char* finalUrl = nullptr;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &finalUrl);
        if (finalUrl != nullptr) response.finalUrl = finalUrl;

        constexpr int START_SUCCESS = 200;
        constexpr int END_SUCCESS = 300;
        if (responseCode >= START_SUCCESS && responseCode < END_SUCCESS)
        {
            LogInfo("Successfully performed a POST request at %s", request.url.c_str());
            response.success = true;
        }
        else
        {
            LOG_ERROR("HTTP request failed: Status %ld. Response: %s", responseCode,
                      readBuffer.c_str());
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

Response PerformPut(PutRequest request, const std::string& cookies)
{
    return PerformCustomData(
        {
            .url = std::move(request.url),
            .headers = std::move(request.headers),
            .data = std::move(request.data),
            .requestName = "PUT",
        },
        cookies);
}

Response PerformDelete(DeleteRequest request, const std::string& cookies)
{
    return PerformCustom(
        {
            .url = std::move(request.url),
            .headers = std::move(request.headers),
            .requestName = "DELETE",
        },
        cookies);
}

Response PerformPatch(PatchRequest request, const std::string& cookies)
{
    return PerformCustomData(
        {
            .url = std::move(request.url),
            .headers = std::move(request.headers),
            .data = std::move(request.data),
            .requestName = "PATCH",
        },
        cookies);
}

Response PerformCustom(CustomRequest request, const std::string& cookies)
{
    LogInfo("Performing a %s request at %s", request.requestName.c_str(), request.url.c_str());

    CURL* curl = curl_easy_init();
    Response response;
    if (curl == nullptr)
    {
        LOG_ERROR("Failed to initialize CURL");
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
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (strlen(USER_AGENT) > 0) curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    LoadCookies(curl, cookies);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        LOG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        response.code = responseCode;
        response.body = readBuffer;
        response.cookies = SaveCookies(curl);

        char* finalUrl = nullptr;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &finalUrl);
        if (finalUrl != nullptr) response.finalUrl = finalUrl;

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
            LOG_ERROR("HTTP request failed: Status %ld. Response: %s", responseCode,
                      readBuffer.c_str());
        }
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

Response PerformCustomData(CustomDataRequest request, const std::string& cookies)
{
    LogInfo("Performing a %s request at %s", request.requestName.c_str(), request.url.c_str());

    CURL* curl = curl_easy_init();
    Response response;
    if (curl == nullptr)
    {
        LOG_ERROR("Failed to initialize CURL");
        return response;
    }

    struct curl_slist* headers = nullptr;
    for (const auto& header: request.headers) headers = curl_slist_append(headers, header.c_str());

    struct ReadContext
    {
        std::string data;
        size_t offset = 0;
    };
    ReadContext ctx = {request.data, 0};

    // NOLINTNEXTLINE (bugprone-easily-swappable-parameters)
    static auto readFn = [](char* buf, size_t size, size_t nmemb, void* userdata) -> size_t
    {
        auto* ctx = static_cast<ReadContext*>(userdata);
        size_t remaining = std::min(size * nmemb, ctx->data.size() - ctx->offset);
        if (remaining > 0)
        {
            // NOLINTNEXTLINE(bugprone-not-null-terminated-result)
            memcpy(buf, ctx->data.data() + ctx->offset, remaining);
            ctx->offset += remaining;
        }
        return remaining;
    };

    std::string readBuffer;
    curl_easy_setopt(curl, CURLOPT_URL, request.url.c_str());
    curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
    curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, request.requestName.c_str());
    curl_easy_setopt(curl, CURLOPT_INFILESIZE_LARGE, (curl_off_t)request.data.size());
    curl_easy_setopt(curl, CURLOPT_READFUNCTION, (curl_read_callback)readFn);
    curl_easy_setopt(curl, CURLOPT_READDATA, &ctx);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    if (strlen(USER_AGENT) > 0) curl_easy_setopt(curl, CURLOPT_USERAGENT, USER_AGENT);
    LoadCookies(curl, cookies);

    CURLcode res = curl_easy_perform(curl);

    if (res != CURLE_OK)
    {
        LOG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        long responseCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &responseCode);
        response.code = responseCode;
        response.body = readBuffer;
        response.cookies = SaveCookies(curl);

        char* finalUrl = nullptr;
        curl_easy_getinfo(curl, CURLINFO_EFFECTIVE_URL, &finalUrl);
        if (finalUrl != nullptr) response.finalUrl = finalUrl;

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
            LOG_ERROR("HTTP request failed: Status %ld. Response: %s", responseCode,
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
        LOG_ERROR("Failed to initialize CURL");
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
