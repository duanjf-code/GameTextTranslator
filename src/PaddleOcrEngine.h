#ifndef PADDLEOCRENGINE_H
#define PADDLEOCRENGINE_H

#include <QObject>
#include <QProcess>
#include <QString>
#include <QJsonObject>

class PaddleOcrEngine : public QObject {
    Q_OBJECT
public:
    explicit PaddleOcrEngine(QObject* parent = nullptr);
    ~PaddleOcrEngine();

    // 初始化引擎
    bool initialize(const QString& exePath, const QString& modelConfig = "config_ch.txt");
    // 识别图片
    void recognize(const QString& imagePath);
    // 识别剪贴板
    void recognizeClipboard();
    // 关闭引擎
    void shutdown();

signals:
    void initialized(bool success, const QString& error);
    void recognitionDone(const QString& text);
    void recognitionError(const QString& error);

private slots:
    void onProcessError(QProcess::ProcessError error);
    void onProcessStateChanged(QProcess::ProcessState state);
    void onReadyRead();

private:
    QProcess* m_process;
    bool m_initialized;
    QString m_exePath;
    QString m_modelConfig;

    void sendCommand(const QJsonObject& command);
};

#endif