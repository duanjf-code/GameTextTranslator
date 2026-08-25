#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QPushButton>
#include <QKeySequenceEdit>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QComboBox>
#include <QPushButton>
#include <QSpinBox>

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    void setConfigPath(const QString& path);  // 设置当前配置路径

protected:
    void showEvent(QShowEvent* event) override;

private slots:
    void onSave();
    void onLoad();
    void onCancel();
    void onSaveAs();

private:
    void loadSettings();
    void saveSettings();
    void loadFromFile(const QString& path);
    void saveToFile(const QString& path);
    void applySettings();
    QString getDefaultConfigPath();

    QLineEdit* m_appIdEdit;
    QLineEdit* m_secretKeyEdit;
    QKeySequenceEdit* m_hotkeyEdit;
    QCheckBox* m_autoStartCheck;
    QComboBox* m_ocrLangCombo;
    QComboBox* m_targetLangCombo;
    QPushButton* m_saveBtn;
    QPushButton* m_saveAsBtn;
    QPushButton* m_loadBtn;
    QPushButton* m_cancelBtn;
    QLabel* m_configPathLabel;
    QSpinBox* m_intervalSpinBox;

    QString m_currentConfigPath;
};

#endif