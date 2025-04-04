#pragma once
#include "HttpServer.h"
#include "DBConnectionPool.h"
#include <jsoncpp/json/json.h>

class RouterHandler {
public:
    static void initRouter(HttpServer& server);

private:
    static void registerUserRoutes(HttpServer& server);
    static void handleUserRegister(const HttpRequest& req, HttpResponse* resp);

    struct UserInfo {
        std::string username;
        std::string password;
        std::string email;
    };

    static UserInfo parseUserInfo(const HttpRequest& req);
    static void saveUserToDatabase(const UserInfo& info);
    static void parseMultipartFormData(const std::string& body, UserInfo& info);
    static void parseUrlEncodedForm(const std::string& body, UserInfo& info);
    static std::string urlDecode(const std::string& encoded);
}; 