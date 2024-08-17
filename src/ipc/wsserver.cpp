/**
 * This websocket server follow jsonrpc 2.0
 */

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QSettings>
#include <QStringList>
#include <QWebSocket>

#include "jsonrpc.h"
#include "win.h"
#include "wsserver.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

WSServer::WSServer(int port, QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer(u"Aria2Tray IPC"_s, QWebSocketServer::NonSecureMode, this))
{
    if (!m_server->listen(QHostAddress::LocalHost, port))
        return;

    connect(m_server, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
}

WSServer::~WSServer()
{
    m_server->close();
    qDeleteAll(m_clients.begin(), m_clients.end());
}

bool WSServer::isListening() { return m_server->isListening(); }

void WSServer::onNewConnection()
{
    QWebSocket *new_socket = m_server->nextPendingConnection();
    qDebug() << "New connection:" << new_socket;

    connect(new_socket, &QWebSocket::textMessageReceived, this, &WSServer::onTextMessage);
    connect(new_socket, &QWebSocket::disconnected, this, &WSServer::onDisconnected);

    m_clients << new_socket;
}

void WSServer::onDisconnected()
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    qDebug() << "Socket disconnected:" << client;

    if (client) {
        m_clients.removeAll(client);
        client->deleteLater();
    }
}

void WSServer::onTextMessage(QString msg)
{
    QWebSocket *client = qobject_cast<QWebSocket *>(sender());
    if (!client)
        return;

    if (msg == u"DOPPELGANGER"_s && m_win) {
        m_win->show();
        return;
    }

    QJsonParseError err;
    auto json_msg     = QJsonDocument::fromJson(msg.toUtf8(), &err);
    auto res_obj_base = QJsonObject();
    res_obj_base.insert("jsonrpc", "2.0");

    // when invalid json
    if (json_msg.isNull()) {
        res_obj_base.insert(u"error"_s,
                            JsonRPC::createError(JsonRPC::ParseError, err.errorString()));
        res_obj_base.insert(u"id"_s, QJsonValue::Null);
        client->sendTextMessage(JsonRPC::jsonStringify(res_obj_base));
        return;
    }

    // jsonrpc batch request
    if (json_msg.isArray()) {
        auto req_array = json_msg.array();
        auto res_array = QJsonArray();

        for (auto req : req_array) {
            QJsonObject res_obj(res_obj_base);
            if (!req.isObject()) {
                res_obj.insert(u"id"_s, QJsonValue::Null);
                res_obj.insert(u"error"_s,
                               JsonRPC::createError(JsonRPC::InvalidRequest, "Invalid Request"));
                res_array.push_back(res_obj);
                continue;
            }

            QJsonObject req_obj = req.toObject();
            if (!JsonRPC::verify(req_obj, res_obj))
                goto continue_push_response;

            // object has pass the vibe check, now process it
            processRequest(req_obj, res_obj);

        continue_push_response:
            if (req_obj.contains("id"))
                res_array.push_back(res_obj);
        }

        client->sendTextMessage(JsonRPC::jsonStringify(res_array));
    } else {
        auto json_msg_obj = json_msg.object();
        if (!JsonRPC::verify(json_msg_obj, res_obj_base)) {
            client->sendTextMessage(JsonRPC::jsonStringify(res_obj_base));
            return;
        }

        processRequest(json_msg_obj, res_obj_base);
        if (json_msg_obj.contains("id"))
            client->sendTextMessage(JsonRPC::jsonStringify(res_obj_base));
    }
}

void WSServer::setWindow(Window *main_win) { m_win = main_win; }

void WSServer::setSecret(const QString &secret) { m_secret = QString(secret); }

void WSServer::onOptionsChange()
{
    QSettings settings;
    setSecret(settings.value(u"RPCSecret"_s, "").toString());
}

void WSServer::processRequest(QJsonObject &req_obj, QJsonObject &res)
{
    auto method = req_obj["method"].toString();
    if (method == "ping") {
        return methodPing(res);
    }

    QJsonArray params;
    if (!verifySecret(req_obj, res, params))
        return;

    if (method == "open") {
        return methodOpen(params, res);
    } else if (method == "delete") {
        return methodDelete(params, res);
    } else if (method == "status") {
        return methodStatus(params, res);
    }
}

/**
 * Also take care of the positional parameter
 */
bool WSServer::verifySecret(QJsonObject &request, QJsonObject &res, QJsonArray &new_param)
{

    auto params = request["params"].toArray();
    if (m_secret.isEmpty()) {
        if (params[0].toString().startsWith("token:"))
            params.removeFirst();
        new_param = params;
        return true;
    }

    if (params.size() == 0) {
        res.insert("error",
                   JsonRPC::createError(JsonRPC::Unauthorized, "Empty params (token needed)"));
        return false;
    }

    if (!params[0].toString().startsWith("token:")) {
        res.insert("error", JsonRPC::createError(JsonRPC::Unauthorized, "Secret is not provided"));
        return false;
    }

    auto secret = params[0].toString().sliced(6);
    if (secret != m_secret) {
        res.insert("error", JsonRPC::createError(JsonRPC::Unauthorized, "Wrong secret"));
        return false;
    }

    params.removeFirst();
    new_param = params;
    return true;
}

