#include "leaderboard.h"
#include <jsonhelper.h>
#include <QDateTime>
#include <set>

namespace {

constexpr int NB_DAYS = 25;
constexpr int MAX_WEIGHT = 3;

QString delayToString(int delay) {
  if (delay < 0)
    return "N/A";
  QString delay_str = "";
  int days = delay / 86400;
  if (days > 0)
    delay_str = QString("%1d").arg(days);
  delay -= 86400 * days;
  int hours = delay / 3600;
  if (hours > 0 || !delay_str.isEmpty())
    delay_str += QString(" %1h").arg(hours);
  delay -= 3600 * hours;
  int minutes = delay / 60;
  if (minutes > 0 || !delay_str.isEmpty())
    delay_str += QString(" %1m").arg(minutes);
  delay -= 60 * minutes;
  delay_str += QString(" %1s").arg(delay);
  return delay_str;
}

double getWeight(int day, int max_nb_days = NB_DAYS, int max_weigth = MAX_WEIGHT)
{
  double day_f = static_cast<double>(day - 1) / static_cast<double>(max_nb_days - 1);
  return static_cast<double>(max_weigth - 1) * day_f + 1.0;
}

}

QString DayResult::delay() const
{
  return delayToString(m_delay_s);
}

bool Member::operator < (const Member& other) const {
  if (score() == other.score())
    return totaldelay() < other.totaldelay();
  return score() > other.score();
}

double Member::score() const
{
  double total = 0;
  for(const int& day : m_results.keys())
    total += score(day);
  return total;
}

double Member::score(int day) const
{
 if (m_results.contains(day))
   return getWeight(day) * static_cast<double>(m_results[day].m_points);
 return 0.0;
}

int Member::points() const
{
  int total = 0;
  for(const DayResult& res : m_results.values())
    total += res.m_points;
  return total;
}

int Member::totaldelay() const
{
  int total = 0;
  for (const DayResult& res : m_results.values())
    total += res.m_delay_s;
  return total;
}

QString Member::delay() const
{
  return delayToString(totaldelay());
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
        result.m_delay_s = t1_dt.secsTo(t2_dt);
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
    int mean_delay = 0;
    int min_delay = std::numeric_limits<int>::max();
    int max_delay = std::numeric_limits<int>::min();
    int nb_delay = 0;
    std::multimap<int, Member*> sorted;
    std::map<std::string, Member*> na;
    for (Member& member : m_members) {
      if (!member.m_results.contains(day))
        member.m_results[day] = DayResult();
      if (member.m_results[day].m_delay_s >= 0) {
        sorted.insert(std::pair<int, Member*>(member.m_results[day].m_delay_s, &member));
        mean_delay += member.m_results[day].m_delay_s;
        ++nb_delay;
        min_delay = std::min(member.m_results[day].m_delay_s, min_delay);
        max_delay = std::max(member.m_results[day].m_delay_s, max_delay);
      }
      else
        na[member.m_name.toStdString()] = &member;
    }
    int range = max_delay - min_delay;
    mean_delay = nb_delay == 0 ? -1 : mean_delay / nb_delay;
    details += QString("<h3>Classement jour %1 (x %2)</h3>"
                       "<p>D&eacute;lai moyen : %3</p><table><tr>"
                       "<th>Rang</th>"
                       "<th>Joueur</th>"
                       "<th>*</th>"
                       "<th>**</th>"
                       "<th>D&eacute;lai</th>"
                       "<th>Points</th>"
                       "<th>Score</th></tr>")
        .arg(day)
        .arg(getWeight(day), 0, 'f', 2)
        .arg(delayToString(mean_delay));
    int rank = 1;
    int nb_points = m_members.size();
    for (auto it = sorted.begin(); it != sorted.end(); ++it) {
      QString delta_to_mean_str = "N/A";
      if (mean_delay >= 0)
      {
        int delta_to_mean = mean_delay - it->second->m_results[day].m_delay_s;
        if (delta_to_mean < 0)
          delta_to_mean_str = "- " + delayToString(-delta_to_mean);
        else
          delta_to_mean_str = "+ " + delayToString(delta_to_mean);
        it->second->m_results[day].m_points = nb_points--;
      }
      details += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>%6</td><td>%7</td></tr>")
          .arg(rank)
          .arg(it->second->m_name)
          .arg(it->second->m_results[day].m_first)
          .arg(it->second->m_results[day].m_second)
          .arg(it->second->m_results[day].delay())
          .arg(it->second->m_results[day].m_points)
          .arg(it->second->score(day), 0, 'f', 2);
      ++rank;
    }
    for (auto it = na.begin(); it != na.end(); ++it) {
      details += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td><td>%5</td><td>0</td><td>0.00</td></tr>")
          .arg(rank)
          .arg(it->second->m_name)
          .arg(it->second->m_results[day].m_first)
          .arg(it->second->m_results[day].m_second)
          .arg(it->second->m_results[day].delay());
      ++rank;
    }
    details += "</table>";
  }

  std::set<Member> general_ranking;
  for (Member& member : m_members)
    general_ranking.insert(member);
  general = QString("<h3>Classement General</h3><table><tr>"
                    "<th>Rang</th>"
                    "<th>Joueur</th>"
                    "<th>Score</th></tr>"
                    );
  int i = 1;
  for (const Member& member : general_ranking) {
    general += QString("<tr><td>%1</td><td>%2</td><td>%3</td></tr>")
        .arg(i)
        .arg(member.m_name)
        .arg(member.score(), 0, 'f', 2);
    ++i;
  }
  general += "</table>";
  return general + details;
}
