#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
class QOAuth2AuthorizationCodeFlow;
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QOAuth2AuthorizationCodeFlow *m_google = nullptr;
    QString m_filename = "client_secret.json"; //default oauth parameter loading path

private slots:
    void initialOAuth();
    void onLogin();
    void onRefreshAccessToken();
    void onGoogleGranted();
    void onGoogleAuthorizeWithBrowser(const QUrl &url);
};
#endif // MAINWINDOW_H
