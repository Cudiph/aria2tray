/**
 * jsonrpc utility function
 */

#ifndef ARIA2TRAY_JSONRPC_H_
#define ARIA2TRAY_JSONRPC_H_

#include <QJsonObject>

namespace Aria2Tray {

class JsonRPC {

public:
    enum Error {
        ParseError     = -32700,
        InvalidRequest = -32600,
        MethodNotFound = -32601,
        InvalidParams  = -32602,
        InternalError  = -32603,
        ServerError    = -32000,

        // derived from http status code
        BadRequest           = 400,
        Unauthorized         = 401,
        Forbidden            = 403,
        NotFound             = 404,
        UnprocessableContent = 422,
    };

    static bool verify(const QJsonObject &req, QJsonObject &response_object);
    static QJsonObject createError(enum Error code, const QString &msg);
    static QByteArray jsonStringify(const QJsonObject &obj);
    static QByteArray jsonStringify(const QJsonArray &arr);
};

} // namespace Aria2Tray

#endif // !ARIA2TRAY_JSONRPC_H_
