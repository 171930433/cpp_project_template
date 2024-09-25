include(FetchContent)

FetchContent_Declare(
    yalantinglibs
    GIT_REPOSITORY https://github.com/alibaba/yalantinglibs.git
    GIT_TAG main # optional ( default master / main )
    GIT_SHALLOW 1 # optional ( --depth=1 )
)

FetchContent_MakeAvailable(yalantinglibs)

add_library(${PROJECT_NAME}_3rd INTERFACE)

target_link_libraries(${PROJECT_NAME}_3rd INTERFACE
yalantinglibs::yalantinglibs
)

target_compile_features(${PROJECT_NAME}_3rd INTERFACE cxx_std_20)
