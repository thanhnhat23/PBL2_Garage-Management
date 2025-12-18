QT += core gui widgets svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Comment this line to see debug output in console
# CONFIG += windows

# Định nghĩa ứng dụng
TARGET = GarageManagement
TEMPLATE = app

# Include paths
INCLUDEPATH += Class \
               Function \
               UI/windows \
               UI/auth \
               UI/admin \
               UI/user \
               UI/dialogs \
               UI/common

# Định nghĩa các file nguồn
SOURCES += \
    main_qt.cpp \
    Function/Auth.cpp \
    Function/Brand.cpp \
    Function/Bus.cpp \
    Function/ConsoleMenu.cpp \
    Function/Driver.cpp \
    Function/Route.cpp \
    Function/Seat.cpp \
    Function/Table.cpp \
    Function/Ticket.cpp \
    Function/Trip.cpp \
    Function/Ultil.cpp \
    Function/User.cpp \
    Function/FareCalculator.cpp \
    UI/windows/MainWindow.cpp \
    UI/auth/LoginWindowNew.cpp \
    UI/admin/AdminWindow.cpp \
    UI/dialogs/CRUDDialogs.cpp \
    UI/user/UserWindow.cpp

# Định nghĩa các file header
HEADERS += \
    Class/Auth.h \
    Class/Brand.h \
    Class/Bus.h \
    Class/ConsoleMenu.h \
    Class/Driver.h \
    Class/Role.h \
    Class/Route.h \
    Class/Seat.h \
    Class/Table.h \
    Class/Ticket.h \
    Class/Trip.h \
    Class/Ultil.h \
    Class/User.h \
    Class/FareCalculator.h \
    UI/windows/MainWindow.h \
    UI/auth/LoginWindowNew.h \
    UI/admin/AdminWindow.h \
    UI/dialogs/CRUDDialogs.h \
    UI/user/UserWindow.h

FORMS += \
    UI/windows/MainWindow.ui \
    UI/admin/AdminWindow.ui \
    UI/user/UserWindow.ui

# Translations
TRANSLATIONS += PBL2_vi_VN.ts

# Qt resources (icons)
RESOURCES += resources.qrc

# Default rules for deployment
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

# Thư mục Data
DISTFILES += \
    Data/*.txt
