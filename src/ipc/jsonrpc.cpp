#include <QJsonDocument>
#include <QJsonValue>

#include "jsonrpc.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

/**
 * Verify request validity by checking required field
 * On error, error will be set in responseObject
 * id key is already set if possible
 */
bool JsonRPC::verify(const QJsonObject &req, QJsonObject &response_obj)
{
    if (req.contains("id")) {
        auto is_double = req["id"].isDouble();
        auto is_string = req["id"].isString();
        if (!is_double && !is_string) {
            response_obj.insert(u"error"_s,
                                createError(InvalidRequest, "id must be integer or string"));
            response_obj.insert(u"id"_s, QJsonValue::Null);
            return false;
        }
        is_string ? response_obj.insert("id", req["id"].toString())
                  : response_obj.insert("id", req["id"].toInteger());
    }

    if (req["jsonrpc"].toString() != "2.0") {
        response_obj.insert(u"error"_s,
                            createError(InvalidRequest, "Server only support jsonrpc 2.0"));
        return false;
    }

    // uncomment if support named parameter
    // if (req.contains("params") && !req["params"].isObject() && !req["params"].isArray()) {
    if (req.contains("params") && !req["params"].isArray()) {
        response_obj.insert(u"error"_s, createError(InvalidRequest, "Invalid params"));
        return false;
    }

    if (!req["method"].isString()) {
        response_obj.insert(u"error"_s, createError(InvalidRequest, "Invalid method"));
        return false;
    }

    return true;
}

QJsonObject JsonRPC::createError(enum Error code, const QString &msg)
{
    QJsonObject error_obj;
    error_obj.insert("code", code);
    error_obj.insert("message", msg);
    return error_obj;
}

QByteArray JsonRPC::jsonStringify(const QJsonObject &obj)
{
    return QJsonDocument(obj).toJson(QJsonDocument::Compact);
}

QByteArray JsonRPC::jsonStringify(const QJsonArray &arr)
{
    return QJsonDocument(arr).toJson(QJsonDocument::Compact);
}

} // namespace Aria2Tray
