#include "SettingsDialog.h"
#include "ConfigManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QHBoxLayout>
#include <QSettings>
#include <QCoreApplication>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent), m_currentConfigPath("") {
    setWindowTitle("配置");
    setFixedSize(500, 480);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QFormLayout* formLayout = new QFormLayout();

    // ===== 配置路径显示 =====
    QHBoxLayout* pathLayout = new QHBoxLayout();
    m_configPathLabel = new QLabel("未加载配置文件", this);
    m_configPathLabel->setStyleSheet("color: gray; font-size: 9pt;");
    QPushButton* browseBtn = new QPushButton("浏览", this);
    browseBtn->setFixedWidth(60);
    connect(browseBtn, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(this, "选择配置文件", "", "JSON Files (*.json)");
        if (!path.isEmpty()) {
            loadFromFile(path);
        }
    });
    pathLayout->addWidget(m_configPathLabel);
    pathLayout->addWidget(browseBtn);
    formLayout->addRow("配置文件:", pathLayout);

    // ===== 百度翻译 APP ID =====
    m_appIdEdit = new QLineEdit(this);
    formLayout->addRow("百度 APP ID:", m_appIdEdit);

    // ===== 百度翻译密钥 =====
    m_secretKeyEdit = new QLineEdit(this);
    m_secretKeyEdit->setEchoMode(QLineEdit::Password);
    formLayout->addRow("百度密钥:", m_secretKeyEdit);

    // ===== 截图热键 =====
    m_hotkeyEdit = new QKeySequenceEdit(this);
    formLayout->addRow("截图热键:", m_hotkeyEdit);

    // ===== OCR 语言 =====
    m_ocrLangCombo = new QComboBox(this);
    m_ocrLangCombo->addItem("config_chinese.txt", "中文");
    m_ocrLangCombo->addItem("config_en.txt", "英文");
    m_ocrLangCombo->addItem("config_japan.txt", "日文");
    m_ocrLangCombo->addItem("config_korean.txt", "韩文");
    formLayout->addRow("OCR 语言:", m_ocrLangCombo);

    // ===== 目标语言 =====
    m_targetLangCombo = new QComboBox(this);
    m_targetLangCombo->addItem("zh", "中文");
    m_targetLangCombo->addItem("en", "英文");
    m_targetLangCombo->addItem("ja", "日文");
    m_targetLangCombo->addItem("ko", "韩文");
    formLayout->addRow("翻译目标语言:", m_targetLangCombo);

    // ===== 监听间隔 =====
    m_intervalSpinBox = new QSpinBox(this);
    m_intervalSpinBox->setRange(1, 10);
    m_intervalSpinBox->setSuffix(" 秒");
    m_intervalSpinBox->setToolTip("自动监听模式下，每隔几秒截图识别一次");
    formLayout->addRow("监听间隔:", m_intervalSpinBox);

    // ===== 开机自启 =====
    m_autoStartCheck = new QCheckBox("开机自动启动", this);
    formLayout->addRow("", m_autoStartCheck);

    mainLayout->addLayout(formLayout);

    // ===== 按钮 =====
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_loadBtn = new QPushButton("加载配置", this);
    m_saveBtn = new QPushButton("保存", this);
    m_saveAsBtn = new QPushButton("另存为...", this);
    m_cancelBtn = new QPushButton("关闭", this);

    btnLayout->addWidget(m_loadBtn);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_saveAsBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_loadBtn, &QPushButton::clicked, this, &SettingsDialog::onLoad);
    connect(m_saveBtn, &QPushButton::clicked, this, &SettingsDialog::onSave);
    connect(m_saveAsBtn, &QPushButton::clicked, this, &SettingsDialog::onSaveAs);
    connect(m_cancelBtn, &QPushButton::clicked, this, &SettingsDialog::onCancel);

    loadSettings();
}

