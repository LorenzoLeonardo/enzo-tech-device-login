#pragma once
#include "json.hpp"
#include "utils.h"
#include <atlconv.h>
#include <optional>
#include <string>
#include <variant>
#include <wininet.h>

using json = nlohmann::json;

#pragma comment(lib, "wininet.lib")

// Define the error codes enum
enum class ErrorCodes {
    authorization_pending,
    invalid_request,
    server_error,
    invalid_grant,
    internal_error,
    invalid_json
};

// Convert between enum and string
NLOHMANN_JSON_SERIALIZE_ENUM(ErrorCodes,
                             {{ErrorCodes::authorization_pending, "authorization_pending"},
                              {ErrorCodes::invalid_request, "invalid_request"},
                              {ErrorCodes::server_error, "server_error"},
                              {ErrorCodes::invalid_grant, "invalid_grant"},
                              {ErrorCodes::internal_error, "internal_error"},
                              {ErrorCodes::invalid_json, "invalid_json"}})

// Define the PollRequest struct
struct PollRequest {
    std::string session_id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PollRequest, session_id)

// Define the PollResponse struct
struct PollResponse {
    std::string user_id;
    std::string name;
    std::string email;
    std::string login_status;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PollResponse, user_id, name, email, login_status)

// Define the PollResponseError struct
struct PollResponseError {
    ErrorCodes error;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PollResponseError, error)

struct DeviceLoginResponseError {
    bool success;
    std::string error;
    ErrorCodes error_code;
    std::string login_status;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeviceLoginResponseError, success, error, error_code,
                                   login_status)

struct DeviceLoginResponseSuccess {
    bool success;
    std::string login_status;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeviceLoginResponseSuccess, success, login_status)

struct DeviceEvent {
    std::string session_id;
    std::string user_id;
    std::string username;
    std::string timestamp;
    std::string action;
    std::string device_id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DeviceEvent, session_id, user_id, username, timestamp, action,
                                   device_id)

struct HttpError {
    DWORD status_code;
    std::string http_error;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(HttpError, status_code, http_error)

#define RETURN_HTTP_ERROR(error, msg)                                                              \
    do {                                                                                           \
        TRACE(traceAppMsg, 0, "Error: (%d)%s.\n", error, msg.c_str());                             \
        return HttpError{error, msg};                                                              \
    } while (0)

struct PackageName {
    std::string Name;
    std::string Version;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PackageName, Name, Version)

struct LogoutSession {
    std::string session_id;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LogoutSession, session_id)

struct LogoutSessionResponse {
    bool success;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(LogoutSessionResponse, success)

using ApiResponse =
    std::variant<PollResponse, PollResponseError, DeviceLoginResponseError,
                 DeviceLoginResponseSuccess, PackageName, LogoutSessionResponse, HttpError>;

template <typename TInput>
ApiResponse HttpPost(const TInput& input, const CString& host, const CString& endpoint) {
    ApiResponse output{};
    json j_input = input;
    std::string body = j_input.dump();
    CString jsonData(CA2T(body.c_str(), CP_UTF8));
    // Open internet session
    HINTERNET hInternet = InternetOpen(_T("MFCApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        RETURN_HTTP_ERROR(error, msg);
    }

    // Connect to host
    HINTERNET hConnect = InternetConnect(hInternet, host, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL,
                                         INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        InternetCloseHandle(hInternet);
        RETURN_HTTP_ERROR(error, msg);
    }

    // Open HTTP request
    HINTERNET hRequest = HttpOpenRequest(
        hConnect, _T("POST"), endpoint, NULL, NULL, NULL,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        RETURN_HTTP_ERROR(error, msg);
    }

    // Convert jsonData (CString - UTF-16) to UTF-8
    CW2A utf8Json(jsonData, CP_UTF8); // convert wide string to UTF-8
    LPCSTR utf8Body = utf8Json;
    int utf8Length = (int)strlen(utf8Body);

    // Send the request with UTF-8 body
    LPCTSTR headers = _T("Content-Type: application/json\r\n");
    BOOL success = HttpSendRequest(hRequest, headers, -1L, (LPVOID)utf8Body,
                                   utf8Length); // size in bytes (UTF-8)

    if (!success) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        RETURN_HTTP_ERROR(error, msg);
    }

    // Read the response
    std::string responseStr;
    char buffer[4096] = {};
    DWORD bytesRead = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
        buffer[bytesRead] = '\0';
        responseStr.append(buffer, bytesRead);
    }

    // Parse JSON response
    try {
        json j = json::parse(responseStr);

        if (j.contains("success") && j.contains("error") && !j["error"].is_null() &&
            j.contains("error_code") && !j["error_code"].is_null() && j.contains("login_status")) {
            output = j.get<DeviceLoginResponseError>();
        } else if (j.contains("success") && j.contains("error") && j["error"].is_null() &&
                   j.contains("error_code") && j["error_code"].is_null() &&
                   j.contains("login_status")) {
            output = j.get<DeviceLoginResponseSuccess>();
        } else if (j.contains("error")) {
            output = j.get<PollResponseError>();
        } else if (j.contains("user_id") && j.contains("name") && j.contains("email") &&
                   j.contains("login_status")) {
            output = j.get<PollResponse>();
        } else if (j.contains("success")) {
            output = j.get<LogoutSessionResponse>();
        } else {
            // Handle unexpected response
            DWORD error = GetLastError();
            output = HttpError{error, "Invalid JSON from server."};
        }
    } catch (const json::exception& e) {
        std::string errorMessage = std::format("JSON parsing failed: {}!", e.what());

        DWORD error = GetLastError();
        output = HttpError{error, errorMessage};
    }

    // Clean up
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return output;
}

template <typename TInput>
ApiResponse HttpGet(const TInput& input, const CString& host, const CString& endpoint) {
    ApiResponse output{};

    // Open internet session
    HINTERNET hInternet = InternetOpen(_T("MFCApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        RETURN_HTTP_ERROR(error, msg);
    }

    // Connect to host
    HINTERNET hConnect = InternetConnect(hInternet, host, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL,
                                         INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        InternetCloseHandle(hInternet);
        RETURN_HTTP_ERROR(error, msg);
    }

    // Open HTTP GET request
    HINTERNET hRequest = HttpOpenRequest(
        hConnect, _T("GET"), endpoint, NULL, NULL, NULL,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        RETURN_HTTP_ERROR(error, msg);
    }

    // Send the GET request (no body)
    BOOL success = HttpSendRequest(hRequest, NULL, 0, NULL, 0); // No headers or body

    if (!success) {
        DWORD error = GetLastError();
        std::string msg = GetLastErrorString(error);

        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        RETURN_HTTP_ERROR(error, msg);
    }

    // Read the response
    std::string responseStr;
    char buffer[4096] = {};
    DWORD bytesRead = 0;
    while (InternetReadFile(hRequest, buffer, sizeof(buffer) - 1, &bytesRead) && bytesRead) {
        buffer[bytesRead] = '\0';
        responseStr.append(buffer, bytesRead);
    }

    // Parse JSON response
    try {
        json j = json::parse(responseStr);

        if (j.contains("PackageName")) {
            json packageJson = j["PackageName"];
            PackageName pkg = packageJson.get<PackageName>();

            output = pkg;
        } else {
            DWORD error = GetLastError();
            output = HttpError{error, "PackageName not found"};
        }

    } catch (const json::exception& e) {
        std::string errorMessage = std::format("JSON parsing failed: {}!", e.what());

        DWORD error = GetLastError();
        output = HttpError{error, errorMessage};
    }

    // Clean up
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return output;
}