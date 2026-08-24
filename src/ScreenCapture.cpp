#include "ScreenCapture.h"
#include <windows.h>
#include <QDebug>
#include <QGuiApplication>
#include <QScreen>

ScreenCapture::ScreenCapture() : m_initialized(true) {
    qDebug() << "ScreenCapture initialized (GDI mode)";
}

ScreenCapture::~ScreenCapture() {
}

QImage ScreenCapture::captureScreen() {
    // 获取屏幕的物理尺寸（物理像素）
    int physicalWidth = GetSystemMetrics(SM_CXSCREEN);
    int physicalHeight = GetSystemMetrics(SM_CYSCREEN);

    qDebug() << "Physical screen size:" << physicalWidth << "x" << physicalHeight;

    // 获取 DPI 缩放系数
    QScreen* screen = QGuiApplication::primaryScreen();
    qreal dpr = screen ? screen->devicePixelRatio() : 1.0;
    qDebug() << "DPI ratio:" << dpr;

    // 逻辑尺寸（Qt 使用的尺寸）
    int logicalWidth = physicalWidth / dpr;
    int logicalHeight = physicalHeight / dpr;
    qDebug() << "Logical screen size:" << logicalWidth << "x" << logicalHeight;

    // GDI 截图
    HWND hDesktopWnd = GetDesktopWindow();
    HDC hDesktopDC = GetDC(hDesktopWnd);
    HDC hCaptureDC = CreateCompatibleDC(hDesktopDC);

    if (!hDesktopDC || !hCaptureDC) {
        qDebug() << "Failed to get DC";
        return QImage();
    }

    // 创建兼容位图（物理尺寸）
    HBITMAP hBitmap = CreateCompatibleBitmap(hDesktopDC, physicalWidth, physicalHeight);
    if (!hBitmap) {
        qDebug() << "CreateCompatibleBitmap failed!";
        ReleaseDC(hDesktopWnd, hDesktopDC);
        DeleteDC(hCaptureDC);
        return QImage();
    }

    SelectObject(hCaptureDC, hBitmap);

    // 复制屏幕内容（物理像素）
    if (!BitBlt(hCaptureDC, 0, 0, physicalWidth, physicalHeight, hDesktopDC, 0, 0, SRCCOPY)) {
        qDebug() << "BitBlt failed!";
        DeleteObject(hBitmap);
        DeleteDC(hCaptureDC);
        ReleaseDC(hDesktopWnd, hDesktopDC);
        return QImage();
    }

    // 读取像素数据
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = physicalWidth;
    bi.biHeight = -physicalHeight;  // 负值表示自上而下
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    QImage result(physicalWidth, physicalHeight, QImage::Format_ARGB32);
    result.fill(Qt::black);

    int resultBytes = GetDIBits(hCaptureDC, hBitmap, 0, physicalHeight, result.bits(), (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    if (resultBytes == 0) {
        qDebug() << "GetDIBits failed!";
        DeleteObject(hBitmap);
        DeleteDC(hCaptureDC);
        ReleaseDC(hDesktopWnd, hDesktopDC);
        return QImage();
    }

    // 修复 Alpha 通道
    for (int y = 0; y < physicalHeight; ++y) {
        uchar* line = result.scanLine(y);
        for (int x = 0; x < physicalWidth; ++x) {
            line[x * 4 + 3] = 255;  // Alpha = 255
            // GDI 返回的是 BGR，不需要转换，QImage::Format_ARGB32 会正确处理
        }
    }

    // 清理资源
    DeleteObject(hBitmap);
    DeleteDC(hCaptureDC);
    ReleaseDC(hDesktopWnd, hDesktopDC);

    qDebug() << "GDI Capture success!";
    return result;
}

bool ScreenCapture::isInitialized() const {
    return m_initialized;
}