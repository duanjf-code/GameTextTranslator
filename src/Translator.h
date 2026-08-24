#ifndef TRANSLATOR_H
#define TRANSLATOR_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Translator : public QObject {
    Q_OBJECT
public:
    explicit Translator(QObject* parent = nullptr);

    void translate(const QString& text, const QString& from = "", const QString& to = "");

signals:
    void translationDone(const QString& result);
    void translationError(const QString& error);

private slots:
    void onReplyFinished(QNetworkReply* reply);

private:
    QNetworkAccessManager* m_networkManager;

    QString generateSign(const QString& query, const QString& salt,
                         const QString& appId, const QString& secretKey);
};

#endif