/**
 * Open file or url.
 *
 * params:
 * [secret, file | uri]
 *
 * result:
 * "OK" | jsonrpc error with reason
 *
 * error is possible in case file/folder not exist
 */
void WSServer::methodOpen(const QJsonArray &params, QJsonObject &res)
{
    if (params[0].isUndefined()) {
        res.insert("error", JsonRPC::createError(JsonRPC::InvalidParams, "Missing argument"));
        return;
    }

    auto url = params[0].toString();
    if (url.isEmpty()) {
        res.insert("error",
                   JsonRPC::createError(JsonRPC::InvalidParams, "Missing required argument"));
        return;
    }

    QProcess proc;
#ifdef Q_OS_WIN32
    proc.setProgram("start");
#else
    proc.setProgram("xdg-open");
#endif // DEBUG

    proc.setArguments({url});
    proc.start();
    proc.waitForFinished();

    if (proc.exitCode() != 0) {
        auto error_msg = proc.readAllStandardError();
        if (!error_msg.trimmed().isEmpty()) {
            res.insert("error", JsonRPC::createError(JsonRPC::BadRequest, error_msg));
        } else {
            res.insert("error", JsonRPC::createError(JsonRPC::ServerError, "Unknown error"));
        }

        qDebug() << "failed to open:" << url;
        return;
    }

    qDebug() << "opened:" << url;
    res.insert("result", "OK");
}

/**
 * Delete a file or folder (torrent).
 *
 * params:
 * [secret, file]
 *
 * result:
 * "OK" | jsonrpc error with reason
 *
 * error is possible in case file/folder can't be deleted or not exist
 *
 */
void WSServer::methodDelete(const QJsonArray &params, QJsonObject &res)
{
    // check if delete is root then cancel
    if (params[0].isUndefined()) {
        res.insert("error", JsonRPC::createError(JsonRPC::InvalidParams, "Missing argument"));
        return;
    }

    auto info = QFileInfo(params[0].toString());
    auto path = info.canonicalFilePath();

    if (!info.exists()) {
        res.insert("error", JsonRPC::createError(JsonRPC::NotFound, "No such file or directory"));
        return;
    }

    // prevention accidental deletion system (PADS)
    QStringList forbidden_pattern_list = {
        // Unix path
        R"(^/*$)",              // / || ////
        R"(^/+home/+[^/]+/*$)", // /home/user/ || //home///user
        R"(^/+[^/]+/*$)"        // /mnt/ || /d

        // Windows path
        R"(^(?:C:)?[/\\]+Users[/\\]+[^/\\]+[/\\]*$)", // C:\Users\username\ || \Users\/sdfsdf
        R"(^\w+:[/\\]*$)",                            // D:/ || C:/
    };

    // HANDLE WITH CARE
    for (auto pattern : forbidden_pattern_list) {
        QRegularExpression re(pattern, QRegularExpression::CaseInsensitiveOption);
        if (re.match(path).hasMatch()) {
            res.insert("error", JsonRPC::createError(JsonRPC::Forbidden, "Forbidden path"));
            qDebug() << "Forbidden path:" << path;
            return;
        }
    }

    // BEWARE
    if (info.isDir()) {
        QDir dir(path);
        if (!dir.removeRecursively()) {
            res.insert("error", JsonRPC::createError(JsonRPC::UnprocessableContent,
                                                     "Folder cannot be deleted"));
            return;
        }
    } else {
        QFile file(path);
        if (!file.remove()) {
            res.insert("error", JsonRPC::createError(JsonRPC::UnprocessableContent,
                                                     "File cannot be deleted"));
            return;
        }
    }

    qDebug() << "deleted:" << path;
    res.insert("result", "OK");
}

/**
 * Status of file or folder.
 *
 * params:
 * [secret, file | folder]
 *
 * result:
 * {
 *     type?: "folder" | "file",
 *     exist: boolean,
 * }
 */
void WSServer::methodStatus(const QJsonArray &params, QJsonObject &res)
{
    auto path = params[0].toString();
    if (path.trimmed().isEmpty()) {
        res.insert("error", JsonRPC::createError(JsonRPC::InvalidParams, "Missing argument"));
        return;
    }

    QFileInfo file_info(path);
    QJsonObject result_obj;
    result_obj.insert("exist", file_info.exists());
    if (file_info.exists()) {
        result_obj.insert("type", file_info.isDir() ? "folder" : "file");
    }

    qDebug() << "processed status:" << path;
    res.insert("result", result_obj);
}

/**
 * Ping the server
 *
 * result:
 * "pong"
 */
void WSServer::methodPing(QJsonObject &res)
{
    res.insert("result", "pong");
    qDebug() << "someone pinged";
}

} // namespace Aria2Tray
