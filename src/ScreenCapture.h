#ifndef SCREENCAPTURE_H
#define SCREENCAPTURE_H

#include <QImage>

class ScreenCapture {
public:
    ScreenCapture();
    ~ScreenCapture();

    QImage captureScreen();
    bool isInitialized() const;

private:
    bool m_initialized;
};

#endif