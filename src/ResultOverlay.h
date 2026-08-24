#ifndef RESULTOVERLAY_H
#define RESULTOVERLAY_H

#include <QWidget>
#include <QTimer>

class ResultOverlay : public QWidget {
    Q_OBJECT
public:
    explicit ResultOverlay(QWidget* parent = nullptr);

    void showResult(const QString& text, int autoHideDelayMs = 5000);
    void hideResult();

protected:
    void paintEvent(QPaintEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QString m_resultText;
    QTimer m_autoHideTimer;
    bool m_show;
    int m_padding;
    int m_maxWidth;
};

#endif