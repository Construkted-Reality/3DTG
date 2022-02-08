#ifndef __UTILS_H__
#define __UTILS_H__

#include <algorithm>
#include <regex>
#include <string>
#include <sstream>
#include <sys/stat.h>

#include <iostream>

// for windows mkdir
#if defined(_WIN32) || defined(_WIN64)
#include <direct.h>
#endif

#define PATH_MAX_LENGHT 260
#include <errno.h>


#if defined(_WIN32) || defined(_WIN64)
const char DIRECTORY_SYMBOL = '\\';
#else
const char DIRECTORY_SYMBOL = '/';
#endif

namespace utils {
  std::string getOsName();
  std::string posix(std::string path);
  std::string nonposix(std::string path);
  std::string normalize(std::string path);

  /**
   * Checks if a folder exists
   * @param foldername path to the folder to check.
   * @return true if the folder exists, false otherwise.
   */
  bool folder_exists(std::string foldername);

  /**
   * Portable wrapper for mkdir. Internally used by mkdir()
   * @param[in] path the full path of the directory to create.
   * @return zero on success, otherwise -1.
   */
  int makeDir(const char *path);

  /**
   * Recursive, portable wrapper for mkdir.
   * @param[in] path the full path of the directory to create.
   * @return zero on success, otherwise -1.
   */
  int mkdir(const char *path);

  int makePath(const char* path);

  std::string concatPath (const std::string& basepath, const std::string& path);
  std::string getDirectory (const std::string& path);
  std::string getFileName (const std::string& path);

  float min(float a, float b);
  float max(float a, float b);
}

#endif