#ifndef RESULTOVERLAY_H
#define RESULTOVERLAY_H

#include <QWidget>
#include <QTimer>
#include <QRect>

class ResultOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ResultOverlay(QWidget* parent = nullptr);

    void showResult(const QString& text, int autoHideDelayMs = 5000);
    void showResultAt(const QString& text, const QRect& targetRect, int autoHideDelayMs = 5000);
    void hideResult();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    void updatePosition(const QRect& targetRect);

    QString m_resultText;
    QTimer m_autoHideTimer;
    bool m_show;
    int m_padding;
    int m_maxWidth;
    QRect m_targetRect;      // ★ 目标区域
    bool m_useTargetPos;     // ★ 是否使用目标位置
};

#endif