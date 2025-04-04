#pragma once

#include <string>
#include <map>

class HttpRequest {
public:
    enum Method {
        kInvalid, kGet, kPost, kHead, kPut, kDelete
    };
    
    enum Version {
        kUnknown, kHttp10, kHttp11
    };

    HttpRequest() : method_(kInvalid), version_(kUnknown) {}

    void setVersion(Version v) { version_ = v; }
    Version getVersion() const { return version_; }

    bool setMethod(const char* start, const char* end) {
        std::string m(start, end);
        if (m == "GET") {
            method_ = kGet;
        } else if (m == "POST") {
            method_ = kPost;
        } else if (m == "HEAD") {
            method_ = kHead;
        } else if (m == "PUT") {
            method_ = kPut;
        } else if (m == "DELETE") {
            method_ = kDelete;
        } else {
            method_ = kInvalid;
        }
        return method_ != kInvalid;
    }

    Method method() const { return method_; }

    const std::string& path() const { return path_; }
    void setPath(const char* start, const char* end) { path_.assign(start, end); }

    void setQuery(const char* start, const char* end) { query_.assign(start, end); }
    const std::string& query() const { return query_; }

    void addHeader(const char* start, const char* colon, const char* end) {
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon)) {
            ++colon;
        }
        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size()-1])) {
            value.resize(value.size()-1);
        }
        headers_[field] = value;
    }

    std::string getHeader(const std::string& field) const {
        std::string result;
        auto it = headers_.find(field);
        if (it != headers_.end()) {
            result = it->second;
        }
        return result;
    }

    const std::map<std::string, std::string>& headers() const { return headers_; }

    void swap(HttpRequest& that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        headers_.swap(that.headers_);
    }

    void setBody(const std::string& body) { body_ = body; }
    const std::string& body() const { return body_; }

private:
    Method method_;
    Version version_;
    std::string path_;
    std::string query_;
    std::map<std::string, std::string> headers_;
    std::string body_;
}; 