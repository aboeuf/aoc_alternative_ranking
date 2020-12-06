QT -= gui
QT += core network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++20

SOURCES += \
    leaderboard.cpp \
    main.cpp \
    manager.cpp

HEADERS += \
    jsonhelper.h \
    leaderboard.h \
    manager.h

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ../.gitignore \
    ../CMakeLists.txt \
    ../deploy.sh \
    ../index.php \
    ../styles.css
