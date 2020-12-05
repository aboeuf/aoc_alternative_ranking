#include <manager.h>
#include <jsonhelper.h>
#include <leaderboard.h>
#include <iostream>
#include <QProcessEnvironment>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>

const QString cookies_path = "/home/aboeuf/.config/aoc_alternative_ranking/cookies.json";
const QString leaderboard_url = "https://adventofcode.com/2020/leaderboard/private/view/991947.json";

Manager::Manager()
{
  m_manager = new QNetworkAccessManager(this);
  connect(m_manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)), Qt::QueuedConnection);
  connect(this, SIGNAL(run()), this, SLOT(onRun()), Qt::QueuedConnection);
  emit run();
}

Manager::~Manager()
{
  delete m_manager;
}

void Manager::onRun()
{
  QString cookies = getCookies();
  if (cookies.isEmpty()) {
    emit stop();
    return;
  }
  QNetworkRequest request;
  request.setUrl(QUrl(leaderboard_url));
  request.setRawHeader("Cookie", cookies.toUtf8());
  m_manager->get(request);
}

void Manager::replyFinished(QNetworkReply* reply)
{
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  reply->deleteLater();

  //QString html = ("<!DOCTYPE html><html><title>AOC Alternative Ranking System</title><body>");
  QString html = "<!DOCTYPE html><html><title>AOC Alternative Ranking System</title><head><link rel=\"stylesheet\" href=\"styles.css\"></head><body><h1>Advent Of Code : classement alternatif </h1><p>Classement alternatif bas&eacute; sur le temps pass&eacute; entre la premi&egrave;re et la deuxi&egrave;me &eacute;toile.<br>Chaque jour le premier prends 6 points, le deuxi&egrave;me 5 points, etc.<br>En cas d'&eacute;galit&eacute; au g&eacute;n&eacute;ral, c'est la somme des d&eacute;lais qui compte.<br>Toutes les propositions de modifications sont les bienvenues.</p>";

  html += LeaderBoard(document).toHtml();
  html += "</body></html>";


  QFile test_file("/mnt/c/Users/aboeuf/Desktop/css/index.php");
  if (test_file.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream out(&test_file);
    out << html;
    test_file.close();
  }

  //std::cout << html.toStdString();
  emit stop();
}

QString Manager::getCookies()
{
  JsonHelper helper;
  QJsonObject cookies_object;
  if (!helper.read(cookies_path, cookies_object)) {
    QString error = helper.error();
    error.replace("\n", "<br>");
    std::cout << "Cannot read cookies<br>" << error.toStdString();
    return "";
  }

  QString value;
  if (!helper.read(cookies_object, "_ga", value)) {
    QString error = helper.error();
    error.replace("\n", "<br>");
    std::cout << "Cannot read cookies<br>" << error.toStdString();
    return "";
  }
  QString cookies = "_ga=" + value;

  if (!helper.read(cookies_object, "_gid", value)) {
    QString error = helper.error();
    error.replace("\n", "<br>");
    std::cout << "Cannot read cookies<br>" << error.toStdString();
    return "";
  }
  cookies += "; _gid=" + value;

  if (!helper.read(cookies_object, "session", value)) {
    QString error = helper.error();
    error.replace("\n", "<br>");
    std::cout << "Cannot read cookies<br>" << error.toStdString();
    return "";
  }
  cookies += "; session=" + value;

  return cookies;
}
