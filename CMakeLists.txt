cmake_minimum_required(VERSION 3.24)

project(IdiomindQt
    VERSION 0.1.0
    DESCRIPTION "Idiomind reimplementation in Qt6/QML (F0 scaffold)"
    LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Qt 6.5+ es el objetivo declarado; el sistema actual tiene Qt 6.8.2 ---
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Gui Qml Quick Sql QuickControls2 Multimedia)
find_package(ZLIB REQUIRED)

qt_standard_project_setup(REQUIRES 6.8)

# ---- core library (dominio puro; F0: AppPaths+Settings; F2.1: topic/learning; F4: learning engine) --
add_library(idiomind_core STATIC
    src/core/config/AppPaths.cpp
    src/core/config/Settings.cpp
    src/core/topic/Item.cpp
    src/core/topic/FlatItemCodec.cpp
    src/core/topic/Topic.cpp
    src/core/topic/TopicList.cpp
    src/core/topic/TopicInfo.cpp
    src/core/learning/ReviewState.cpp
    src/core/learning/Review.cpp
    src/core/learning/ReviewCalculator.cpp
    src/core/learning/PracticeLogs.cpp
    src/core/learning/IndexBuilder.cpp
)
target_include_directories(idiomind_core PUBLIC src)
target_link_libraries(idiomind_core PUBLIC Qt6::Core Qt6::Sql)

# ---- storage library (persistencia SQLite; F2.2+F4) ----------------------------
add_library(idiomind_storage STATIC
    src/storage/dbs/SqliteDatabase.cpp
    src/storage/config/ConfigRepository.cpp
    src/storage/topic/TopicRepository.cpp
    src/storage/topic/TopicDataRepository.cpp
    src/storage/topic/ReviewRepository.cpp
    src/storage/interchange/IdmndCodec.cpp
    src/storage/interchange/LegacyImporter.cpp
    src/storage/interchange/TopicExporter.cpp
    src/storage/SharedRepository.cpp
    src/storage/LearningEngine.cpp
    src/storage/PracticeController.cpp
    src/services/topic/TopicRepairService.cpp
)
target_include_directories(idiomind_storage PUBLIC src)
target_link_libraries(idiomind_storage PUBLIC idiomind_core Qt6::Core Qt6::Sql ZLIB::ZLIB)

# ---- services library (F5: traducción, TTS, grammar, images) -------------------
add_library(idiomind_services STATIC
    src/services/impl/TranslationService.cpp
    src/services/impl/TtsPipelineService.cpp
    src/services/impl/GrammarColorizer.cpp
    src/services/impl/ImageProcessor.cpp
    src/services/impl/AddNoteFlow.cpp
    src/services/impl/GoogleTranslateProvider.cpp
    src/services/impl/TextProcessor.cpp
    src/services/impl/PlaybackController.cpp
)
target_include_directories(idiomind_services PUBLIC src)
target_link_libraries(idiomind_services PUBLIC idiomind_storage Qt6::Core Qt6::Gui Qt6::Network Qt6::Multimedia)

# ---- ui library (F6-B.1: models + controllers) --------------------------------
add_library(idiomind_ui STATIC
    src/ui/models/TopicListModel.cpp
    src/ui/models/IndexListModel.cpp
    src/ui/models/TaskListModel.cpp
    src/ui/controllers/TopicProxy.cpp
)
target_include_directories(idiomind_ui PUBLIC src)
target_link_libraries(idiomind_ui PUBLIC idiomind_services Qt6::Core)

# ---- application executable --------------------------------------------------
qt_add_executable(idiomindqt
    src/app/main.cpp
    src/app/Application.cpp
    resources/idiomind.qrc
)
target_include_directories(idiomindqt PRIVATE src)
target_link_libraries(idiomindqt PRIVATE idiomind_ui Qt6::Gui Qt6::Qml Qt6::Quick Qt6::Multimedia)

# --- Plataforma --------------------------------------------------------------

if(ANDROID)

    # Android: applicationId y nombre visible
    set_target_properties(idiomindqt PROPERTIES
        QT_ANDROID_PACKAGE_NAME "com.idiomind.app"
        QT_ANDROID_APP_NAME "Idiomind"
        QT_ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_SOURCE_DIR}/android"
    )

elseif(WIN32)

    # Windows
    set_target_properties(idiomindqt PROPERTIES
        WIN32_EXECUTABLE TRUE
        OUTPUT_NAME "Idiomind"
    )

endif()

qt_add_qml_module(idiomindqt
    URI Idiomind
    VERSION 1.0
    QML_FILES
        src/ui/qml/Main.qml
        src/ui/qml/components/NavigationStrips.qml
        src/ui/qml/components/ContextualFooter.qml
        src/ui/qml/components/LearningView.qml
        src/ui/qml/components/LearntView.qml
        src/ui/qml/components/NotesView.qml
        src/ui/qml/components/ManageView.qml
        src/ui/qml/components/TopicsModal.qml
        src/ui/qml/components/PlayDialog.qml
        src/ui/qml/components/PracticeDialog.qml
        src/ui/qml/components/ViewerDialog.qml
        src/ui/qml/components/AppDrawer.qml
)

# --- Resources (images) ---
qt_add_resources(idiomindqt "idiomind_images"
    PREFIX "/"
    BASE "resources"
    FILES
        resources/images/logo.png
        resources/images/states/img.0.png
        resources/images/states/img.1.png
        resources/images/states/img.2.png
        resources/images/states/img.3.png
        resources/images/states/img.4.png
        resources/images/states/img.5.png
        resources/images/states/img.6.png
        resources/images/states/img.7.png
        resources/images/states/img.8.png
        resources/images/states/img.9.png
        resources/images/states/img.10.png
        resources/images/states/img.13.png
        resources/images/practice/0.png
        resources/images/practice/1.png
        resources/images/practice/2.png
        resources/images/practice/3.png
        resources/images/practice/4.png
        resources/images/practice/5.png
        resources/images/practice/6.png
        resources/images/practice/7.png
        resources/images/practice/8.png
        resources/images/practice/9.png
        resources/images/practice/10.png
        resources/images/practice/11.png
        resources/images/practice/12.png
        resources/images/practice/13.png
        resources/images/practice/14.png
        resources/images/practice/15.png
        resources/images/practice/16.png
        resources/images/practice/17.png
        resources/images/practice/18.png
        resources/images/practice/19.png
        resources/images/practice/20.png
        resources/images/practice/21.png
)

# ---- tests --------------------------------------------------------------------
enable_testing()
add_subdirectory(tests)
