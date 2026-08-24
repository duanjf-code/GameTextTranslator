#ifndef SELECTIONOVERLAY_H
#define SELECTIONOVERLAY_H

#include <QWidget>
#include <QRect>

class SelectionOverlay : public QWidget {
    Q_OBJECT
public:
    explicit SelectionOverlay(QWidget* parent = nullptr);

    void startSelection();

signals:
    void regionSelected(const QRect& rect);

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    bool m_selecting;
    QPoint m_startPos;
    QPoint m_endPos;
    QRect m_selectedRect;
};

#endif