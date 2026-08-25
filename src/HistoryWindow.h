#ifndef HISTORYWINDOW_H
#define HISTORYWINDOW_H

#include <QDialog>
#include <QStringList>
#include <QTextEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

class HistoryWindow : public QDialog {
    Q_OBJECT
public:
    explicit HistoryWindow(const QString& title, QWidget* parent = nullptr);

    void addEntry(const QString& text);
    void clearHistory();
    void setHistory(const QStringList& history);

private slots:
    void onCopyAll();
    void onClear();

private:
    QTextEdit* m_textEdit;
    QPushButton* m_copyBtn;
    QPushButton* m_clearBtn;
    QStringList m_history;
};

#endif