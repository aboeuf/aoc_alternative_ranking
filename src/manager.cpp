#include <manager.h>
#include <jsonhelper.h>
#include <leaderboard.h>
#include <iostream>
#include <QProcessEnvironment>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QTextStream>
#include <QDir>

//#define TEST_BUILD

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

#ifdef TEST_BUILD
void exportResult(const QString& result)
{
  QFile html_in("../index.php");
  QString html;
  if (html_in.open(QFile::ReadOnly | QFile::Text)) {
    while (!html_in.atEnd()) {
      QString line(html_in.readLine());
      while (!line.isEmpty() && (line.endsWith('\n') || line.endsWith(' ')))
        line.chop(1);
      while (!line.isEmpty() && line[0] == ' ')
        line.remove(0, 1);
      if (!line.isEmpty())
        html += line;
    }
    html_in.close();
  }
  html.replace("<?phpecho exec('aoc_alternative_ranking');?>", result);
  QDir().mkdir("test_build");
  QFile html_out("test_build/index.php");
  if (html_out.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream out(&html_out);
    out << html;
    html_out.close();
  }
  QFile css_in("../styles.css");
  if (css_in.open(QFile::ReadOnly | QFile::Text)) {
    QFile css_out("test_build/styles.css");
    if (css_out.open(QFile::WriteOnly | QFile::Text)) {
      QTextStream out(&css_out);
      out << css_in.readAll();
      css_out.close();
    }
    css_in.close();
  }
}
#endif

void Manager::replyFinished(QNetworkReply* reply)
{
  QJsonDocument document = QJsonDocument::fromJson(reply->readAll());
  reply->deleteLater();
#ifdef TEST_BUILD
  exportResult(LeaderBoard(document).toHtml());
#else
  std::cout << LeaderBoard(document).toHtml().toStdString();
#endif
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
