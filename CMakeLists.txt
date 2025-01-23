cmake_minimum_required(VERSION 3.29)
project(proje_test C)

set(CMAKE_C_STANDARD 11)

# Çıktı dosyasını önce tanımla
add_executable(proje_test main.c)

# Raylib'in bulunduğu yeri belirtin
target_include_directories(proje_test PRIVATE "C:/Users/User/vcpkg/installed/x64-windows/include")

# Raylib'i bulmak için
find_library(RAYLIB_LIBRARY raylib HINTS "C:/Users/User/vcpkg/installed/x64-windows/lib")
find_library(CURL_LIBRARY libcurl HINTS "C:/Users/User/vcpkg/installed/x64-windows/lib")

# Raylib ve GLFW DLL yollarını belirtin
set(RAYLIB_DLL_DIR "C:/Users/User/vcpkg/installed/x64-windows/bin")
add_custom_command(TARGET proje_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RAYLIB_DLL_DIR}/raylib.dll" $<TARGET_FILE_DIR:proje_test>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RAYLIB_DLL_DIR}/glfw3.dll" $<TARGET_FILE_DIR:proje_test>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RAYLIB_DLL_DIR}/libcurl.dll" $<TARGET_FILE_DIR:proje_test>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${RAYLIB_DLL_DIR}/zlib1.dll" $<TARGET_FILE_DIR:proje_test>)


# Raylib ve CURL'u projeye dahil et
target_link_libraries(proje_test PRIVATE ${RAYLIB_LIBRARY} ${CURL_LIBRARY})

