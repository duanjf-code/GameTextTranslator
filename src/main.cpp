#include <QApplication>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QStyle>
#include <QDebug>
#include <QScreen>
#include <QGuiApplication>
#include <QDir>
#include <QTimer>
#include "GlobalHotkey.h"
#include "SelectionOverlay.h"
#include "ResultOverlay.h"
#include "ScreenCapture.h"
#include "Translator.h"
#include "ConfigManager.h"
#include "PaddleOcrEngine.h"
#include "SettingsDialog.h"

class MainApp : public QObject {
    Q_OBJECT
public:
    MainApp(QApplication* app) : m_app(app), m_screenshotEnabled(true), m_autoMonitorActive(false) {
        qDebug() << "App starting...";

        ConfigManager::instance().load();

        setupTray();
        setupHotkeys();

        m_resultOverlay = new ResultOverlay();
        m_translator = new Translator(this);

        setupOcr();

        connect(m_translator, &Translator::translationDone,
                this, &MainApp::onTranslationDone);
        connect(m_translator, &Translator::translationError,
                this, &MainApp::onTranslationError);

        qDebug() << "App initialized successfully!";
    }

    void setScreenshotEnabled(bool enabled) {
        m_screenshotEnabled = enabled;
        qDebug() << "Screenshot enabled:" << enabled;
    }

private slots:
    void showSettingsDialog() {
        setScreenshotEnabled(false);

        SettingsDialog dialog;
        dialog.setConfigPath(QCoreApplication::applicationDirPath() + "/config.json");
        dialog.setWindowModality(Qt::ApplicationModal);

        connect(&dialog, &QDialog::finished, this, [this](int) {
            ConfigManager::instance().load();
            setupHotkeys();
            setScreenshotEnabled(true);
            qDebug() << "Settings dialog closed, screenshot re-enabled";
        });

        dialog.exec();
    }

    void startAutoMonitor() {
        qDebug() << "Starting auto monitor mode...";

        if (m_selectionOverlay) {
            m_selectionOverlay->deleteLater();
            m_selectionOverlay = nullptr;
        }

        m_selectionOverlay = new SelectionOverlay();
        m_selectionOverlay->setMode(SelectionOverlay::Mode::LockRegion);
        connect(m_selectionOverlay, &SelectionOverlay::regionSelected,
                this, &MainApp::onLockRegionSelected);
        m_selectionOverlay->startSelection();
    }

    void stopAutoMonitor() {
        qDebug() << "Stopping auto monitor mode...";
        m_autoMonitorActive = false;

        if (m_monitorTimer) {
            m_monitorTimer->stop();
        }

        m_lockedRegion = QRect();
        m_lastOcrResult.clear();

        if (m_autoMonitorAction) {
            m_autoMonitorAction->setChecked(false);
        }

        m_resultOverlay->showResult("已停止自动监听");
    }

    void onLockRegionSelected(const QRect& rect) {
        qDebug() << "Region locked:" << rect;

        m_lockedRegion = rect;
        m_autoMonitorActive = true;
        m_lastOcrResult.clear();

        if (!m_monitorTimer) {
            m_monitorTimer = new QTimer(this);
            connect(m_monitorTimer, &QTimer::timeout, this, &MainApp::onMonitorTick);
        }

        int interval = ConfigManager::instance().getMonitorInterval();
        if (interval <= 0) interval = 1000;
        m_monitorTimer->start(interval);

        m_resultOverlay->showResultAt(QString("已锁定区域，开始自动监听\n%1×%2\n间隔 %3ms")
                                          .arg(rect.width()).arg(rect.height()).arg(interval), rect);

        if (m_selectionOverlay) {
            m_selectionOverlay->deleteLater();
            m_selectionOverlay = nullptr;
        }
    }

