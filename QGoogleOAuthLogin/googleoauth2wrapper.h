#ifndef GOOGLEOAUTH2WRAPPER_H
#define GOOGLEOAUTH2WRAPPER_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QList>
#include <QDebug>

struct OAuth2Parameter
{
    QString scope;
    QUrl authorizationUrl;
    QString clientIdentifier;
    QUrl accessTokenUrl;
    QString clientIdentifierSharedKey;
    QList<QUrl> redirectUris;
    inline bool isEmpty() const{
        return scope.isEmpty() && authorizationUrl.isEmpty() && accessTokenUrl.isEmpty() &&
                clientIdentifier.isEmpty() && clientIdentifierSharedKey.isEmpty() && redirectUris.isEmpty();
    }
    QString toString() const{
        QString msg;
        QTextStream out(&msg);
        out << "OAuth2Parameter{\n"
            << "scope: " << this->scope << "\n"
            << "authorizationUrl: " << this->authorizationUrl.toString() << "\n"
            << "clientIdentifier: " << this->clientIdentifier << "\n"
            << "accessTokenUrl: " << this->accessTokenUrl.toString() << "\n"
            << "clientIdentifierSharedKey: " << this->clientIdentifierSharedKey << "\n"
            << "redirectUris: ";
        out << "[";
        for(auto url:this->redirectUris){
            out << url.toDisplayString() << ",\n";
        }
        out << "]\n}";
        return msg;
    }
};
//! make OAuth2Parameter print to qDebug()
QDebug operator<<(QDebug debug, OAuth2Parameter parameter){
    QDebugStateSaver saver(debug);
    debug.nospace() << "OAuth2Parameter("
                    << "scope:" << parameter.scope
                    << ",authorizationUrl:" << parameter.authorizationUrl
                    << ",clientIdentifier:" << parameter.clientIdentifier
                    << ",accessTokenUrl:" << parameter.accessTokenUrl
                    << ",clientIdentifierSharedKey:" << parameter.clientIdentifierSharedKey
                    << ",redirectUris:" << parameter.redirectUris
                    <<")";
    return debug;
}
//! parse json file download form google api console
OAuth2Parameter parseJson(const QString &filepath){
    QFile file(filepath);
    if(!file.open(QIODevice::ReadOnly|QIODevice::Text)){
        qWarning() << QStringLiteral("File not open:") << file.errorString();
        return OAuth2Parameter();
    }
    auto jsondoc = QJsonDocument::fromJson(file.readAll());
    auto webobj = jsondoc["web"].toObject();
    OAuth2Parameter parameter;
    parameter.scope = "https://www.googleapis.com/auth/drive.file";
    parameter.authorizationUrl = webobj["auth_uri"].toString();
    parameter.clientIdentifier = webobj["client_id"].toString();
    parameter.accessTokenUrl = webobj["token_uri"].toString();
    parameter.clientIdentifierSharedKey = webobj["client_secret"].toString();
    for(auto jsonval : webobj["redirect_uris"].toArray()){
        QUrl redirect_url(jsonval.toString());
        parameter.redirectUris.append(redirect_url);
    }
    return parameter;
}
//! modify parameter to fit google setting
QAbstractOAuth::ModifyParametersFunction buildModifyParametersFunction(QUrl clientIdentifier,QUrl clientIdentifierSharedKey){
    return [clientIdentifier,clientIdentifierSharedKey]
            (QAbstractOAuth::Stage stage, QVariantMap *parameters){
        if(stage == QAbstractOAuth::Stage::RequestingAuthorization){
            /* setup Authoriation Url here */
//            parameters->insert("redirect_uri","http://127.0.0.1:8080/");
            parameters->insert("access_type","offline"); /* Request refresh token*/
            parameters->insert("prompt","consent"); /* force user check scope again */
        }
        if(stage == QAbstractOAuth::Stage::RefreshingAccessToken){
            parameters->insert("client_id",clientIdentifier);
            parameters->insert("client_secret",clientIdentifierSharedKey);
        }
    };
}

QOAuth2AuthorizationCodeFlow* setupOAuth2(const OAuth2Parameter &parameter,QNetworkAccessManager *manager = nullptr,QObject *parent = nullptr)
{
    if(parameter.isEmpty()){
        qWarning() << QStringLiteral("Null OAuth2Parameter.");
        return nullptr;
    }

    if(!manager){
        manager = new QNetworkAccessManager(parent);
    }
    auto google = new QOAuth2AuthorizationCodeFlow(manager,parent);
    google->setScope(parameter.scope);
    google->setAuthorizationUrl(parameter.authorizationUrl);
    google->setClientIdentifier(parameter.clientIdentifier);
    google->setAccessTokenUrl(parameter.accessTokenUrl);
    google->setClientIdentifierSharedKey(parameter.clientIdentifierSharedKey);
    /* reset Oauth parameter to fit Google OAuth situation */
    google->setModifyParametersFunction(buildModifyParametersFunction(parameter.clientIdentifier,parameter.clientIdentifierSharedKey));

    auto port = parameter.redirectUris[0].port();
    auto replyHandler = new QOAuthHttpServerReplyHandler(port, parent);
    google->setReplyHandler(replyHandler);
    return google;
}

#endif // GOOGLEOAUTH2WRAPPER_H
