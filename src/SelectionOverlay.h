#ifndef SELECTIONOVERLAY_H
#define SELECTIONOVERLAY_H

#include <QWidget>
#include <QRect>

class SelectionOverlay : public QWidget {
    Q_OBJECT
public:
    enum class Mode {
        Normal,      // 普通截图模式
        LockRegion   // 锁定区域模式
    };

    explicit SelectionOverlay(QWidget* parent = nullptr);

    void startSelection();
    void setMode(Mode mode);

signals:
    void regionSelected(const QRect& rect);
    void cancelled();  // ★ 取消信号

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    bool m_selecting;
    QPoint m_startPos;
    QPoint m_endPos;
    QRect m_selectedRect;
    Mode m_mode;
};

#endif