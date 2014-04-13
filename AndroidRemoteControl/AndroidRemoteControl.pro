QT += qml quick network widgets
CONFIG += c++11

SOURCES += main.cpp \
    RemoteInterface.cpp \
    RemoteConnection.cpp \
    RemoteImage.cpp \
    RemoteCommand.cpp \
    Client.cpp \
    CategoryModel.cpp \
    MyImage.cpp \
    SearchInfo.cpp \
    Settings.cpp \
    ImageStore.cpp \
    ScreenInfo.cpp \
    Action.cpp \
    History.cpp

OTHER_FILES += \
    qml/main.qml \
    qml/Icon.qml \
    qml/OverviewPage.qml \
    qml/CategoryItemsPage.qml \
    qml/ThumbnailsPage.qml \
    qml/ImageViewer.qml \
    qml/ScrollBar.qml

RESOURCES += \
    resources.qrc

HEADERS += \
    RemoteInterface.h \
    RemoteConnection.h \
    RemoteImage.h \
    RemoteCommand.h \
    Client.h \
    CategoryModel.h \
    MyImage.h \
    SearchInfo.h \
    Settings.h \
    ImageStore.h \
    ScreenInfo.h \
    Types.h \
    Action.h \
    History.h
