#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);

    // Connect signals and slots
    connect(ui->buttonGenerate, SIGNAL(clicked()), this, SLOT(generate()));
    connect(ui->linePassword, SIGNAL(textChanged(QString)), this, SLOT(textChanged()));
    connect(ui->linePassword, SIGNAL(returnPressed()), this, SLOT(generate()));
    connect(ui->comboService, SIGNAL(currentTextChanged(QString)), this, SLOT(textChanged()));
    connect(ui->comboService, SIGNAL(currentIndexChanged(QString)), this, SLOT(indexChanged()));
    connect(ui->buttonCopyUsername, SIGNAL(clicked()), this, SLOT(usernameToClipboard()));
    connect(ui->actionSavePassword, SIGNAL(triggered()), this, SLOT(savePassword()));
    connect(ui->actionEditServices, SIGNAL(triggered()), this, SLOT(editServices()));
    connect(ui->actionReloadServices, SIGNAL(triggered()), this, SLOT(loadServices()));
    connect(ui->actionDeletePassword, SIGNAL(triggered()), this, SLOT(deletePassword()));
    connect(&watcher, SIGNAL(finished()), this, SLOT(passwordToClipboard()));

    // Get system clipboard
    clipboard = QApplication::clipboard();

    // Load file of services and password
    loadServices();
    loadPassword();

    // Set password line to hide input
    ui->linePassword->setEchoMode(QLineEdit::Password);

    // Update UI
    textChanged(); 
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::generate() {

    // Check inputs
    if (ui->comboService->currentText().length() == 0 && ui->linePassword->text().length() == 0) {
        status("Enter details first", red);
        return;
    } else if (ui->linePassword->text().length() == 0) {
        status("Enter password", yellow);
        return;
    } else if (ui->comboService->currentText().length() < minServiceLength) {
        status("Service name too short", red);
        return;
    } else if (ui->linePassword->text().length() < minPwLength) {
        status("Password too short", red);
        return;
    }

    // Compute password
    status("Generating password...", cyan);
    QString charset = charsetMap.value(ui->comboService->currentText(), crypt_backend::defaultCharset);
    future = QtConcurrent::run(crypt_backend::generator, ui->linePassword->text(), ui->comboService->currentText().toLower(), charset);
    watcher.setFuture(future);

}

void MainWindow::passwordToClipboard() {
    clipboard->setText(future.result());
    status("Copied password to clipboard", green);
}

void MainWindow::usernameToClipboard() {
    clipboard->setText(usernameMap.value(ui->comboService->currentText()));
    status("Copied username to clipboard", green);
}

void MainWindow::textChanged() {
    // Check inputs
    if (ui->linePassword->text().length() < minPwLength && ui->comboService->currentText().length() < minServiceLength) {
        status("Enter details", yellow);
    } else if (ui->linePassword->text().length() < minPwLength) {
        status("Enter password", yellow);
    } else if (ui->comboService->currentText().length() < minServiceLength) {
        status("Enter service", yellow);
    } else {
        status("Ready to generate", green);
    }

    // Check if username is available
    QString username = usernameMap.value(ui->comboService->currentText(), "n/a");
    if (username != "n/a" && username != "") {
        ui->buttonCopyUsername->setText("Copy \"" + username + "\"");
        ui->buttonCopyUsername->setEnabled(true);
    } else {
        ui->buttonCopyUsername->setText("Copy Username");
        ui->buttonCopyUsername->setEnabled(false);
    }
}

void MainWindow::indexChanged() {
    textChanged();
    generate();
}

void MainWindow::loadServices() {
    servicesFile = new QFile("services.txt");
    if (servicesFile->open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString servicesString = QString::fromUtf8(servicesFile->readAll());
        QStringList servicesRows = servicesString.split('\n', QString::SplitBehavior::SkipEmptyParts);

        usernameMap.clear(); // Clear maps
        charsetMap.clear();
        foreach (QString row, servicesRows) { // Split services and usernames into maps
            QStringList buffer = row.split('/', QString::SplitBehavior::SkipEmptyParts);
            if (buffer.length() == 1) {             // If there is only a service
                usernameMap.insert(buffer.at(0), "n/a");
            } else if (buffer.length() == 2) {      // If there are service and username
                usernameMap.insert(buffer.at(0), buffer.at(1));
            } else if (buffer.length() == 3) {      // If there are service, username and charset
                usernameMap.insert(buffer.at(0), buffer.at(1));
                charsetMap.insert(buffer.at(0), buffer.at(2));
            }
        }

        QStringList services = QStringList(usernameMap.keys());
        for (int i = 0; i < (services.size()/2); i++) services.swap(i, services.size()-(1+i)); // Reverse list

        ui->comboService->blockSignals(true);
        ui->comboService->clear();
        ui->comboService->insertItems(0, services);
        ui->comboService->setCurrentText("");
        ui->comboService->setCurrentIndex(-1);
        ui->comboService->blockSignals(false);
        servicesFile->close();

        status("Reloaded services", green);
    } else {
        status("Could not access file", red);
    }
}

void MainWindow::loadPassword() {
    passwordFile = new QFile("password");
    if (passwordFile->open(QIODevice::ReadOnly)) {
        QByteArray* key = crypt_backend::getLocalKey();
        QByteArray* passwordBytes = crypt_backend::decrypt(passwordFile->readAll(), *key);
        QString passwordPlain = QString::fromUtf8(*passwordBytes);
        if (passwordPlain.contains("PW:")) { // Check if password is valid
            passwordPlain.remove(0, 3);
            ui->linePassword->setText(passwordPlain);
            status("Password loaded", green);
        } else {
            status("Password decryption failed", red);
        }
        passwordFile->close();
        delete key;
        delete passwordBytes;
    }
}

void MainWindow::savePassword() {
    if (ui->linePassword->text().length() < minPwLength) {
        status("Password too short", red);
        return;
    }
    if (passwordFile->open(QIODevice::WriteOnly)) {

        // Obtain key and encrypt password
        QByteArray* key = crypt_backend::getLocalKey();
        QByteArray passwordBytes = ("PW:" + ui->linePassword->text()).toUtf8();
        QByteArray* encrypted = crypt_backend::encrypt(passwordBytes, *key);

        // Save to file
        passwordFile->write(*encrypted);
        passwordFile->close();

        // Update UI
        status("Password saved", green);

        delete key;
        delete encrypted;
    } else {
        status("Could not access file", red);
    }
}

void MainWindow::deletePassword() {
    passwordFile = new QFile("password");
    if (passwordFile->open(QIODevice::WriteOnly)) {
        passwordFile->remove();
        status("Password deleted", green);
    } else {
        status("Could not access file", red);
    }
}

void MainWindow::editServices() {
    QDesktopServices::openUrl(QUrl::fromLocalFile(QFileInfo("services.txt").absoluteFilePath()));
}

void MainWindow::status(QString status, QString color) { // Color: "#rrggbb"
    ui->buttonStatus->setText(status);
    ui->buttonStatus->setStyleSheet("background-color: " + color);
}
