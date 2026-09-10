find_package(Qt6 6.5 REQUIRED COMPONENTS Test)
find_package(ZLIB REQUIRED)

function(idiomind_add_unit_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} PRIVATE idiomind_storage Qt6::Test ZLIB::ZLIB)
    target_compile_definitions(${name} PRIVATE
        IDIOMIND_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/idmnd/")
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

function(idiomind_add_ui_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} PRIVATE idiomind_ui Qt6::Test)
    target_compile_definitions(${name} PRIVATE
        IDIOMIND_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/idmnd/")
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

function(idiomind_add_service_test name)
    qt_add_executable(${name} ${name}.cpp)
    target_include_directories(${name} PRIVATE ${CMAKE_SOURCE_DIR}/src)
    target_link_libraries(${name} PRIVATE idiomind_services Qt6::Test)
    target_compile_definitions(${name} PRIVATE
        IDIOMIND_TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/fixtures/idmnd/")
    add_test(NAME ${name} COMMAND ${name})
    set_tests_properties(${name} PROPERTIES
        ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
endfunction()

idiomind_add_unit_test(tst_apppaths)
idiomind_add_unit_test(tst_flatitemcodec)
idiomind_add_unit_test(tst_reviewstate)
idiomind_add_unit_test(tst_reviewcalculator)
idiomind_add_unit_test(tst_topicinfo)
idiomind_add_unit_test(tst_topic)
idiomind_add_unit_test(tst_topiclist)
idiomind_add_unit_test(tst_configrepository)
idiomind_add_unit_test(tst_topicrepository)
idiomind_add_unit_test(tst_topicdatarepository)
idiomind_add_unit_test(tst_reviewrepository)
idiomind_add_unit_test(tst_topicrepair)
idiomind_add_unit_test(tst_idmndcodec)
idiomind_add_unit_test(tst_idmndroundtrip)
idiomind_add_unit_test(tst_learning)
idiomind_add_service_test(tst_services)
idiomind_add_ui_test(tst_topicproxy)
idiomind_add_service_test(tst_textprocessor)
