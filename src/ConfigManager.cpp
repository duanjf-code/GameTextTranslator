#include "ConfigManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <QSettings>
#include <QCoreApplication>

ConfigManager& ConfigManager::instance() {
    static ConfigManager instance;
    return instance;
}

bool ConfigManager::load() {
    QString appDir = QCoreApplication::applicationDirPath();
    m_configPath = appDir + "/config.json";
    qDebug() << "ConfigManager: config path =" << m_configPath;

    QFile file(m_configPath);

    if (!file.exists()) {
        qDebug() << "Config file not found, using defaults";
        m_hotkey = QKeySequence("Ctrl+Alt+T");
        m_baiduAppId = "";
        m_baiduSecretKey = "";
        m_autoStart = false;
        m_ocrLanguage = "config_chinese.txt";
        m_targetLanguage = "zh";
        save();
        return true;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Failed to open config file, using defaults";
        m_hotkey = QKeySequence("Ctrl+Alt+T");
        m_baiduAppId = "";
        m_baiduSecretKey = "";
        m_autoStart = false;
        m_ocrLanguage = "config_chinese.txt";
        m_targetLanguage = "zh";
        return true;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        qDebug() << "Invalid JSON, using defaults";
        m_hotkey = QKeySequence("Ctrl+Alt+T");
        m_baiduAppId = "";
        m_baiduSecretKey = "";
        m_autoStart = false;
        m_ocrLanguage = "config_chinese.txt";
        m_targetLanguage = "zh";
        return true;
    }

    QJsonObject obj = doc.object();
    m_hotkey = QKeySequence(obj["hotkey"].toString("Ctrl+Alt+T"));
    m_baiduAppId = obj["baidu_app_id"].toString("");
    m_baiduSecretKey = obj["baidu_secret_key"].toString("");
    m_autoStart = obj["auto_start"].toBool(false);
    m_ocrLanguage = obj["ocr_language"].toString("config_chinese.txt");
    m_targetLanguage = obj["target_language"].toString("zh");

    qDebug() << "Config loaded: hotkey =" << m_hotkey.toString()
             << ", autoStart =" << m_autoStart
             << ", ocrLang =" << m_ocrLanguage
             << ", targetLang =" << m_targetLanguage;
    return true;
}

bool ConfigManager::save() {
    QJsonObject obj;
    obj["hotkey"] = m_hotkey.toString();
    obj["baidu_app_id"] = m_baiduAppId;
    obj["baidu_secret_key"] = m_baiduSecretKey;
    obj["auto_start"] = m_autoStart;
    obj["ocr_language"] = m_ocrLanguage;
    obj["target_language"] = m_targetLanguage;

    QJsonDocument doc(obj);

    QFile file(m_configPath);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Failed to save config";
        return false;
    }

    file.write(doc.toJson());
    file.close();
    qDebug() << "Config saved to:" << m_configPath;
    return true;
}

// ===== Getters =====
QKeySequence ConfigManager::getScreenshotHotkey() const { return m_hotkey; }
QString ConfigManager::getBaiduAppId() const { return m_baiduAppId; }
QString ConfigManager::getBaiduSecretKey() const { return m_baiduSecretKey; }
bool ConfigManager::getAutoStart() const { return m_autoStart; }
QString ConfigManager::getOcrLanguage() const { return m_ocrLanguage; }
QString ConfigManager::getTargetLanguage() const { return m_targetLanguage; }

// ===== Setters =====
void ConfigManager::setScreenshotHotkey(const QKeySequence& hotkey) {
    m_hotkey = hotkey;
    save();
    emit configChanged();
}

void ConfigManager::setBaiduAppId(const QString& appId) {
    m_baiduAppId = appId;
    save();
}

void ConfigManager::setBaiduSecretKey(const QString& secretKey) {
    m_baiduSecretKey = secretKey;
    save();
}

void ConfigManager::setAutoStart(bool enabled) {
    m_autoStart = enabled;
    save();
    emit configChanged();
}

void ConfigManager::setOcrLanguage(const QString& lang) {
    m_ocrLanguage = lang;
    save();
}

void ConfigManager::setTargetLanguage(const QString& lang) {
    m_targetLanguage = lang;
    save();
}