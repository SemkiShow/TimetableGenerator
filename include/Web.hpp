// SPDX-FileCopyrightText: 2026 SemkiShow
//
// SPDX-License-Identifier: GPL-3.0-only
/**
 * @file
 * @brief Web functions
 * @version 1.1.0
 */

#pragma once

#include <filesystem>
#include <string>
#include <utility>
#include <vector>

enum class Status
{
    Continue = 100,
    SwitchingProtocols = 101,
    Processing = 102,
    EarlyHints = 103,
    OK = 200,
    Created = 201,
    Accepted = 202,
    NonAuthoritativeInformation = 203,
    NoContent = 204,
    ResetContent = 205,
    PartialContent = 206,
    MultiStatus = 207,
    AlreadyReported = 208,
    IMUsed = 226,
    MultipleChoices = 300,
    MovedPermanently = 301,
    Found = 302,
    SeeOther = 303,
    NotModified = 304,
    TemporaryRedirect = 307,
    PermanentRedirect = 308,
    BadRequest = 400,
    Unauthorized = 401,
    PaymentRequired = 402,
    Forbidden = 403,
    NotFound = 404,
    MethodNotAllowed = 405,
    NotAcceptable = 406,
    ProxyAuthenticationRequired = 407,
    RequestTimeout = 408,
    Conflict = 409,
    Gone = 410,
    LengthRequired = 411,
    PreconditionFailed = 412,
    ContentTooLarge = 413,
    URITooLong = 414,
    UnsupportedMediaType = 415,
    RangeNotSatisfiable = 416,
    ExpectationFailed = 417,
    ImATeapot = 418,
    MisdirectedRequest = 421,
    UnprocessableContent = 422,
    Locked = 423,
    FailedDependency = 424,
    TooEarly = 425,
    UpgradeRequired = 426,
    PreconditionRequired = 428,
    TooManyRequests = 429,
    RequestHeaderFieldsTooLarge = 431,
    UnavailableForLegalReasons = 451,
    InternalServerError = 500,
    NotImplemented = 501,
    BadGateway = 502,
    ServiceUnavailable = 503,
    GatewayTimeout = 504,
    HTTPVersionNotSupported = 505,
    VariantAlsoNegotiates = 506,
    InsufficientStorage = 507,
    LoopDetected = 508,
    NotExtended = 510,
    NetworkAuthenticationRequired = 511,
};

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

struct PutRequest
{
    std::string url;
    std::vector<std::string> headers;
    std::string data;
};

struct DeleteRequest
{
    std::string url;
    std::vector<std::string> headers;
};

struct PatchRequest
{
    std::string url;
    std::vector<std::string> headers;
    std::string data;
};

struct CustomRequest
{
    std::string url;
    std::vector<std::string> headers;
    std::string requestName;
};

struct CustomDataRequest
{
    std::string url;
    std::vector<std::string> headers;
    std::string data;
    std::string requestName;
};

struct Response
{
    bool success = false;
    long code;
    std::string body;
    std::string cookies;
    std::string finalUrl;
};

std::string EscapeUrl(const std::string& str);
Response PerformGet(GetRequest request, const std::string& cookies = "");
Response PerformPost(PostRequest request, const std::string& cookies = "");
Response PerformPut(PutRequest request, const std::string& cookies = "");
Response PerformDelete(DeleteRequest request, const std::string& cookies = "");
Response PerformPatch(PatchRequest request, const std::string& cookies = "");
Response PerformCustom(CustomRequest request, const std::string& cookies = "");
Response PerformCustomData(CustomDataRequest request, const std::string& cookies = "");
bool DownloadFile(const std::string& url, const std::filesystem::path& outputPath);
