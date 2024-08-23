/**
 * This websocket server follow jsonrpc 2.0
 */

#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonValue>
#include <QRandomGenerator>
#include <QSettings>
#include <QStringList>
#include <QWebSocket>

#include "jsonrpc.h"
#include "win.h"
#include "wsserver.h"

using namespace Qt::Literals::StringLiterals;

namespace Aria2Tray {

const QString BATCH_KEY = "X-ARIA2TRAY-BATCH-ID";

WSServer::WSServer(int port, QObject *parent)
    : QObject(parent),
      m_server(new QWebSocketServer(u"Aria2Tray IPC"_s, QWebSocketServer::NonSecureMode, this))
{
    if (!m_server->listen(QHostAddress::LocalHost, port))
        return;

    connect(m_server, &QWebSocketServer::newConnection, this, &WSServer::onNewConnection);
    m_batch_hash.reserve(8);
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
        auto req_array  = json_msg.array();
        auto batch_id   = QRandomGenerator::global()->generate();
        auto batch_data = new batch_counter({req_array.size()});
        m_batch_hash.insert(batch_id, batch_data);

        for (auto req : req_array) {
            QJsonObject res_obj(res_obj_base);
            if (!req.isObject()) {
                res_obj.insert(u"id"_s, QJsonValue::Null);
                res_obj.insert(u"error"_s,
                               JsonRPC::createError(JsonRPC::InvalidRequest, "Invalid Request"));
                m_batch_hash[batch_id]->data.push_back(res_obj);
                m_batch_hash[batch_id]->use_remaining--;
                continue;
            }

            QJsonObject req_obj = req.toObject();
            if (!JsonRPC::verify(req_obj, res_obj) && req_obj.contains("id")) {
                m_batch_hash[batch_id]->data.push_back(res_obj);
                m_batch_hash[batch_id]->use_remaining--;
                continue;
            }

            req_obj.insert(BATCH_KEY, QJsonValue::fromVariant(batch_id));
            // object has pass the vibe check, now process it
            processRequest(req_obj, client);
        }
    } else {
        auto req_obj = json_msg.object();
        if (!JsonRPC::verify(req_obj, res_obj_base)) {
            client->sendTextMessage(JsonRPC::jsonStringify(res_obj_base));
            return;
        }

        processRequest(req_obj, client);
    }
}

void WSServer::setWindow(Window *main_win) { m_win = main_win; }

void WSServer::setSecret(const QString &secret) { m_secret = QString(secret); }

void WSServer::onOptionsChange()
{
    QSettings settings;
    setSecret(settings.value(u"RPCSecret"_s, "").toString());
}

void WSServer::onResultReady(struct response result, QWebSocket *client, QJsonObject req_obj)
{
    if (!req_obj.contains("id")) {
        if (req_obj.contains(BATCH_KEY)) {
            auto batch_id                   = static_cast<quint32>(req_obj[BATCH_KEY].toInteger());
            struct batch_counter *batch_ptr = m_batch_hash[batch_id];
            batch_ptr->use_remaining--;
        }

        return;
    }

    auto response_obj = QJsonObject();
    response_obj.insert("jsonrpc", "2.0");
    req_obj["id"].isString() ? response_obj.insert("id", req_obj["id"].toString())
                             : response_obj.insert("id", req_obj["id"].toInt());

    if (result.success) {
        if (!result.response_obj.isEmpty()) {
            response_obj.insert("result", result.response_obj);
        } else {
            response_obj.insert("result", result.response_str);
        }
    } else {
        response_obj.insert("error", result.response_obj);
    }

    if (req_obj.contains(BATCH_KEY)) {
        auto batch_id                   = static_cast<quint32>(req_obj[BATCH_KEY].toInteger());
        struct batch_counter *batch_ptr = m_batch_hash[batch_id];
        batch_ptr->data.push_back(response_obj);

        batch_ptr->use_remaining--;
        if (batch_ptr->use_remaining == 0) {
            client->sendTextMessage(JsonRPC::jsonStringify(batch_ptr->data));
            delete batch_ptr;
            m_batch_hash.remove(batch_id);
        }
        return;
    } else {
        client->sendTextMessage(JsonRPC::jsonStringify(response_obj));
    }
}

void WSServer::processRequest(QJsonObject &req_obj, QWebSocket *client)
{
    auto thread = new RequestProcessor(m_secret, req_obj, this);
    thread->setClient(client);
    connect(thread, &RequestProcessor::finished, thread, &RequestProcessor::deleteLater);
    connect(thread, &RequestProcessor::resultReady, this, &WSServer::onResultReady);
    thread->start();
}

RequestProcessor::RequestProcessor(const QString &secret, QJsonObject req_obj, QObject *parent)
    : QThread(parent), m_secret(secret), m_req_obj(req_obj)
{
}

