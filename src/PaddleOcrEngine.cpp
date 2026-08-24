#include "PaddleOcrEngine.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QJsonArray>

PaddleOcrEngine::PaddleOcrEngine(QObject* parent)
    : QObject(parent), m_process(nullptr), m_initialized(false) {
}

PaddleOcrEngine::~PaddleOcrEngine() {
    shutdown();
}

bool PaddleOcrEngine::initialize(const QString& exePath, const QString& modelConfig) {
    m_exePath = exePath;
    m_modelConfig = modelConfig;

    if (!QFile::exists(exePath)) {
        emit initialized(false, "找不到 PaddleOCR-json.exe");
        return false;
    }

    QDir dir = QFileInfo(exePath).dir();
    if (!dir.exists("models")) {
        emit initialized(false, "找不到 models 目录，请确保 PaddleOCR-json 目录结构完整");
        return false;
    }

    m_process = new QProcess(this);
    m_process->setWorkingDirectory(dir.absolutePath());

    // ★ 用绝对路径
    QString configPath = dir.absolutePath() + "/models/" + modelConfig;

    QStringList args;
    args << "--config_path=" + configPath;

    qDebug() << "Starting PaddleOCR-json with working dir:" << dir.absolutePath();
    qDebug() << "Command:" << exePath << args.join(" ");

    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    env.insert("PATH", dir.absolutePath() + ";" + env.value("PATH"));
    m_process->setProcessEnvironment(env);

    m_process->setProcessChannelMode(QProcess::MergedChannels);

    m_process->start(exePath, args);

    connect(m_process, &QProcess::readyReadStandardOutput, this, &PaddleOcrEngine::onReadyRead);
    connect(m_process, &QProcess::readyReadStandardError, this, &PaddleOcrEngine::onReadyRead);
    connect(m_process, &QProcess::errorOccurred, this, &PaddleOcrEngine::onProcessError);
    connect(m_process, &QProcess::stateChanged, this, &PaddleOcrEngine::onProcessStateChanged);

    if (!m_process->waitForStarted(5000)) {
        emit initialized(false, "启动 PaddleOCR-json 超时");
        return false;
    }

    qDebug() << "PaddleOCR-json started, waiting for init...";
    return true;
}

void PaddleOcrEngine::onReadyRead() {
    if (!m_process) return;

    QString output = m_process->readAllStandardOutput();
    qDebug() << "OCR stdout:" << output;

    // 检查是否初始化完成
    if (!m_initialized && output.contains("OCR init completed.")) {
        m_initialized = true;
        emit initialized(true, "");
        qDebug() << "PaddleOCR-json initialized successfully!";
        return;
    }

    // 检查是否初始化失败
    if (!m_initialized && output.contains("OCR init failed")) {
        emit initialized(false, "OCR 引擎初始化失败");
        return;
    }

    // 如果是识别结果（JSON 格式），解析并发送
    if (m_initialized) {
        // 尝试解析 JSON
        QJsonParseError error;
        QJsonDocument doc = QJsonDocument::fromJson(output.toUtf8(), &error);
        if (error.error == QJsonParseError::NoError) {
            QJsonObject obj = doc.object();
            int code = obj["code"].toInt();
            if (code == 100) {
                // 识别成功，提取文本
                QJsonArray data = obj["data"].toArray();
                QStringList texts;
                for (const auto& item : data) {
                    QJsonObject block = item.toObject();
                    texts << block["text"].toString();
                }
                emit recognitionDone(texts.join(" "));
            } else if (code == 101) {
                emit recognitionError("未识别到文字");
            } else {
                emit recognitionError(QString("识别失败，错误码: %1").arg(code));
            }
        }
    }
}

void PaddleOcrEngine::recognize(const QString& imagePath) {
    if (!m_initialized || !m_process) {
        emit recognitionError("OCR 引擎未初始化");
        return;
    }

    // 构建 JSON 命令 [citation:4]
    QJsonObject command;
    // 注意：路径需要使用正斜杠，避免转义问题 [citation:4]
    QString path = QDir::toNativeSeparators(imagePath);
    command["image_path"] = path;

    sendCommand(command);
}

void PaddleOcrEngine::recognizeClipboard() {
    if (!m_initialized || !m_process) {
        emit recognitionError("OCR 引擎未初始化");
        return;
    }

    QJsonObject command;
    command["image_path"] = "clipboard";  // 识别剪贴板 [citation:4]

    sendCommand(command);
}

void PaddleOcrEngine::sendCommand(const QJsonObject& command) {
    QByteArray data = QJsonDocument(command).toJson(QJsonDocument::Compact);
    data.append('\n');  // 必须以换行结尾 [citation:4]

    qDebug() << "Sending command:" << data;
    m_process->write(data);
}

void PaddleOcrEngine::shutdown() {
    if (m_process) {
        m_process->close();
        m_process->waitForFinished(3000);
        delete m_process;
        m_process = nullptr;
    }
    m_initialized = false;
    qDebug() << "PaddleOCR-engine shutdown";
}

void PaddleOcrEngine::onProcessError(QProcess::ProcessError error) {
    qDebug() << "Process error:" << error;
    emit recognitionError(QString("进程错误: %1").arg(error));
}

void PaddleOcrEngine::onProcessStateChanged(QProcess::ProcessState state) {
    qDebug() << "Process state:" << state;
    if (state == QProcess::NotRunning && m_initialized) {
        emit recognitionError("OCR 进程意外退出");
    }
}