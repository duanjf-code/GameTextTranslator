#include "ResultOverlay.h"
#include <QPainter>
#include <QMouseEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QFontMetrics>

ResultOverlay::ResultOverlay(QWidget* parent)
    : QWidget(parent), m_show(false), m_padding(20), m_maxWidth(600) {
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    // 初始位置在右下角，稍后根据内容调整
    setVisible(false);

    connect(&m_autoHideTimer, &QTimer::timeout, this, &ResultOverlay::hideResult);
}

void ResultOverlay::showResult(const QString& text, int autoHideDelayMs) {
    m_resultText = text;
    m_show = true;

    // ★ 计算文本所需大小
    QFont font("Microsoft YaHei", 12);
    QFontMetrics fm(font);

    // 计算文本矩形（限制最大宽度，自动换行）
    int maxWidth = m_maxWidth - m_padding * 2;
    QRect textRect = fm.boundingRect(QRect(0, 0, maxWidth, 2000),
                                     Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop,
                                     text);

    // 计算窗口大小（文本 + 边距）
    int windowWidth = qMin(textRect.width() + m_padding * 2, m_maxWidth);
    int windowHeight = textRect.height() + m_padding * 2;

    // ★ 调整窗口位置（右下角）
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenRect = screen->geometry();
        int x = screenRect.width() - windowWidth - 20;
        int y = screenRect.height() - windowHeight - 20;
        setGeometry(x, y, windowWidth, windowHeight);
    }

    setVisible(true);
    raise();
    update();

    m_autoHideTimer.stop();
    if (autoHideDelayMs > 0) {
        m_autoHideTimer.start(autoHideDelayMs);
    }
}

void ResultOverlay::hideResult() {
    m_show = false;
    setVisible(false);
    m_autoHideTimer.stop();
}

void ResultOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    if (!m_show) return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    // 半透明毛玻璃背景
    QColor bgColor(0, 0, 0, 200);
    painter.setBrush(bgColor);
    painter.setPen(QPen(Qt::white, 1));
    painter.drawRoundedRect(rect(), 10, 10);

    // 文字
    painter.setPen(Qt::white);
    QFont font("Microsoft YaHei", 12);
    painter.setFont(font);

    QRect textRect = rect().adjusted(m_padding, m_padding, -m_padding, -m_padding);
    painter.drawText(textRect, Qt::TextWordWrap | Qt::AlignLeft | Qt::AlignTop, m_resultText);
}

void ResultOverlay::mousePressEvent(QMouseEvent* event) {
    Q_UNUSED(event);
    hideResult();
}

void ResultOverlay::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        hideResult();
    }
}