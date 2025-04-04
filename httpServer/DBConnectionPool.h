#pragma once
#include <mysql/mysql.h>
#include <stdexcept>
#include <condition_variable>
#include <string>
#include <queue>
#include <mutex>
#include <memory>

class DBConnection {
public:
    DBConnection(const std::string& host, 
                const std::string& user,
                const std::string& passwd, 
                const std::string& db,
                unsigned int port = 3306) {
        mysql_ = mysql_init(nullptr);
        if (!mysql_real_connect(mysql_, host.c_str(), user.c_str(),
                              passwd.c_str(), db.c_str(), port, nullptr, 0)) {
            throw std::runtime_error(mysql_error(mysql_));
        }
    }
    
    ~DBConnection() {
        if (mysql_) {
            mysql_close(mysql_);
        }
    }
    
    MYSQL* get() { return mysql_; }
    
private:
    MYSQL* mysql_;
};

class DBConnectionPool {
public:
    static DBConnectionPool& getInstance() {
        static DBConnectionPool instance;
        return instance;
    }
    
    void init(const std::string& host, 
             const std::string& user,
             const std::string& passwd, 
             const std::string& db,
             unsigned int port = 3306,
             int maxSize = 10) {
        host_ = host;
        user_ = user;
        passwd_ = passwd;
        db_ = db;
        port_ = port;
        maxSize_ = maxSize;
        
        for (int i = 0; i < maxSize_; ++i) {
            addConnection();
        }
    }
    
    std::shared_ptr<DBConnection> getConnection() {
        std::unique_lock<std::mutex> lock(mutex_);
        while (connections_.empty()) {
            if (currentSize_ < maxSize_) {
                addConnection();
            } else {
                cv_.wait(lock);
            }
        }
        
        auto conn = std::shared_ptr<DBConnection>(
            connections_.front(),
            [this](DBConnection* conn) {
                std::unique_lock<std::mutex> lock(mutex_);
                connections_.push(conn);
                cv_.notify_one();
            });
        connections_.pop();
        return conn;
    }
    
private:
    DBConnectionPool() = default;
    
    void addConnection() {
        connections_.push(new DBConnection(host_, user_, passwd_, db_, port_));
        ++currentSize_;
    }
    
    std::string host_;
    std::string user_;
    std::string passwd_;
    std::string db_;
    unsigned int port_;
    int maxSize_;
    int currentSize_ = 0;
    
    std::queue<DBConnection*> connections_;
    std::mutex mutex_;
    std::condition_variable cv_;
}; 