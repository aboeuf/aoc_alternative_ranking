#ifndef MANAGER_H
#define MANAGER_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class Manager : public QObject
{
  Q_OBJECT

public:
  Manager();
  ~Manager();

signals:
  void run();
  void stop();

private slots:
  void onRun();
  void replyFinished(QNetworkReply* reply);

private:
  QString getCookies();
  QNetworkAccessManager *m_manager;
};

#endif // MANAGER_H
