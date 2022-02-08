#include "./utils.h"


std::string utils::getOsName()
{
    #ifdef _WIN32
    return "Windows";
    #elif _WIN64
    return "Windows";
    #elif __APPLE__ || __MACH__
    return "Mac OSX";
    #elif __linux__
    return "Linux";
    #elif __FreeBSD__
    return "FreeBSD";
    #elif __unix || __unix__
    return "Unix";
    #else
    return "Other";
    #endif
}

std::string utils::posix(std::string path) {
  std::string result = path;
  std::replace(result.begin(), result.end(), '\\', '/');

  return result;
}

std::string utils::nonposix(std::string path) {
  std::string result = path;
  std::replace(result.begin(), result.end(), '/', '\\');

  return result;
}

std::string utils::normalize(std::string path) {
  #if defined(_WIN32) || defined(_WIN64)
    // ([^:]|^)(\\{2,})
    // return utils::nonposix(path);

    return std::regex_replace(utils::nonposix(path), std::regex("([^:]|^)(\\\\{2,})"), "$1\\");
  #else
    // return utils::posix(path);
    return std::regex_replace(utils::posix(path), std::regex("([^:]|^)(\\/{2,})"), "$1/");
  #endif
}

/**
 * Checks if a folder exists
 * @param foldername path to the folder to check.
 * @return true if the folder exists, false otherwise.
 */
bool utils::folder_exists(std::string foldername)
{
    struct stat st;
    stat(posix(foldername).c_str(), &st);
    return st.st_mode & S_IFDIR;
}

/**
 * Portable wrapper for mkdir. Internally used by mkdir()
 * @param[in] path the full path of the directory to create.
 * @return zero on success, otherwise -1.
 */
int utils::makeDir(const char *path)
{
#ifdef _WIN32
  return ::_mkdir(path);
#else
  #if _POSIX_C_SOURCE
    return ::mkdir(path, 0777);
  #else
    return ::mkdir(path, 0777); // not sure if this works on mac
  #endif
#endif
}

int utils::makePath(const char* path) {
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  const size_t len = strlen(path);
  char _path[PATH_MAX_LENGHT];
  char *p; 

  errno = 0;

  /* Copy string so its mutable */
  if (len > sizeof(_path)-1) {
    errno = ENAMETOOLONG;
    return -1; 
  }   
  strcpy(_path, path);

  /* Iterate the string */
  for (p = _path + 1; *p; p++) {
    if (*p == DIRECTORY_SYMBOL) {
      /* Temporarily truncate */
      *p = '\0';

      if (utils::mkdir(_path) != 0) {
        if (errno != EEXIST)
          return -1; 
      }

      *p = DIRECTORY_SYMBOL;
    }
  }   

  if (utils::mkdir(_path) != 0) {
    if (errno != EEXIST)
      return -1; 
  }   

  return 0;
}

/**
 * Recursive, portable wrapper for mkdir.
 * @param[in] path the full path of the directory to create.
 * @return zero on success, otherwise -1.
 */
int utils::mkdir(const char *path)
{
  std::string current_level = "";
  std::string level;
  std::stringstream ss(path);

  char delimiter = DIRECTORY_SYMBOL;

  // split path using slash as a separator
  while (std::getline(ss, level, delimiter))
  {
    current_level += level; // append folder to the current level

    // create current level
    // if (!folder_exists(current_level) && makeDir(current_level.c_str()) != 0)
    //   return -1;

    if (!folder_exists(current_level))
    {
      int code = makeDir(current_level.c_str());

      if (code != 0) {
        std::cout << "Cannot create: " << current_level.c_str() <<", code: " << code << std::endl;
        return -1;
      }
    }

    current_level += delimiter; // don't forget to append a slash
  }

  return 0;
}


std::string utils::concatPath (const std::string& basepath, const std::string& path) {
  std::string baseCopy = basepath;
  std::string pathCopy = path;

  if (baseCopy.back() == DIRECTORY_SYMBOL) {
    baseCopy.pop_back();
  }

  if (pathCopy.front() == DIRECTORY_SYMBOL) {
    pathCopy.substr(1);
  }

  return baseCopy + DIRECTORY_SYMBOL + pathCopy;
}

std::string utils::getDirectory (const std::string& path)
{
  size_t found = path.find_last_of(DIRECTORY_SYMBOL);
  if (found == std::string::npos) {
    return utils::normalize("./");
  }
  
  return(path.substr(0, found));
}

std::string utils::getFileName (const std::string& path)
{
    size_t foundFullName = path.find_last_of(DIRECTORY_SYMBOL);
    std::string fullName = path.substr(foundFullName + 1, path.size());

    size_t foundWithoutExt = fullName.find_last_of(".");
    return(fullName.substr(0, foundWithoutExt));
}

float utils::min(float a, float b) {
    if (a < b) {
        return a;
    }

    return b;
};
float utils::max(float a, float b) {
    if (a < b) {
        return b;
    }

    return a;
};