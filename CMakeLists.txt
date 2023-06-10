# Configure with [fxsdk build-fx] or [fxsdk build-cg], which provide the
# toolchain file and module path of the fxSDK

cmake_minimum_required(VERSION 3.15)
project(MyAddin)

set(FXPFX, /root/.local)
set(FXSDK_PLATFORM_LONG fxCG50  )
set(CMAKE_MODULE_PATH /root/.local/lib/cmake/fxsdk)
set(CMAKE_TOOLCHAIN_FILE  /root/.local/lib/cmake/fxsdk/FXCG50.cmake)
set(FXSDK_CMAKE_MODULE_PATH /root/.local/lib/cmake/fxsdk)


include(GenerateG1A)
include(GenerateG3A)
include(Fxconv)
find_package(Gint 2.9 REQUIRED)


set(SOURCES
  src/main.c
  # ...
)
# Shared assets, fx-9860G-only assets and fx-CG-50-only assets
set(ASSETS
  assets/pieces.png
  # ...
)
set(ASSETS_fx
  # ...
)
set(ASSETS_cg
  assets-cg/example.png
  # ...
)

fxconv_declare_assets(${ASSETS} ${ASSETS_fx} ${ASSETS_cg} WITH_METADATA)

add_executable(myaddin ${SOURCES} ${ASSETS} ${ASSETS_${FXSDK_PLATFORM}})
target_compile_options(myaddin PRIVATE -Wall -Wextra -Os)
target_link_libraries(myaddin Gint::Gint)

if("${FXSDK_PLATFORM_LONG}" STREQUAL fxCG50)
  generate_g3a(TARGET myaddin OUTPUT "chess.g3a"
    NAME "chess" ICONS assets-cg/icon-uns.png assets-cg/icon-sel.png)
endif()
