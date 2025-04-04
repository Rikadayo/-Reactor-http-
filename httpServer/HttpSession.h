#pragma once
#include <string>
#include <map>
#include <memory>

class HttpSession {
public:
    using Ptr = std::shared_ptr<HttpSession>;
    
    HttpSession(const std::string& id) : sessionId_(id) {}
    
    void setAttribute(const std::string& key, const std::string& value) {
        attributes_[key] = value;
    }
    
    std::string getAttribute(const std::string& key) const {
        auto it = attributes_.find(key);
        return it != attributes_.end() ? it->second : "";
    }
    
    const std::string& getSessionId() const { return sessionId_; }
    
private:
    std::string sessionId_;
    std::map<std::string, std::string> attributes_;
}; 