void SettingsDialog::setConfigPath(const QString& path) {
    m_currentConfigPath = path;
    if (!path.isEmpty()) {
        m_configPathLabel->setText(path);
        m_configPathLabel->setStyleSheet("color: black; font-size: 9pt;");
    }
}

QString SettingsDialog::getDefaultConfigPath() {
    return QCoreApplication::applicationDirPath() + "/config.json";
}

void SettingsDialog::loadFromFile(const QString& path) {
    QFile file(path);
    if (!file.exists()) {
        QMessageBox::warning(this, "错误", "配置文件不存在");
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        QMessageBox::warning(this, "错误", "无法读取配置文件");
        return;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (doc.isNull()) {
        QMessageBox::warning(this, "错误", "配置文件格式错误");
        return;
    }

    QJsonObject obj = doc.object();

    // 填入界面
    m_appIdEdit->setText(obj["baidu_app_id"].toString(""));
    m_secretKeyEdit->setText(obj["baidu_secret_key"].toString(""));
    m_hotkeyEdit->setKeySequence(QKeySequence(obj["hotkey"].toString("Ctrl+Alt+T")));
    m_autoStartCheck->setChecked(obj["auto_start"].toBool(false));

    QString ocrLang = obj["ocr_language"].toString("config_chinese.txt");
    int ocrIndex = m_ocrLangCombo->findText(ocrLang);
    if (ocrIndex >= 0) m_ocrLangCombo->setCurrentIndex(ocrIndex);

    QString targetLang = obj["target_language"].toString("zh");
    int targetIndex = m_targetLangCombo->findData(targetLang);
    if (targetIndex >= 0) m_targetLangCombo->setCurrentIndex(targetIndex);

    int interval = obj["monitor_interval"].toInt(3000);
    m_intervalSpinBox->setValue(interval / 1000);

    m_currentConfigPath = path;
    m_configPathLabel->setText(path);
    m_configPathLabel->setStyleSheet("color: black; font-size: 9pt;");

    // 加载后立即应用到程序
    applySettings();

    QMessageBox::information(this, "成功", "配置已加载并应用");
}

void SettingsDialog::saveToFile(const QString& path) {
    QJsonObject obj;
    obj["baidu_app_id"] = m_appIdEdit->text().trimmed();
    obj["baidu_secret_key"] = m_secretKeyEdit->text().trimmed();
    obj["hotkey"] = m_hotkeyEdit->keySequence().toString();
    obj["auto_start"] = m_autoStartCheck->isChecked();
    obj["ocr_language"] = m_ocrLangCombo->currentText();
    obj["target_language"] = m_targetLangCombo->currentData().toString();
    obj["monitor_interval"] = m_intervalSpinBox->value() * 1000;

    QJsonDocument doc(obj);

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "错误", "无法保存配置文件: " + file.errorString());
        return;
    }

    file.write(doc.toJson());
    file.close();

    m_currentConfigPath = path;
    m_configPathLabel->setText(path);
    m_configPathLabel->setStyleSheet("color: black; font-size: 9pt;");

    QMessageBox::information(this, "成功", "配置已保存到:\n" + path);
}

void SettingsDialog::applySettings() {
    auto& config = ConfigManager::instance();

    config.setBaiduAppId(m_appIdEdit->text().trimmed());
    config.setBaiduSecretKey(m_secretKeyEdit->text().trimmed());

    QKeySequence hotkey = m_hotkeyEdit->keySequence();
    if (!hotkey.isEmpty()) {
        config.setScreenshotHotkey(hotkey);
    }

    config.setAutoStart(m_autoStartCheck->isChecked());
    config.setOcrLanguage(m_ocrLangCombo->currentText());
    config.setTargetLanguage(m_targetLangCombo->currentData().toString());
    config.setMonitorInterval(m_intervalSpinBox->value() * 1000);

    // 应用开机自启（注册表）
    if (m_autoStartCheck->isChecked()) {
        QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            QSettings::NativeFormat);
        QString appPath = QCoreApplication::applicationFilePath();
        appPath.replace("/", "\\");
        settings.setValue("GameTextTranslator", appPath);
    } else {
        QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            QSettings::NativeFormat);
        settings.remove("GameTextTranslator");
    }

    emit config.configChanged();

    qDebug() << "配置已应用到程序";
    qDebug() << "  monitor_interval:" << config.getMonitorInterval();
}

