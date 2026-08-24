#include "Translator.h"
#include "ConfigManager.h"
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QDateTime>
#include <QDebug>

Translator::Translator(QObject* parent) : QObject(parent) {
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &Translator::onReplyFinished);
}

QString Translator::generateSign(const QString& query, const QString& salt,
                                 const QString& appId, const QString& secretKey) {
    QString raw = appId + query + salt + secretKey;
    QByteArray hash = QCryptographicHash::hash(raw.toUtf8(), QCryptographicHash::Md5);
    return hash.toHex();
}

void Translator::translate(const QString& text, const QString& from, const QString& to) {
    if (text.isEmpty()) {
        emit translationError("翻译文本为空");
        return;
    }

    // ★ 填你的密钥
    QString appId = ConfigManager::instance().getBaiduAppId();
    QString secretKey = ConfigManager::instance().getBaiduSecretKey();

    if (appId.isEmpty() || secretKey.isEmpty()) {
        emit translationError("请先在代码中配置百度翻译 API Key");
        return;
    }

    QString salt = QString::number(QDateTime::currentSecsSinceEpoch());
    QString sign = generateSign(text, salt, appId, secretKey);

    QUrl url("https://fanyi-api.baidu.com/api/trans/vip/translate");
    QUrlQuery query;
    query.addQueryItem("q", text);
    query.addQueryItem("from", from.isEmpty() ? "auto" : from);   // auto = 自动识别语种
    query.addQueryItem("to", to.isEmpty() ? "zh" : to);
    query.addQueryItem("appid", appId);
    query.addQueryItem("salt", salt);
    query.addQueryItem("sign", sign);

    url.setQuery(query);

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    m_networkManager->get(request);
    qDebug() << "Translator: sending request to Baidu API";
}

void Translator::onReplyFinished(QNetworkReply* reply) {
    if (reply->error() != QNetworkReply::NoError) {
        emit translationError(QString("网络错误: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    qDebug() << "Translator: response received:" << response;

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (doc.isNull()) {
        emit translationError("解析响应失败");
        return;
    }

    QJsonObject obj = doc.object();
    if (obj.contains("error_code")) {
        int errorCode = obj["error_code"].toInt();
        QString errorMsg = obj["error_msg"].toString();
        emit translationError(QString("翻译API错误 (%1): %2").arg(errorCode).arg(errorMsg));
        return;
    }

    if (obj.contains("trans_result")) {
        QJsonArray results = obj["trans_result"].toArray();
        if (!results.isEmpty()) {
            QJsonObject first = results[0].toObject();
            QString dst = first["dst"].toString();
            emit translationDone(dst);
            return;
        }
    }

    emit translationError("未知响应格式");
}