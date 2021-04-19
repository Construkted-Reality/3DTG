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
    return ::mkdir(path);
#else
    return ::mkdir(path, 0755); // not sure if this works on mac
#endif
#endif
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

    char delimiter = (getOsName() == "Windows") ? '\\' : '/';
    std::cout << "Delimiter: " << delimiter << std::endl;

    // split path using slash as a separator
    while (std::getline(ss, level, delimiter))
    {
        current_level += level; // append folder to the current level

        // create current level
        if (!folder_exists(current_level) && makeDir(current_level.c_str()) != 0)
            return -1;

        current_level += delimiter; // don't forget to append a slash
    }

    return 0;
}


std::string utils::concatPath (const std::string& basepath, const std::string& path) {
  if (getOsName() == "Windows") {
    return basepath + "\\" + path;
  }

  return basepath + "/" + path;
}

std::string utils::getDirectory (const std::string& path)
{
    size_t found = path.find_last_of("/\\");
    return(path.substr(0, found));
}