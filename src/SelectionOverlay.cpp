#include "SelectionOverlay.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QApplication>
#include <QDebug>

SelectionOverlay::SelectionOverlay(QWidget* parent)
    : QWidget(parent), m_selecting(false), m_mode(Mode::Normal) {
    setWindowFlags(Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setCursor(Qt::CrossCursor);
    setMouseTracking(true);
}

void SelectionOverlay::setMode(Mode mode) {
    m_mode = mode;
}

void SelectionOverlay::startSelection() {
    qDebug() << "=== SelectionOverlay::startSelection called ===";

    m_selecting = false;
    m_selectedRect = QRect();
    m_startPos = QPoint();
    m_endPos = QPoint();

    QScreen* screen = QGuiApplication::primaryScreen();
    if (!screen) return;

    QRect screenRect = screen->geometry();
    setGeometry(screenRect);

    showFullScreen();
    raise();
    activateWindow();
    update();
    qDebug() << "=== SelectionOverlay shown ===";
}

void SelectionOverlay::paintEvent(QPaintEvent* event) {
    Q_UNUSED(event);
    QPainter painter(this);

    // 半透明遮罩
    painter.fillRect(rect(), QColor(0, 0, 0, 150));

    if (!m_selectedRect.isNull() && m_selectedRect.width() > 0 && m_selectedRect.height() > 0) {
        // 镂空选区
        painter.setCompositionMode(QPainter::CompositionMode_Clear);
        painter.fillRect(m_selectedRect, Qt::transparent);

        // 边框
        painter.setCompositionMode(QPainter::CompositionMode_SourceOver);
        painter.setPen(QPen(Qt::red, 2));
        painter.drawRect(m_selectedRect);

        // 显示尺寸
        QString sizeText = QString("%1 × %2").arg(m_selectedRect.width()).arg(m_selectedRect.height());
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 11));
        painter.drawText(m_selectedRect.bottomRight() + QPoint(8, 20), sizeText);

        // ★ 根据模式显示不同提示
        if (m_mode == Mode::LockRegion) {
            painter.drawText(m_selectedRect.bottomRight() + QPoint(8, 40),
                             "松开即锁定此区域");
        } else {
            painter.drawText(m_selectedRect.bottomRight() + QPoint(8, 40),
                             "松开即翻译");
        }
    } else {
        // 提示
        painter.setPen(Qt::white);
        painter.setFont(QFont("Microsoft YaHei", 14));
        if (m_mode == Mode::LockRegion) {
            painter.drawText(rect(), Qt::AlignCenter,
                             "拖拽选择要锁定的区域\n此区域将被持续监控");
        } else {
            painter.drawText(rect(), Qt::AlignCenter,
                             "按住左键拖拽选择区域\n松开即自动翻译");
        }
    }
}

void SelectionOverlay::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_selecting = true;
        m_startPos = event->pos();
        m_endPos = m_startPos;
        m_selectedRect = QRect();
        update();
    } else if (event->button() == Qt::RightButton) {
        hide();
    }
}

void SelectionOverlay::mouseMoveEvent(QMouseEvent* event) {
    if (m_selecting) {
        m_endPos = event->pos();
        m_selectedRect = QRect(m_startPos, m_endPos).normalized();
        update();
    }
}

void SelectionOverlay::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && m_selecting) {
        m_selecting = false;

        if (!m_selectedRect.isNull() &&
            m_selectedRect.width() > 10 &&
            m_selectedRect.height() > 10) {
            qDebug() << "=== Region confirmed! Rect:" << m_selectedRect;
            QRect rect = m_selectedRect;
            m_selectedRect = QRect();
            hide();
            emit regionSelected(rect);
        } else {
            qDebug() << "=== Region too small, cancelling";
            m_selectedRect = QRect();
            hide();
        }
    }
}

void SelectionOverlay::keyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "ESC pressed, cancelling";
        m_selecting = false;
        m_selectedRect = QRect();
        hide();
    }
}