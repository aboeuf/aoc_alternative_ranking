#include <QCoreApplication>
#include <manager.h>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  Manager m;
  QCoreApplication::connect(&m, SIGNAL(stop()), &a, SLOT(quit()), Qt::QueuedConnection);
  return a.exec();
}