    void onMonitorTick() {
        if (!m_autoMonitorActive || m_lockedRegion.isNull()) {
            return;
        }

        ScreenCapture capture;
        QImage fullScreen = capture.captureScreen();
        if (fullScreen.isNull()) return;

        QScreen* screen = QGuiApplication::primaryScreen();
        qreal dpr = screen ? screen->devicePixelRatio() : 1.0;

        QRect physicalRect(
            m_lockedRegion.x() * dpr,
            m_lockedRegion.y() * dpr,
            m_lockedRegion.width() * dpr,
            m_lockedRegion.height() * dpr
            );

        QImage currentFrame = fullScreen.copy(physicalRect);
        if (currentFrame.isNull()) return;

        QString tempPath = QDir::temp().absoluteFilePath("auto_ocr_temp.png");
        if (!currentFrame.save(tempPath, "PNG")) return;

        connect(m_ocrEngine, &PaddleOcrEngine::recognitionDone, this, [this](const QString& text) {
            if (text.isEmpty()) {
                qDebug() << "Auto monitor: OCR returned empty";
                return;
            }

            qDebug() << "Auto monitor: OCR result:" << text;

            if (m_lastOcrResult.isEmpty()) {
                m_lastOcrResult = text;
                m_resultOverlay->showResultAt("正在翻译...", m_lockedRegion);
                m_translator->translate(text);
                return;
            }

            if (text != m_lastOcrResult) {
                m_lastOcrResult = text;
                m_resultOverlay->showResultAt("正在翻译...", m_lockedRegion);
                m_translator->translate(text);
                qDebug() << "Auto monitor: text changed, translating";
            } else {
                qDebug() << "Auto monitor: text unchanged, skipping";
            }
        }, Qt::SingleShotConnection);

        m_ocrEngine->recognize(tempPath);
    }

private:
    QApplication* m_app;
    QSystemTrayIcon* m_tray = nullptr;
    GlobalHotkey* m_hotkey = nullptr;
    SelectionOverlay* m_selectionOverlay = nullptr;
    ResultOverlay* m_resultOverlay = nullptr;
    Translator* m_translator = nullptr;
    PaddleOcrEngine* m_ocrEngine = nullptr;
    bool m_screenshotEnabled;
    bool m_autoMonitorActive;
    QRect m_lockedRegion;
    QRect m_currentRect;  // 保存当前截图区域
    QTimer* m_monitorTimer = nullptr;
    QString m_lastOcrResult;
    QAction* m_autoMonitorAction = nullptr;

