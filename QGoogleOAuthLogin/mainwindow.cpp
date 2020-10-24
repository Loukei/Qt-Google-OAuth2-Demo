#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "googleoauth2wrapper.h"
#include <QFileDialog>
#include <QDesktopServices>
#include <QDebug>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->actionLogin,&QAction::triggered,
            this,&MainWindow::onLogin);
    connect(ui->actionRefersh_token,&QAction::triggered,
            this,&MainWindow::onRefreshAccessToken);
    ui->statusbar->showMessage("Ready");
    QTimer::singleShot(1000,this,&MainWindow::initialOAuth);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initialOAuth()
{
    ui->textBrowser->append(tr("Loading file ") + m_filename + "...");
    //! if initial is not first time execute
    if(!QFile::exists(m_filename)){
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open OAuth setting File"),
                                                        m_filename,
                                                        tr("Json檔案 (*json)"));
        m_filename = fileName.trimmed();
    }
    auto param = parseJson(m_filename);
    if(param.isEmpty()){
        ui->textBrowser->append(tr("[Error]Loading error, check file is exist or content."));
        return;
    }
    ui->textBrowser->append(tr("Loading sucess!"));
    ui->textBrowser->append(param.toString());
    m_google = setupOAuth2(param,nullptr,this);
    connect(m_google,&QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this,&MainWindow::onGoogleAuthorizeWithBrowser);
    connect(m_google,&QOAuth2AuthorizationCodeFlow::granted,
            this,&MainWindow::onGoogleGranted);
}

void MainWindow::onLogin()
{
    if(!m_google){
        ui->textBrowser->append(tr("Google auth not exist."));
        return;
    }
    m_google->grant();
}

void MainWindow::onRefreshAccessToken()
{
    if(!m_google){
        ui->textBrowser->append(tr("Google auth not exist."));
        return;
    }
    ui->textBrowser->append(tr("Refresh token..."));
    m_google->refreshAccessToken();
}

void MainWindow::onGoogleGranted()
{
    ui->lineEdit_AccessToken->setText(m_google->token());
    ui->lineEdit_RefreshToken->setText(m_google->refreshToken());
    ui->textBrowser->append(tr("Google authorized."));
}

void MainWindow::onGoogleAuthorizeWithBrowser(const QUrl &url)
{
    ui->textBrowser->append(tr("Open with browser:") + url.toString());
    QDesktopServices::openUrl(url);
}
