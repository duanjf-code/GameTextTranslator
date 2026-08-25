#include "HistoryWindow.h"
#include <QDateTime>
#include <QClipboard>
#include <QGuiApplication>
#include <QMessageBox>

HistoryWindow::HistoryWindow(const QString& title, QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(title);
    setMinimumSize(500, 400);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_textEdit = new QTextEdit(this);
    m_textEdit->setReadOnly(true);
    m_textEdit->setFont(QFont("Microsoft YaHei", 11));
    mainLayout->addWidget(m_textEdit);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_copyBtn = new QPushButton("复制全部", this);
    m_clearBtn = new QPushButton("清空记录", this);
    QPushButton* closeBtn = new QPushButton("关闭", this);

    btnLayout->addWidget(m_copyBtn);
    btnLayout->addWidget(m_clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_copyBtn, &QPushButton::clicked, this, &HistoryWindow::onCopyAll);
    connect(m_clearBtn, &QPushButton::clicked, this, &HistoryWindow::onClear);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
}

void HistoryWindow::setHistory(const QStringList& history) {
    m_history = history;
    m_textEdit->clear();
    for (const QString& entry : m_history) {
        m_textEdit->append(entry);
    }
    // 滚动到底部
    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
}

void HistoryWindow::addEntry(const QString& text) {
    if (text.isEmpty()) return;

    QString timestamp = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString entry = QString("[%1] %2").arg(timestamp).arg(text);

    m_history.append(entry);
    m_textEdit->append(entry);

    QTextCursor cursor = m_textEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_textEdit->setTextCursor(cursor);
}

void HistoryWindow::clearHistory() {
    m_history.clear();
    m_textEdit->clear();
}

void HistoryWindow::onCopyAll() {
    QString allText = m_history.join("\n");
    if (allText.isEmpty()) {
        QMessageBox::information(this, "提示", "没有内容可复制");
        return;
    }

    QClipboard* clipboard = QGuiApplication::clipboard();
    clipboard->setText(allText);
    QMessageBox::information(this, "成功", "已复制全部记录到剪贴板");
}

void HistoryWindow::onClear() {
    if (m_history.isEmpty()) return;

    QMessageBox::StandardButton reply = QMessageBox::question(
        this, "确认", "确定要清空所有记录吗？",
        QMessageBox::Yes | QMessageBox::No
        );

    if (reply == QMessageBox::Yes) {
        clearHistory();
    }
}