    void setupTray() {
        m_tray = new QSystemTrayIcon(this);
        m_tray->setIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));
        m_tray->setToolTip("游戏文本翻译器");

        QMenu* menu = new QMenu();

        QAction* autoMonitorAction = new QAction("自动监听模式", this);
        autoMonitorAction->setCheckable(true);
        autoMonitorAction->setChecked(false);
        connect(autoMonitorAction, &QAction::triggered, this, [this, autoMonitorAction](bool checked) {
            if (checked) {
                startAutoMonitor();
            } else {
                stopAutoMonitor();
            }
            autoMonitorAction->setChecked(checked);
        });
        menu->addAction(autoMonitorAction);
        m_autoMonitorAction = autoMonitorAction;

        menu->addSeparator();

        QAction* settingsAction = new QAction("配置", this);
        connect(settingsAction, &QAction::triggered, this, &MainApp::showSettingsDialog);
        menu->addAction(settingsAction);

        menu->addSeparator();

        QAction* quitAction = new QAction("退出", this);
        connect(quitAction, &QAction::triggered, this, &MainApp::onQuit);
        menu->addAction(quitAction);

        m_tray->setContextMenu(menu);
        m_tray->show();
    }

    void setupHotkeys() {
        if (m_hotkey) {
            delete m_hotkey;
            m_hotkey = nullptr;
        }

        m_hotkey = new GlobalHotkey(this);
        QKeySequence hotkey = ConfigManager::instance().getScreenshotHotkey();
        m_hotkey->registerHotkey(1, hotkey);

        connect(m_hotkey, &GlobalHotkey::hotkeyPressed, this, [this](int id) {
            if (id == 1) {
                if (!m_screenshotEnabled) {
                    qDebug() << "Screenshot disabled, ignoring hotkey";
                    return;
                }

                if (m_selectionOverlay) {
                    m_selectionOverlay->deleteLater();
                    m_selectionOverlay = nullptr;
                }
                m_selectionOverlay = new SelectionOverlay();
                m_selectionOverlay->setMode(SelectionOverlay::Mode::Normal);
                connect(m_selectionOverlay, &SelectionOverlay::regionSelected,
                        this, &MainApp::onRegionSelected);
                m_selectionOverlay->startSelection();
            }
        });
        qDebug() << "Hotkey registered:" << hotkey.toString();
    }

    void setupOcr() {
        m_ocrEngine = new PaddleOcrEngine(this);

        connect(m_ocrEngine, &PaddleOcrEngine::initialized,
                this, &MainApp::onOcrInitialized);
        connect(m_ocrEngine, &PaddleOcrEngine::recognitionDone,
                this, &MainApp::onOcrDone);
        connect(m_ocrEngine, &PaddleOcrEngine::recognitionError,
                this, &MainApp::onOcrError);

        QString appDir = QCoreApplication::applicationDirPath();
        QString exePath = appDir + "/PaddleOCR-json/PaddleOCR-json.exe";

        qDebug() << "OCR exe path:" << exePath;

        if (!QFile::exists(exePath)) {
            qDebug() << "PaddleOCR-json.exe not found!";
            m_resultOverlay->showResult("OCR 引擎缺失，请确保 PaddleOCR-json 目录存在");
            return;
        }

        QString ocrLang = ConfigManager::instance().getOcrLanguage();
        if (ocrLang.isEmpty()) {
            ocrLang = "config_chinese.txt";
        }

        bool result = m_ocrEngine->initialize(exePath, ocrLang);
        if (!result) {
            qDebug() << "OCR engine initialization started...";
        }
    }

    void onOcrInitialized(bool success, const QString& error) {
        if (success) {
            qDebug() << "✅ OCR 引擎初始化成功！";
            m_resultOverlay->showResult("OCR 引擎已就绪");
        } else {
            qDebug() << "❌ OCR 引擎初始化失败:" << error;
            m_resultOverlay->showResult("OCR 初始化失败: " + error);
        }
    }

    void onOcrDone(const QString& text) {
        qDebug() << "onOcrDone called, text:" << text << ", autoMonitorActive:" << m_autoMonitorActive;

        if (text.isEmpty()) {
            if (!m_autoMonitorActive) {
                m_resultOverlay->showResultAt("未识别到文字，请重试", m_currentRect);
            }
            return;
        }

        if (!m_autoMonitorActive) {
            m_resultOverlay->showResultAt("正在翻译...", m_currentRect);
            m_translator->translate(text);
        }
    }

    void onOcrError(const QString& error) {
        qDebug() << "OCR 错误:" << error;
        m_resultOverlay->showResult("OCR 错误: " + error);
    }

    void onRegionSelected(const QRect& rect) {
        qDebug() << "Region selected:" << rect;
        m_currentRect = rect;

        QScreen* screen = QGuiApplication::primaryScreen();
        qreal dpr = screen ? screen->devicePixelRatio() : 1.0;

        QRect physicalRect(
            rect.x() * dpr,
            rect.y() * dpr,
            rect.width() * dpr,
            rect.height() * dpr
            );

        ScreenCapture capture;
        QImage fullScreen = capture.captureScreen();
        if (fullScreen.isNull()) {
            m_resultOverlay->showResultAt("截图失败", rect);
            return;
        }

        QImage cropped = fullScreen.copy(physicalRect);
        if (cropped.isNull()) {
            m_resultOverlay->showResultAt("裁剪失败", rect);
            return;
        }

        QString tempPath = QDir::temp().absoluteFilePath("ocr_temp.png");
        if (!cropped.save(tempPath, "PNG")) {
            m_resultOverlay->showResultAt("保存临时图片失败", rect);
            return;
        }

        qDebug() << "Screenshot saved to:" << tempPath;

        m_resultOverlay->showResultAt("正在识别文字...", rect);
        m_ocrEngine->recognize(tempPath);

        if (m_selectionOverlay) {
            m_selectionOverlay->deleteLater();
            m_selectionOverlay = nullptr;
        }
    }

    void onTranslationDone(const QString& result) {
        if (m_autoMonitorActive) {
            m_resultOverlay->showResultAt(result, m_lockedRegion, 0);
        } else {
            m_resultOverlay->showResultAt(result, m_currentRect, 5000);
        }
    }

    void onTranslationError(const QString& error) {
        if (m_autoMonitorActive) {
            m_resultOverlay->showResultAt("翻译失败: " + error, m_lockedRegion);
        } else {
            m_resultOverlay->showResultAt("翻译失败: " + error, m_currentRect);
        }
    }

    void onQuit() {
        qDebug() << "Shutting down...";

        if (m_monitorTimer) {
            m_monitorTimer->stop();
        }

        if (m_ocrEngine) {
            m_ocrEngine->shutdown();
        }

        qApp->quit();
    }
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("GameTextTranslator");
    app.setOrganizationName("GameTextTranslator");

    qDebug() << "Qt Application started";
    MainApp mainApp(&app);
    return app.exec();
}

#include "main.moc"