void SettingsDialog::showEvent(QShowEvent* event) {
    QDialog::showEvent(event);
    loadSettings();
}

void SettingsDialog::loadSettings() {
    auto& config = ConfigManager::instance();
    m_appIdEdit->setText(config.getBaiduAppId());
    m_secretKeyEdit->setText(config.getBaiduSecretKey());
    m_hotkeyEdit->setKeySequence(config.getScreenshotHotkey());
    m_autoStartCheck->setChecked(config.getAutoStart());

    QString ocrLang = config.getOcrLanguage();
    int ocrIndex = m_ocrLangCombo->findText(ocrLang);
    if (ocrIndex >= 0) m_ocrLangCombo->setCurrentIndex(ocrIndex);

    QString targetLang = config.getTargetLanguage();
    int targetIndex = m_targetLangCombo->findData(targetLang);
    if (targetIndex >= 0) m_targetLangCombo->setCurrentIndex(targetIndex);

    int interval = config.getMonitorInterval();
    if (interval <= 0) interval = 3000;
    m_intervalSpinBox->setValue(interval / 1000);

    // 显示当前配置路径
    QString defaultPath = getDefaultConfigPath();
    if (QFile::exists(defaultPath)) {
        m_currentConfigPath = defaultPath;
        m_configPathLabel->setText(defaultPath);
        m_configPathLabel->setStyleSheet("color: black; font-size: 9pt;");
    }
}

void SettingsDialog::saveSettings() {
    auto& config = ConfigManager::instance();
    config.setBaiduAppId(m_appIdEdit->text().trimmed());
    config.setBaiduSecretKey(m_secretKeyEdit->text().trimmed());

    QKeySequence hotkey = m_hotkeyEdit->keySequence();
    if (!hotkey.isEmpty()) {
        config.setScreenshotHotkey(hotkey);
    }

    config.setAutoStart(m_autoStartCheck->isChecked());
    config.setOcrLanguage(m_ocrLangCombo->currentText());
    config.setTargetLanguage(m_targetLangCombo->currentData().toString());
    config.setMonitorInterval(m_intervalSpinBox->value() * 1000);

    // 应用开机自启
    if (m_autoStartCheck->isChecked()) {
        QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            QSettings::NativeFormat);
        QString appPath = QCoreApplication::applicationFilePath();
        appPath.replace("/", "\\");
        settings.setValue("GameTextTranslator", appPath);
    } else {
        QSettings settings(
            "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
            QSettings::NativeFormat);
        settings.remove("GameTextTranslator");
    }

    QMessageBox::information(this, "成功", "配置已保存到内存");
}

void SettingsDialog::onSave() {
    if (!m_currentConfigPath.isEmpty() && QFile::exists(m_currentConfigPath)) {
        saveToFile(m_currentConfigPath);
    } else {
        onSaveAs();
    }
}

void SettingsDialog::onSaveAs() {
    QString path = QFileDialog::getSaveFileName(this, "保存配置文件",
                                                getDefaultConfigPath(),
                                                "JSON Files (*.json)");
    if (!path.isEmpty()) {
        saveToFile(path);
    }
}

void SettingsDialog::onLoad() {
    QString path = QFileDialog::getOpenFileName(this, "加载配置文件",
                                                getDefaultConfigPath(),
                                                "JSON Files (*.json)");
    if (!path.isEmpty()) {
        loadFromFile(path);
    }
}

void SettingsDialog::onCancel() {
    reject();
}