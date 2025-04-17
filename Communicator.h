#pragma once
#include "json.hpp"
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

using ApiResponse = std::variant<PollResponse, PollResponseError, DeviceLoginResponseError,
                                 DeviceLoginResponseSuccess>;

template <typename TInput>
ApiResponse HttpPost(const TInput& input, const CString& host, const CString& endpoint) {
    ApiResponse output{};
    json j_input = input;
    std::string body = j_input.dump();
    CString jsonData(CA2T(body.c_str(), CP_UTF8));

    // Open internet session
    HINTERNET hInternet = InternetOpen(_T("MFCApp"), INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0);
    if (!hInternet) {
        AfxMessageBox(_T("InternetOpen failed"));
        return output;
    }

    // Connect to host
    HINTERNET hConnect = InternetConnect(hInternet, host, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL,
                                         INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        AfxMessageBox(_T("InternetConnect failed"));
        InternetCloseHandle(hInternet);
        return output;
    }

    // Open HTTP request
    HINTERNET hRequest = HttpOpenRequest(
        hConnect, _T("POST"), endpoint, NULL, NULL, NULL,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        AfxMessageBox(_T("HttpOpenRequest failed"));
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return output;
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
        AfxMessageBox(_T("HttpSendRequest failed"));
        InternetCloseHandle(hRequest);
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return output;
    }

    // Read the response
    std::string responseStr;
    char buffer[4096];
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
        } else {
            // Handle unexpected response
            AfxMessageBox(_T("Unexpected response format"));
        }
    } catch (const json::exception& e) {
        CString errorMessage;
        errorMessage.Format(_T("JSON parsing failed: %S"), e.what());
        AfxMessageBox(errorMessage, MB_OK | MB_ICONERROR);
    }

    // Clean up
    InternetCloseHandle(hRequest);
    InternetCloseHandle(hConnect);
    InternetCloseHandle(hInternet);

    return output;
}