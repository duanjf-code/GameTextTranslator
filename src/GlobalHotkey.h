#ifndef GLOBALHOTKEY_H
#define GLOBALHOTKEY_H

#include <QObject>
#include <QAbstractNativeEventFilter>
#include <QHash>
#include <QKeySequence>
#include <windows.h>

class GlobalHotkey : public QObject, public QAbstractNativeEventFilter {
    Q_OBJECT
public:
    explicit GlobalHotkey(QObject* parent = nullptr);
    ~GlobalHotkey();

    bool registerHotkey(int id, const QKeySequence& sequence);
    bool registerHotkey(int id, UINT modifiers, UINT key);
    bool unregisterHotkey(int id);
    void unregisterAll();

    void updateHotkey(int id, const QKeySequence& newSequence);

signals:
    void hotkeyPressed(int id);

protected:
    bool nativeEventFilter(const QByteArray& eventType, void* message, qintptr* result) override;

private:
    QHash<int, UINT> m_hotkeyIds;
    QHash<int, QKeySequence> m_hotkeySequences;  // 用于重新注册
    bool convertKeySequence(const QKeySequence& sequence, UINT& modifiers, UINT& key);
};

#endif