#pragma once

#include "TcpServer.h"
#include "HttpContext.h"
#include "HttpSession.h"
#include "HttpResponse.h"
#include <functional>
#include <map>
#include <memory>
#include <string>
#include <random>

class HttpServer {
public:
    using HandleFunction = std::function<void(const HttpRequest&, HttpResponse*)>;
    
    HttpServer(EventLoop* loop,
               const InetAddress& listenAddr,
               const std::string& name)
        : server_(loop, listenAddr, name)
    {
        server_.setConnectionCallback(
            std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
        server_.setMessageCallback(
            std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                     std::placeholders::_2, std::placeholders::_3));
                     
        // 注册默认路由处理函数
        registerHandler("/", HttpRequest::kGet, 
            [](const HttpRequest& req, HttpResponse* resp) {
                resp->setStatusCode(HttpResponse::k200Ok);
                resp->setStatusMessage("OK");
                resp->setContentType("text/html");
                resp->setBody("<h1>Hello, World!</h1>");
            });
    }

    void start() {
        server_.start();
    }
    
    // 注册路由处理函数
    void registerHandler(const std::string& path, 
                        HttpRequest::Method method,
                        HandleFunction handler) {
        routes_[path][method] = std::move(handler);
    }

private:
    void onConnection(const TcpConnectionPtr& conn) {
        if (conn->connected()) {
            conn->setContext(HttpContext());
        }
    }

    void onMessage(const TcpConnectionPtr& conn,
                  Buffer* buf,
                  Timestamp receiveTime) {
        HttpContext* context = std::any_cast<HttpContext>(conn->getMutableContext());

        if (!context->parseRequest(buf)) {
            conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
            conn->shutdown();
            return;
        }

        if (context->gotAll()) {
            onRequest(conn, context->request());
            context->reset();
        }
    }

    void onRequest(const TcpConnectionPtr& conn, const HttpRequest& req) {
        const std::string& connection = req.getHeader("Connection");
        bool close = connection == "close" ||
            (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");

        HttpResponse response(close);
        
        // 处理Session
        std::string sessionId = req.getHeader("Cookie");
        if (sessionId.empty()) {
            sessionId = generateSessionId();
            response.addHeader("Set-Cookie", "sessionId=" + sessionId);
        }
        
        auto session = getOrCreateSession(sessionId);
        
        // 路由分发
        handleRequest(req, &response, session);
        
        Buffer buf;
        response.appendToBuffer(&buf);
        conn->send(buf.retrieveAllAsString());
        
        if (response.closeConnection()) {
            conn->shutdown();
        }
    }
    
    void handleRequest(const HttpRequest& req, 
                      HttpResponse* resp,
                      const HttpSession::Ptr& session) {
        const std::string& path = req.path();
        auto methodHandlers = routes_.find(path);
        
        if (methodHandlers != routes_.end()) {
            auto handler = methodHandlers->second.find(req.method());
            if (handler != methodHandlers->second.end()) {
                handler->second(req, resp);
                return;
            }
        }
        
        // 404 Not Found
        resp->setStatusCode(HttpResponse::k404NotFound);
        resp->setStatusMessage("Not Found");
        resp->setContentType("text/html");
        resp->setBody("<h1>404 Not Found</h1>");
    }
    
    std::string generateSessionId() {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_int_distribution<> dis(0, 15);
        static const char* digits = "0123456789abcdef";
        
        std::string id;
        for (int i = 0; i < 32; ++i) {
            id += digits[dis(gen)];
        }
        return id;
    }
    
    HttpSession::Ptr getOrCreateSession(const std::string& sessionId) {
        auto it = sessions_.find(sessionId);
        if (it != sessions_.end()) {
            return it->second;
        }
        auto session = std::make_shared<HttpSession>(sessionId);
        sessions_[sessionId] = session;
        return session;
    }

    TcpServer server_;
    std::map<std::string, 
             std::map<HttpRequest::Method, HandleFunction>> routes_;
    std::map<std::string, HttpSession::Ptr> sessions_;
}; 