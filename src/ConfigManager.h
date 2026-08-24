#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include <QObject>
#include <QString>
#include <QKeySequence>

class ConfigManager : public QObject {
    Q_OBJECT
public:
    static ConfigManager& instance();

    // 热键
    QKeySequence getScreenshotHotkey() const;
    void setScreenshotHotkey(const QKeySequence& hotkey);

    // 百度翻译
    QString getBaiduAppId() const;
    QString getBaiduSecretKey() const;
    void setBaiduAppId(const QString& appId);
    void setBaiduSecretKey(const QString& secretKey);

    // 开机自启
    bool getAutoStart() const;
    void setAutoStart(bool enabled);

    // ★ OCR 语言
    QString getOcrLanguage() const;
    void setOcrLanguage(const QString& lang);

    // ★ 目标语言
    QString getTargetLanguage() const;
    void setTargetLanguage(const QString& lang);

    bool load();
    bool save();

signals:
    void configChanged();

private:
    ConfigManager() : QObject(nullptr) {}
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    QString m_configPath;
    QKeySequence m_hotkey;
    QString m_baiduAppId;
    QString m_baiduSecretKey;
    bool m_autoStart;
    QString m_ocrLanguage;
    QString m_targetLanguage;
};

#endif