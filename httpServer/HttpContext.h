#pragma once

#include "Buffer.h"
#include "HttpRequest.h"

class HttpContext {
public:
    enum HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

    HttpContext() : state_(kExpectRequestLine) {}

    // 解析请求是否成功
    bool parseRequest(Buffer* buf);
    
    // 是否得到了一个完整的请求
    bool gotAll() const { return state_ == kGotAll; }

    // 重置HttpContext状态
    void reset() {
        state_ = kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    const HttpRequest& request() const { return request_; }
    HttpRequest& request() { return request_; }

private:
    // 解析请求行
    bool processRequestLine(const char* begin, const char* end);

    HttpRequestParseState state_;
    HttpRequest request_;
}; 