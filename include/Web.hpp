// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief Web functions
 */

#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

struct GetRequest
{
    std::string url;
    std::vector<std::string> headers;
};

struct PostRequest
{
    struct PostFields
    {
        PostFields(std::string postFields) : value(std::move(postFields)) {}
        PostFields(const std::vector<std::pair<std::string, std::string>>& postFields);

        std::string value;
    };

    std::string url;
    std::vector<std::string> headers;
    PostFields postFields;
};

struct CustomRequest
{
    std::string url;
    std::vector<std::string> headers;
    std::string requestName;
};

struct Response
{
    bool success = false;
    std::string response;
};

Response PerformGet(GetRequest request);
Response PerformPost(PostRequest request);
Response PerformCustom(CustomRequest request);
bool DownloadFile(const std::string& url, const std::filesystem::path& outputPath);
