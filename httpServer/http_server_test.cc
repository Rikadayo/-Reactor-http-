#include "HttpServer.h"
#include "EventLoop.h"
#include "HttpResponse.h"
#include "DBConnectionPool.h"
#include "ConfigParser.h"
#include "RouterHandler.h"
#include "SqlConfig.h"
#include <iostream>

int main() {
    // 加载配置
    ConfigParser config;
    if (!config.load("../db_config.ini")) {
        std::cerr << "Failed to load config file" << std::endl;
        return 1;
    }
    
    // 加载SQL配置
    if (!SqlConfig::getInstance().load("../sql_config.xml")) {
        std::cerr << "Failed to load SQL config file" << std::endl;
        return 1;
    }
    
    // 初始化数据库连接池
    auto& dbPool = DBConnectionPool::getInstance();
    dbPool.init(
        config.get("database.host"),
        config.get("database.user"),
        config.get("database.password"),
        config.get("database.database"),
        std::stoi(config.get("database.port", "3306")),
        std::stoi(config.get("database.max_connections", "10"))
    );
    
    EventLoop loop;
    InetAddress listenAddr(8000, "0.0.0.0");
    HttpServer server(&loop, listenAddr, "HttpServer");
    
    // 注册所有路由
    RouterHandler::initRouter(server);
    
    server.start();
    loop.loop();
    return 0;
} 