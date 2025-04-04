#include "RouterHandler.h"
#include "SqlConfig.h"
#include <cstdio>
#include <sstream>

void RouterHandler::initRouter(HttpServer& server) {
    registerUserRoutes(server);
}

void RouterHandler::registerUserRoutes(HttpServer& server) {
    server.registerHandler("/api/register", HttpRequest::kPost, handleUserRegister);
}

void RouterHandler::handleUserRegister(const HttpRequest& req, HttpResponse* resp) {
    try {
        std::cout << "Headers:" << std::endl;
        for (const auto& header : req.headers()) {
            std::cout << header.first << ": " << header.second << std::endl;
        }
        std::cout << "Body length: " << req.body().length() << std::endl;
        auto userInfo = parseUserInfo(req);
        saveUserToDatabase(userInfo);
        
        resp->setStatusCode(HttpResponse::k200Ok);
        resp->setStatusMessage("OK");
        resp->setContentType("application/json");
        resp->setBody("{\"message\": \"User registered successfully\"}");
        
    } catch (const std::exception& e) {
        resp->setStatusCode(HttpResponse::k400BadRequest);
        resp->setStatusMessage("Bad Request");
        resp->setContentType("application/json");
        resp->setBody("{\"error\": \"" + std::string(e.what()) + "\"}");
    }
}

RouterHandler::UserInfo RouterHandler::parseUserInfo(const HttpRequest& req) {
    // 检查Content-Type
    std::string contentType = req.getHeader("Content-Type");
    const std::string& body = req.body();
    UserInfo info;
    
    if (contentType.find("application/json") != std::string::npos) {
        // 处理JSON格式
        Json::Value root;
        Json::Reader reader;
        if (!reader.parse(body, root)) {
            throw std::runtime_error("Invalid JSON format");
        }
        
        info.username = root["username"].asString();
        info.password = root["password"].asString();
        info.email = root["email"].asString();
    } else if (contentType.find("multipart/form-data") != std::string::npos) {
        // 处理form-data格式
        parseMultipartFormData(body, info);
    } else if (contentType.find("application/x-www-form-urlencoded") != std::string::npos) {
        // 处理URL编码格式
        parseUrlEncodedForm(body, info);
    } else {
        throw std::runtime_error("Unsupported Content-Type");
    }
    
    if (info.username.empty() || info.password.empty() || info.email.empty()) {
        throw std::runtime_error("Missing required fields");
    }
    
    return info;
}

void RouterHandler::saveUserToDatabase(const UserInfo& info) {
    auto conn = DBConnectionPool::getInstance().getConnection();
    
    // 从配置文件获取SQL语句
    std::string sqlTemplate = SqlConfig::getInstance().getSql("user.insert");
    if (sqlTemplate.empty()) {
        throw std::runtime_error("SQL template not found");
    }
    
    char sql[1024] = {0};
    snprintf(sql, sizeof(sql), 
        sqlTemplate.c_str(),
        info.username.c_str(), 
        info.password.c_str(), 
        info.email.c_str());
    
    if (mysql_query(conn->get(), sql)) {
        throw std::runtime_error(mysql_error(conn->get()));
    }
}

// 添加新的辅助函数来解析form-data
void RouterHandler::parseMultipartFormData(const std::string& body, UserInfo& info) {
    // 这里实现form-data解析
    // 注意：完整的form-data解析比较复杂，需要处理boundary等
    // 这里只是一个简单示例
}

void RouterHandler::parseUrlEncodedForm(const std::string& body, UserInfo& info) {
    std::string key, value;
    std::istringstream stream(body);
    
    while (std::getline(stream, key, '=')) {
        if (!std::getline(stream, value, '&')) {
            value = stream.str().substr(stream.tellg());
        }
        
        if (key == "username") info.username = urlDecode(value);
        else if (key == "password") info.password = urlDecode(value);
        else if (key == "email") info.email = urlDecode(value);
    }
}

std::string RouterHandler::urlDecode(const std::string& encoded) {
    // 实现URL解码
    // 这里需要处理%XX编码和+号等
    return encoded; // 简化版本
} 