#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QClipboard>
#include <QByteArray>
#include <QCryptographicHash>
#include <QFuture>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include "crypt_backend.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow* ui;
    QFile* servicesFile;
    QFile* passwordFile;
    QClipboard* clipboard;
    QMap<QString, QString> usernameMap;
    QMap<QString, QString> charsetMap;
    QFuture<QString> future;
    QFutureWatcher<QString> watcher;

    /*!
     * \brief Minimum length of master password.
     */
    inline static const int minPwLength = 8;

    /*!
     * \brief Minimum length of service name.
     */
    inline static const int minServiceLength = 3;

    /*!
     * \brief Updates status button at the buttom of the UI.
     * \param status    New status string.
     * \param color     New status color.
     */
    void status(QString status, QString color = "#ffffff");

private slots:

    /*!
     * \brief Starts generating password in separate thread and saves master password if selected.
     */
    void generate();

    /*!
     * \brief Copies service-specific password to clipboard when generation is finished.
     */
    void passwordToClipboard();

    /*!
     * \brief Copies service-specific username to clipboard.
     */
    void usernameToClipboard();

    /*!
     * \brief Updates UI whenever text is changed.
     */
    void textChanged();

    /*!
     * \brief Updates UI whenever combobox index is changed.
     */
    void indexChanged();
};
#endif // MAINWINDOW_H