void RequestProcessor::run()
{
    auto method = m_req_obj["method"].toString();
    if (method == "ping") {
        emit resultReady(methodPing(), m_client, m_req_obj);
        return;
    }

    QJsonArray params;
    struct response verify_result = verifySecret(m_req_obj, params);
    if (!verify_result.success) {
        emit resultReady(verify_result, m_client, m_req_obj);
        return;
    }

    if (method == "open") {
        emit resultReady(methodOpen(params), m_client, m_req_obj);
    } else if (method == "delete") {
        emit resultReady(methodDelete(params), m_client, m_req_obj);
    } else if (method == "status") {
        emit resultReady(methodStatus(params), m_client, m_req_obj);
    } else if (method == "version") {
        emit resultReady(methodVersion(params), m_client, m_req_obj);
    }
}

void RequestProcessor::setClient(QWebSocket *client) { m_client = client; }

/**
 * Also take care of the positional parameter
 */
struct response RequestProcessor::verifySecret(QJsonObject &request, QJsonArray &new_param)
{

    auto params = request["params"].toArray();
    if (m_secret.isEmpty()) {
        if (params[0].toString().startsWith("token:"))
            params.removeFirst();
        new_param = params;
        return {.success = true, .response_str = "OK"};
    }

    if (params.size() == 0) {
        return {false, JsonRPC::createError(JsonRPC::Unauthorized, "Empty params (token needed)")};
    }

    if (!params[0].toString().startsWith("token:")) {
        return {false, JsonRPC::createError(JsonRPC::Unauthorized, "Secret is not provided")};
    }

    auto secret = params[0].toString().sliced(6);
    if (secret != m_secret) {
        return {false, JsonRPC::createError(JsonRPC::Unauthorized, "Wrong secret")};
    }

    params.removeFirst();
    new_param = params;
    return {.success = true, .response_str = "OK"};
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
struct response RequestProcessor::methodOpen(const QJsonArray &params)
{
    if (params[0].isUndefined()) {
        return {false, JsonRPC::createError(JsonRPC::InvalidParams, "Missing argument")};
    }

    auto url = params[0].toString();
    if (url.isEmpty()) {
        return {false, JsonRPC::createError(JsonRPC::InvalidParams, "Missing required argument")};
    }

    QProcess proc;
#ifdef Q_OS_WIN32
    proc.setProgram("start");
#else
    proc.setProgram("xdg-open");
#endif // DEBUG

    proc.setArguments({url});
    proc.start();
    proc.waitForFinished(5000);

    if (proc.exitCode() != 0) {
        auto error_msg = proc.readAllStandardError();
        qDebug() << "failed to open:" << url;
        if (!error_msg.trimmed().isEmpty()) {
            return {false, JsonRPC::createError(JsonRPC::BadRequest, error_msg)};
        }

        return {false, JsonRPC::createError(JsonRPC::ServerError, "Unknown error")};
    }

    qDebug() << "opened:" << url;
    return {.success = true, .response_str = "OK"};
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
struct response RequestProcessor::methodDelete(const QJsonArray &params)
{
    // check if delete is root then cancel
    if (params[0].isUndefined()) {
        return {false, JsonRPC::createError(JsonRPC::InvalidParams, "Missing argument")};
    }

    auto info = QFileInfo(params[0].toString());
    auto path = info.canonicalFilePath();

    if (!info.exists()) {
        return {false, JsonRPC::createError(JsonRPC::NotFound, "No such file or directory")};
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
            qDebug() << "Forbidden path:" << path;
            return {false, JsonRPC::createError(JsonRPC::Forbidden, "Forbidden path")};
        }
    }

    // BEWARE
    if (info.isDir()) {
        QDir dir(path);
        if (!dir.removeRecursively()) {
            return {false, JsonRPC::createError(JsonRPC::UnprocessableContent,
                                                "Error when deleting folder")};
        }
    } else {
        QFile file(path);
        if (!file.remove()) {
            return {false, JsonRPC::createError(JsonRPC::UnprocessableContent,
                                                "Error when deleting file")};
        }
    }

    qDebug() << "deleted:" << path;
    return {.success = true, .response_str = "OK"};
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
struct response RequestProcessor::methodStatus(const QJsonArray &params)
{
    auto path = params[0].toString();
    if (path.trimmed().isEmpty()) {
        return {false, JsonRPC::createError(JsonRPC::InvalidParams, "Missing argument")};
    }

    QFileInfo file_info(path);
    QJsonObject result_obj;
    result_obj.insert("exist", file_info.exists());
    if (file_info.exists()) {
        result_obj.insert("type", file_info.isDir() ? "folder" : "file");
    }

    qDebug() << "processed status:" << path;
    return {true, result_obj};
}

/**
 * Send version of this software
 *
 * params:
 * [secret]
 *
 * result:
 * {
 *     version: string,
 * }
 */
struct response RequestProcessor::methodVersion(const QJsonArray &params)
{
    QJsonObject response_obj;
    response_obj.insert("version", A2T_VERSION);
    return {true, response_obj};
}

/**
 * Ping the server
 *
 * result:
 * "pong"
 */
struct response RequestProcessor::methodPing()
{
    qDebug() << "someone pinged";
    return {.success = true, .response_str = "pong"};
}

} // namespace Aria2Tray
