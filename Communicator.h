#pragma once
#include <string>
#include <wininet.h>
#include "json.hpp"
#include <variant>

using json = nlohmann::json;

#pragma comment(lib, "wininet.lib")



// Define the error codes enum
enum class ErrorCodes {
    authorization_pending,
    invalid_request
};

// Convert between enum and string
NLOHMANN_JSON_SERIALIZE_ENUM(ErrorCodes, {
    {ErrorCodes::authorization_pending, "authorization_pending"},
    {ErrorCodes::invalid_request, "invalid_request"}
})

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
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PollResponse, user_id, name, email)

// Define the PollResponseError struct
struct PollResponseError {
    ErrorCodes error;
};

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(PollResponseError, error)


template<typename TInput>
std::variant<PollResponse, PollResponseError>
HttpPost(const TInput& input, const CString& host, const CString& endpoint)
{
    std::variant<PollResponse, PollResponseError> output{};
    json j_input = input;
    std::string body = j_input.dump();

    // Convert UTF-8 string to wide string
    int wideLength = MultiByteToWideChar(CP_UTF8, 0, body.c_str(), -1, nullptr, 0);
    std::wstring wBody(wideLength, 0);
    MultiByteToWideChar(CP_UTF8, 0, body.c_str(), -1, &wBody[0], wideLength);

    // Open internet session
    HINTERNET hInternet = InternetOpen(_T("MFCApp"), INTERNET_OPEN_TYPE_DIRECT, nullptr, nullptr, 0);
    if (!hInternet) {
        AfxMessageBox(_T("InternetOpen failed"));
        return output;
    }

    // Connect to host
    HINTERNET hConnect = InternetConnect(hInternet, host, INTERNET_DEFAULT_HTTPS_PORT,
        nullptr, nullptr, INTERNET_SERVICE_HTTP, 0, 0);
    if (!hConnect) {
        AfxMessageBox(_T("InternetConnect failed"));
        InternetCloseHandle(hInternet);
        return output;
    }

    // Open HTTP request
    HINTERNET hRequest = HttpOpenRequest(hConnect, _T("POST"), endpoint, nullptr, nullptr, nullptr,
        INTERNET_FLAG_RELOAD | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_SECURE, 0);
    if (!hRequest) {
        AfxMessageBox(_T("HttpOpenRequest failed"));
        InternetCloseHandle(hConnect);
        InternetCloseHandle(hInternet);
        return output;
    }

    // Set headers
    LPCTSTR headers = _T("Content-Type: application/json\r\n");

    // Convert wide string to UTF-8 for sending
    CW2A utf8Body(wBody.c_str(), CP_UTF8);
    LPCSTR lpUtf8Body = utf8Body;
    DWORD dwBodyLength = static_cast<DWORD>(strlen(lpUtf8Body));

    // Send the request
    BOOL bSuccess = HttpSendRequest(hRequest, headers, -1L, (LPVOID)lpUtf8Body, dwBodyLength);
    if (!bSuccess) {
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

        if (j.contains("error_code")) {
            output = j.get<PollResponseError>();
        }
        else {
            output = j.get<PollResponse>();
        }
    }
    catch (const json::exception& e) {
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