#include "HttpRequest.h"
#include "HttpContext.h"
#include "Buffer.h"
#include <algorithm>

bool HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    
    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        
        if (space != end) {
            const char* question = std::find(start, space, '?');
            if (question != space) {
                request_.setPath(start, question);
                request_.setQuery(question + 1, space);
            } else {
                request_.setPath(start, space);
            }
            start = space + 1;
            
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::kHttp11);
                } else if (*(end - 1) == '0') {
                    request_.setVersion(HttpRequest::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

bool HttpContext::parseRequest(Buffer* buf) {
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {
        if (state_ == kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    buf->retrieveUntil(crlf + 2);
                    state_ = kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (state_ == kExpectHeaders) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    // 空行，headers结束
                    state_ = kExpectBody;
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore = false;
            }
        } else if (state_ == kExpectBody) {
            // 获取Content-Length
            std::string lengthStr = request_.getHeader("Content-Length");
            if (!lengthStr.empty()) {
                size_t length = std::stoul(lengthStr);
                if (buf->readableBytes() >= length) {
                    request_.setBody(std::string(buf->peek(), length));
                    buf->retrieve(length);
                    state_ = kGotAll;
                }
            } else {
                state_ = kGotAll;
            }
            hasMore = false;
        }
    }
    return ok;
} 