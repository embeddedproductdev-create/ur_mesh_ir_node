# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "C:/Espressif/v5.1.2/esp-idf/components/bootloader/subproject"
  "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader"
  "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader-prefix"
  "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader-prefix/tmp"
  "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader-prefix/src/bootloader-stamp"
  "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader-prefix/src"
  "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "D:/Kulasekaran/MyProjects/IR/NewIR/hello_world/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
