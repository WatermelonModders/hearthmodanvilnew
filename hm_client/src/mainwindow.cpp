/*
   hm_client -  hearthmod client
   Copyright (C) 2016 Filip Pancik

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <QtWidgets/QScrollBar>
#include <QtWidgets/QStyle>
#include <QtWidgets/QToolButton>
#include <QtWidgets/QMessageBox>
#include <QtNetwork/QSslCipher>
#include <QCryptographicHash>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>

#include "proto.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"

static int version = 1;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{
    socket = NULL;
    ui = new Ui::MainWindow;
    ui->setupUi(this);

    m_isReady = true;

    secureConnect();

    ncards = 0;

    connect(ui->loginButton, SIGNAL(clicked()),
            this, SLOT(login()));

    connect(ui->pushPlay, SIGNAL(clicked()),
            this, SLOT(play()));
    downloadFileFromURL("http://hearthmod.com/static/checksum", "../../hearthstone/Data/Win", E_CHECKSUM);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::socketError(QAbstractSocket::SocketError)
{
    addLog("[Lobby]: Connection error: " + socket->errorString());

    QMessageBox::critical(this, QString(socket->errorString()), QString("Connection error"));

    if(socket->error() == 13) {
        return;
    }

    exit(0);
}

void MainWindow::play()
{
#ifdef __linux__
    int r = system("cd ../../hearthstone/ && sh linux");
#else
    QString run = "RunAsDate.exe \"01\\04\\2016\" \"" + QDir::currentPath().replace("/", "\\") + "\\..\\..\\hearthstone\\Hearthstone.exe\" -launch";
    QByteArray ba = run.toLatin1();
    int r = system(ba.data());
    if(r != 0) {
        addLog("Executable not found ");
    }
#endif
}

void MainWindow::login()
{
    if(ui->editUsername->text().length() < 2 || ui->editUsername->text().length() > 32 ||
       ui->editPassword->text().length() < 2 || ui->editPassword->text().length() > 32
            ) {
        addLog("[Lobby]: Username and Password must be between 2 and 32 chars");
        return;
    }

    QJsonDocument doc;
    QJsonObject obj;

    obj["user"] = ui->editUsername->text();
    obj["pass"] = ui->editPassword->text();

    doc.setObject(obj);

    QByteArray js = doc.toJson();

    char output[128];
    int plen = proto.packet(output, P_LOGIN, js.data(), js.length());

    socket->write(output, plen);
}

void MainWindow::socketStateChanged(QAbstractSocket::SocketState state)
{
    qDebug() << "state changed " << state;
}

void MainWindow::addLog(QString msg)
{
    if(ui->listLogs->count() > 10) {
        ui->listLogs->clear();
    }
    ui->listLogs->addItem(msg);
}

void MainWindow::socketReadyRead()
{
    QByteArray qb = socket->readAll();

    QJsonDocument doc(QJsonDocument::fromJson(qb));
    QJsonObject json = doc.object();

    qint32 type = json["type"].toInt();
    qint32 error = json["error"].toInt();
    QString secret = json["secret"].toString();

    if(error != 0) {
        switch(error) {
            case 1:
            case 2:
                addLog(QString("[Lobby]: Incorrect login details, to create an account go to hearthmod.com"));
            break;

            case 3:
                addLog(QString("[Lobby]: No deck found, please go to hearthmod.com and create your deck"));
            break;
        }
        return;
    }

    if(type == P_LOGIN) {
        addLog(QString("[Lobby]: Login successful"));
        addLog(QString("[Lobby]: Deck found"));
        ui->editPassword->setDisabled(true);
        ui->editUsername->setDisabled(true);
        ui->loginButton->setDisabled(true);
        ui->player1->setDisabled(true);
        ui->player2->setDisabled(true);

        ui->checkDeck->setChecked(1);
        ui->pushPlay->setEnabled(1);

        QString cmd = "Windows Registry Editor Version 5.00\n\
\n\
[HKEY_CURRENT_USER\\Software\\Blizzard Entertainment\\Battle.net\\Launch Options\\WTCG]\n\
\"REGION\"=\"EU\"\n\
\"HBS_TOKENX\"=\"<>\"\n\
";
        cmd.replace("<>", secret);
        if(ui->player1->isChecked()) {
            cmd.replace("HBS_TOKENX", "HBS_TOKEN0");
#ifdef __linux__
            int r = system("cp ../../hearthstone/Hearthstone_Data/Managed/player1/Assembly-CSharp.dll ../../hearthstone/Hearthstone_Data/Managed/");
            if(r != 0) {
                qDebug() << "couldn't copy";
            }
#else
            int r = system("copy ..\\..\\hearthstone\\Hearthstone_Data\\Managed\\player1\\Assembly-CSharp.dll ..\\..\\hearthstone\\Hearthstone_Data\\Managed\\ ");
            if(r != 0) {
                qDebug() << "couldn't copy";
            }
#endif
        } else {
            cmd.replace("HBS_TOKENX", "HBS_TOKEN1");

#ifdef __linux__
            int r = system("cp ../../hearthstone/Hearthstone_Data/Managed/player2/Assembly-CSharp.dll ../../hearthstone/Hearthstone_Data/Managed/");
            if(r != 0) {
                qDebug() << "couldn't copy";
            }
#else
            int r = system("copy ..\\..\\hearthstone\\Hearthstone_Data\\Managed\\player2\\Assembly-CSharp.dll ..\\..\\hearthstone\\Hearthstone_Data\\Managed\\ ");
            if(r != 0) {
                qDebug() << "couldn't copy";
            }
#endif
        }

        QString filename = "tmp.reg";
        QFile file(filename);
        if(file.open(QIODevice::ReadWrite)) {
            QTextStream stream(&file);
            stream << cmd << endl;
            file.close();
            //qDebug() << QDir::currentPath();
#ifdef __linux__
            int r = system("regedit tmp.reg");
#else
            int r = system("tmp.reg");
#endif
            if(r != 0) {
                addLog(QString("[Lobby]: Cannot register client"));
                ui->pushPlay->setEnabled(0);
            }
#ifdef __linux__
            r = system("rm tmp.reg");
#else
            r = system("del tmp.reg");
#endif
            if(r != 0) {
                addLog(QString("[Lobby]: Cannot clean client"));
            }
        }
    }
}

void MainWindow::sslErrors(const QList<QSslError> &errors)
{
    socket->ignoreSslErrors();

    if (socket->state() != QAbstractSocket::ConnectedState) {
        socketStateChanged(socket->state());
    }
}

void MainWindow::secureConnect()
{
    if (!socket) {
        socket = new QSslSocket(this);

        connect(socket, SIGNAL(stateChanged(QAbstractSocket::SocketState)),
                this, SLOT(socketStateChanged(QAbstractSocket::SocketState)));
        connect(socket, SIGNAL(encrypted()),
                this, SLOT(socketEncrypted()));
        connect(socket, SIGNAL(error(QAbstractSocket::SocketError)),
                this, SLOT(socketError(QAbstractSocket::SocketError)));
        connect(socket, SIGNAL(sslErrors(QList<QSslError>)),
                this, SLOT(sslErrors(QList<QSslError>)));
        connect(socket, SIGNAL(readyRead()),
                this, SLOT(socketReadyRead()));
    }

    addLog(QString("[Lobby]: Connecting to hearthmod.com"));

    socket->connectToHostEncrypted("hearthmod.com", 1119);

    updateEnabledState();
}

void MainWindow::socketEncrypted()
{
    qDebug() << "socket encrypted";
}

void MainWindow::updateEnabledState()
{
    qDebug() << "state: "<< socket->state();
}

void MainWindow::downloadFileFromURL(const QString &url, const QString &filePath, enum download_step_e step) {
    if (!m_isReady)
        return;
    m_isReady = false;

    assets_step = step;

    const QString fileName = filePath + url.right(url.size() - url.lastIndexOf("/")); // your filePath should end with a forward slash "/"
    m_file = new QFile();
    m_file->setFileName(fileName);
    m_file->open(QIODevice::WriteOnly);
    if (!m_file->isOpen()) {
        m_isReady = true;
        addLog(QString("Couldn't open file " + fileName));
        return;
    }

    if(assets_step == E_CHECKSUM) {
        addLog(QString("[Update]: Version checking.. "));
    } else {
        addLog(QString("[Update]: Updating assets.. "));
    }

    QNetworkAccessManager *manager = new QNetworkAccessManager;

    QNetworkRequest request;
    request.setUrl(QUrl(url));

    connect(manager, SIGNAL(finished(QNetworkReply *)), this, SLOT(onDownloadFileComplete(QNetworkReply *)));

    manager->get(request);
}

void MainWindow::onDownloadFileComplete(QNetworkReply *reply) {
    int update = 0;

    if (!m_file->isWritable()) {
        m_isReady = true;
        addLog(QString("Update couldn't complete"));
        return; // TODO: error check
    }

    if(assets_step == E_CHECKSUM) {
        // write to checksum
        QString cs = reply->readAll();
        if(cs.length() < 1) {
            QMessageBox::critical(this, tr("Version check"), "Version check failed. Server is most likely offline.");
            //QMessageBox::critical(this, QString(socket->errorString()), QString("Connection error"));
            exit(0);
        }

        m_file->write(cs.toStdString().c_str());
        m_file->close();

        QFile f("../../hearthstone/Data/Win/local_checksum");
        f.open(QFile::ReadOnly | QFile::Text);
        QTextStream in(&f);
        QString checksum_local = in.readAll();
        f.close();

        assets_step = E_ASSETS;

        if(cs == checksum_local) {
            addLog(QString("[Update]: Assets up to date."));
        } else {
            addLog(QString("[Update]: Update required."));
            update = 1;
        }

        // write to local checksum
        QFile f1("../../hearthstone/Data/Win/local_checksum");
        f1.open(QFile::ReadWrite);
        f1.write(cs.toStdString().c_str());
        f1.close();

        m_isReady = true;

        if(update == 1) {
            ui->editPassword->setDisabled(true);
            ui->editUsername->setDisabled(true);
            ui->loginButton->setDisabled(true);
            ui->player1->setDisabled(true);
            ui->player2->setDisabled(true);

            downloadFileFromURL("http://hearthmod.com/static/cardxml0.unity3d", "../../hearthstone/Data/Win", E_ASSETS);
        } else {
            ui->checkAssets->setChecked(1);
        }
    } else {
        addLog(QString("[Update]: Assets downloaded."));

        m_file->write(reply->readAll());
        m_file->close(); // TODO: delete the file from the system later on
        m_isReady = true;

        ui->editPassword->setDisabled(false);
        ui->editUsername->setDisabled(false);
        ui->loginButton->setDisabled(false);
        ui->player1->setDisabled(false);
        ui->player2->setDisabled(false);

        ui->checkAssets->setChecked(1);
    }
}
