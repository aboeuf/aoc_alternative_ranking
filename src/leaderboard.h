#ifndef LEADERBOARD_H
#define LEADERBOARD_H

#include <QString>
#include <QDateTime>
#include <QMap>

struct DayResult
{
  QString delta() const;
  QString m_first{"N/A"};
  QString m_second{"N/A"};
  int m_delta_s{-1};
  int m_points{0};
};

struct Member
{
  bool operator < (const Member& other) const;
  double score() const;
  double score(int day) const;
  int points() const;
  int totalDelta() const;
  QString delta() const;
  QString m_name;
  QMap<int, DayResult> m_results;
};

class QJsonDocument;
class LeaderBoard
{
public:
  LeaderBoard(const QJsonDocument& document);
  QString toHtml();

private:
  QList<Member> m_members;
  int m_day_max{-1};
};

#endif // LEADERBOARD_H
