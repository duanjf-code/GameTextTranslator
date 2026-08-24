#include "GlobalHotkey.h"
#include <QCoreApplication>
#include <QDebug>

GlobalHotkey::GlobalHotkey(QObject* parent) : QObject(parent) {
    if (qApp) {
        qApp->installNativeEventFilter(this);
    } else {
        qDebug() << "Warning: qApp is null in GlobalHotkey constructor";
    }
}

GlobalHotkey::~GlobalHotkey() {
    qApp->removeNativeEventFilter(this);
    unregisterAll();
}

bool GlobalHotkey::convertKeySequence(const QKeySequence& sequence, UINT& modifiers, UINT& key) {
    if (sequence.isEmpty()) return false;

    int qtKey = sequence[0];
    modifiers = 0;
    key = 0;

    // 解析修饰键
    if (qtKey & Qt::ControlModifier) modifiers |= MOD_CONTROL;
    if (qtKey & Qt::ShiftModifier) modifiers |= MOD_SHIFT;
    if (qtKey & Qt::AltModifier) modifiers |= MOD_ALT;
    if (qtKey & Qt::MetaModifier) modifiers |= MOD_WIN;

    // 解析主键
    int baseKey = qtKey & ~Qt::KeyboardModifierMask;

    // 特殊键映射
    if (baseKey >= Qt::Key_F1 && baseKey <= Qt::Key_F24) {
        key = VK_F1 + (baseKey - Qt::Key_F1);
    } else if (baseKey == Qt::Key_Escape) {
        key = VK_ESCAPE;
    } else if (baseKey == Qt::Key_Return || baseKey == Qt::Key_Enter) {
        key = VK_RETURN;
    } else if (baseKey == Qt::Key_Space) {
        key = VK_SPACE;
    } else if (baseKey == Qt::Key_Tab) {
        key = VK_TAB;
    } else if (baseKey >= Qt::Key_A && baseKey <= Qt::Key_Z) {
        key = baseKey;  // A-Z 直接映射
    } else if (baseKey >= Qt::Key_0 && baseKey <= Qt::Key_9) {
        key = baseKey;  // 0-9 直接映射
    } else {
        qDebug() << "Unsupported key:" << baseKey;
        return false;
    }

    return true;
}

bool GlobalHotkey::registerHotkey(int id, const QKeySequence& sequence) {
    UINT modifiers, key;
    if (!convertKeySequence(sequence, modifiers, key)) {
        qDebug() << "Failed to convert key sequence:" << sequence;
        return false;
    }

    m_hotkeySequences[id] = sequence;
    return registerHotkey(id, modifiers, key);
}

bool GlobalHotkey::registerHotkey(int id, UINT modifiers, UINT key) {
    if (!RegisterHotKey(nullptr, id, modifiers, key)) {
        qDebug() << "RegisterHotKey failed for ID:" << id << "error:" << GetLastError();
        return false;
    }

    m_hotkeyIds[id] = (modifiers << 16) | key;
    qDebug() << "Hotkey registered:" << id;
    return true;
}

bool GlobalHotkey::unregisterHotkey(int id) {
    if (m_hotkeyIds.contains(id)) {
        m_hotkeyIds.remove(id);
        m_hotkeySequences.remove(id);
        return UnregisterHotKey(nullptr, id);
    }
    return false;
}

void GlobalHotkey::unregisterAll() {
    for (int id : m_hotkeyIds.keys()) {
        UnregisterHotKey(nullptr, id);
    }
    m_hotkeyIds.clear();
    m_hotkeySequences.clear();
}

void GlobalHotkey::updateHotkey(int id, const QKeySequence& newSequence) {
    unregisterHotkey(id);
    registerHotkey(id, newSequence);
}

bool GlobalHotkey::nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) {
    Q_UNUSED(result);

    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        if (msg->message == WM_HOTKEY) {
            int id = static_cast<int>(msg->wParam);
            if (m_hotkeyIds.contains(id)) {
                emit hotkeyPressed(id);
                return true;
            }
        }
    }
    return false;
}