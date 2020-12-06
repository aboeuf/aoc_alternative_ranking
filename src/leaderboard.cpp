#include "leaderboard.h"
#include <jsonhelper.h>
#include <QDateTime>
#include <set>

namespace {

constexpr int NB_DAYS = 25;
constexpr int MAX_WEIGHT = 3;

QString deltaToString(int delta) {
  if (delta < 0)
    return "N/A";
  QString delta_str = "";
  int days = delta / 86400;
  if (days > 0)
    delta_str = QString("%1d").arg(days);
  delta -= 86400 * days;
  int hours = delta / 3600;
  if (hours > 0 || !delta_str.isEmpty())
    delta_str += QString(" %1h").arg(hours);
  delta -= 3600 * hours;
  int minutes = delta / 60;
  if (minutes > 0 || !delta_str.isEmpty())
    delta_str += QString(" %1m").arg(minutes);
  delta -= 60 * minutes;
  delta_str += QString(" %1s").arg(delta);
  return delta_str;
}

double getWeight(int day, int max_nb_days, int max_weigth)
{
  double day_f = static_cast<double>(day - 1) / static_cast<double>(max_nb_days - 1);
  return static_cast<double>(max_weigth) * day;
}

}

QString DayResult::delta() const
{
  return deltaToString(m_delta_s);
}

bool Member::operator < (const Member& other) const {
  if (points() == other.points())
    return totalDelta() < other.totalDelta();
  return points() > other.points();
}

int Member::points() const
{
  int points = 0;
  for(const DayResult& res : m_results.values())
    points += res.m_points;
  return points;
}

int Member::totalDelta() const
{
  int total = 0;
  for (const DayResult& res : m_results.values())
    total += res.m_delta_s;
  return total;
}

QString Member::delta() const
{
  return deltaToString(totalDelta());
}

LeaderBoard::LeaderBoard(const QJsonDocument& document)
{
  bool ok;
  JsonHelper helper;
  QJsonObject object = document.object(), members;
  if (!helper.read(object, "members", members))
    return;
  for (const QJsonValue& member_value : members) {
    QJsonObject member_object = member_value.toObject();
    Member member;
    if (!helper.read(member_object, "name", member.m_name))
      continue;
    QJsonObject completion_day_level_object;
    if (!helper.read(member_object, "completion_day_level", completion_day_level_object))
      continue;
    for (const QString& day_key : completion_day_level_object.keys()) {
      int day = day_key.toInt(&ok);
      if (!ok)
        continue;
      if (day > m_day_max)
        m_day_max = day;
      QJsonObject results_object;
      if (!helper.read(completion_day_level_object, day_key, results_object))
        continue;
      DayResult result;
      QJsonObject result_object;
      QString result_string;
      bool t1 = false, t2 = false;
      QDateTime t1_dt, t2_dt;
      if (helper.read(results_object, "1", result_object) &&
          helper.read(result_object, "get_star_ts", result_string)) {
        int value = result_string.toInt(&ok);
        if (ok) {
          t1 = true;
          t1_dt = QDateTime::fromSecsSinceEpoch(value);
          result.m_first = t1_dt.time().toString();
        }
      }
      if (helper.read(results_object, "2", result_object) &&
          helper.read(result_object, "get_star_ts", result_string)) {
        int value = result_string.toInt(&ok);
        if (ok) {
          t2 = true;
          t2_dt = QDateTime::fromSecsSinceEpoch(value);
          result.m_second = t2_dt.time().toString();
        }
      }
      if (t1 && t2)
        result.m_delta_s = t1_dt.secsTo(t2_dt);
      member.m_results[day] = result;
    }
    m_members << member;
  }
}

QString LeaderBoard::toHtml()
{
  if (m_members.empty())
    return "EMPTY";
  QString general, details;

  for (int day = m_day_max; day > 0; --day) {
    details += QString("<p><h3>Classement jour %1</h3><table><tr>"
                       "<th>Rang</th>"
                       "<th>Joueur</th>"
                       "<th>*</th>"
                       "<th>**</th>"
                       "<th>D&eacute;lai</th>"
                       "<th>Points</th></tr>").arg(day);
    std::multimap<int, Member*> sorted;
    std::map<std::string, Member*> na;
    for (Member& member : m_members)
    {
      if (!member.m_results.contains(day))
        member.m_results[day] = DayResult();
      if (member.m_results[day].m_delta_s >= 0)
        sorted.insert(std::pair<int, Member*>(member.m_results[day].m_delta_s, &member));
      else
        na[member.m_name.toStdString()] = &member;
    }
    int nb_points = m_members.size();
    int rank = 1;
    for (auto it = sorted.begin(); it != sorted.end(); ++it) {
      it->second->m_results[day].m_points = nb_points;
      details += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td></tr>")
          .arg(rank)
          .arg(it->second->m_name)
          .arg(it->second->m_results[day].m_first)
          .arg(it->second->m_results[day].m_second)
          .arg(it->second->m_results[day].delta())
          .arg(nb_points);
      --nb_points;
      ++rank;
    }
    for (auto it = na.begin(); it != na.end(); ++it) {
      details += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>0</td></tr>")
          .arg(rank)
          .arg(it->second->m_name)
          .arg(it->second->m_results[day].m_first)
          .arg(it->second->m_results[day].m_second)
          .arg(it->second->m_results[day].delta());
      ++rank;
    }
    details += "</table></p>";
  }

  std::set<Member> general_ranking;
  for (Member& member : m_members)
    general_ranking.insert(member);
  general = QString("<p><h3>Classement General</h3><table><tr>"
                    "<th>Rang</th>"
                    "<th>Joueur</th>"
                    "<th>Points</th>"
                    "<th>D&eacute;lai Total</th></tr>"
                    );
  int i = 1;
  for (const Member& member : general_ranking) {
    general += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
        .arg(i)
        .arg(member.m_name)
        .arg(member.points())
        .arg(member.delta());
    ++i;
  }
  general += "</table></p>";
  return general + details;
}
