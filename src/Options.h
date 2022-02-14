#ifndef __OPTIONS_H__
#define __OPTIONS_H__

#include <cxxopts/cxxopts.hpp>

// #include <memory>
#include <string>
#include <map>

#include "types.h"

typedef std::map<std::string, bool> OptionList;

class Options {
  public:
    std::string input;
    std::string output;

    uint32_t limit;
    uint32_t grid;

    float32_t iso;

    bool dracoEnabled;
    int textureLevels;

    std::string format;
    std::string algorithm;

    static Options& GetInstance() {
      // Allocate with `new` in case Options is not trivially destructible.
      static Options* opts = new Options();
      return *opts;
    };

    bool valid(int argc, char** argv) {
      this->_options = cxxopts::Options("3dtg", "3d models compiler from various formats to tile format.");

      cxxopts::OptionAdder rootOptions = this->_options.add_options();

      rootOptions("i,input", "Input model path", cxxopts::value(this->input));
      rootOptions("o,output", "Output directory", cxxopts::value(this->output)->default_value("./exported"));
      rootOptions("l,limit", "Polygons per chunk limit", cxxopts::value(this->limit)->default_value("2048"));
      rootOptions("g,grid", "Grid resolution", cxxopts::value(this->grid)->default_value("64"));
      rootOptions("iso", "Iso level", cxxopts::value(this->iso)->default_value("1.0"));
      rootOptions("compress", "Enable draco compression", cxxopts::value(this->dracoEnabled));
      rootOptions("texlevels", "Count of texture LOD levels", cxxopts::value(this->textureLevels)->default_value("8"));
      // rootOptions("f,format", "Model format to export", cxxopts::value(this->format)->default_value("b3dm"));

      /** Algorithm option start */
      std::shared_ptr<cxxopts::Value> algorithmValue = cxxopts::value<std::string>();
      if (this->hasDefaultAlgoritm()) {
        algorithmValue->default_value(this->_defaultAlgorithm);
      }
      rootOptions("a,algorithm", "Algorithm to use to split", algorithmValue);
      /** Algorithm option end */

      rootOptions("algorithms", "List of available algorithms");


      /** Format option start */
      std::shared_ptr<cxxopts::Value> formatValue = cxxopts::value<std::string>();
      if (this->hasDefaultFormat()) {
        formatValue->default_value(this->_defaultFormat);
      }
      rootOptions("f,format", "Model format to export", formatValue);
      /** Format option end */

      rootOptions("formats", "List of available formats");

      rootOptions("h,help", "Help");


      this->_options.allow_unrecognised_options();
      this->_options.parse_positional({"input", "output"});

      cxxopts::ParseResult result;

      try {
        result = this->_options.parse(argc, argv);
      } catch (cxxopts::missing_argument_exception const &exeption) {
        std::cout << exeption.what() << std::endl;
        return false;
      }

      if (result.count("help")) {
        std::cout << this->_options.help() << std::endl;
        return false;
      }

      if (this->dracoEnabled) {
        std::cout << "Draco ENABLED" << std::endl;
      } else {
        std::cout << "Draco DISABLED" << std::endl;
      }

      if (result.count("algorithms")) {
        this->showAvailableAlgorithms();
        return false;
      }

      if (result.count("formats")) {
        this->showAvailableFormats();
        return false;
      }

      if (!result.count("input")) {
        std::cout << "Input isn't specified" << std::endl;
        return false;
      }

      if (!this->parseAlgorithm(result)) {
        return false;
      }

      if (!this->parseFormat(result)) {
        return false;
      }

      return true;
    };

    bool parseAlgorithm(cxxopts::ParseResult &result) {
      if (this->_availableAlgoritms.size() == 0) {
        std::cout << "There no available algorithms" << std::endl;
        return false;
      }

      if (!result.count("algorithm")) {
        if (!this->_defaultAlgorithm.empty()) {
          this->algorithm = this->_defaultAlgorithm;
        } else {
          std::cout << "There no default algorithm" << std::endl;
          return false;
        }
      } else {
        this->algorithm = result["algorithm"].as<std::string>();
      }

      if (this->_availableAlgoritms.count(this->algorithm) == 0) {
        std::cout << "Algorithm \"" << this->algorithm << "\" wasn't found." << std::endl;

        this->showAvailableAlgorithms();

        return false;
      }

      return true;
    };

    bool parseFormat(cxxopts::ParseResult &result) {
      if (this->_availableFormats.size() == 0) {
        std::cout << "There no available formats" << std::endl;
        return false;
      }

      if (!result.count("format")) {
        if (!this->_defaultFormat.empty()) {
          this->format = this->_defaultFormat;
        } else {
          std::cout << "There no default format" << std::endl;
          return false;
        }
      } else {
        this->format = result["format"].as<std::string>();
      }

      if (this->_availableFormats.count(this->format) == 0) {
        std::cout << "Format \"" << this->format << "\" wasn't found." << std::endl;

        this->showAvailableFormats();

        return false;
      }

      return true;
    };

    void addAlgorithm(std::string algorithm) {
      this->_availableAlgoritms[algorithm] = true;
    };

    void setDefaultAlgorithm(std::string algorithm) {
      this->_defaultAlgorithm = algorithm;
    };

    void addFormat(std::string format) {
      this->_availableFormats[format] = true;
    };

    void setDefaultFormat(std::string format) {
      this->_defaultFormat = format;
    };

  private:
    OptionList _availableAlgoritms;
    OptionList _availableFormats;

    std::string _defaultAlgorithm;
    std::string _defaultFormat;

    cxxopts::Options _options = cxxopts::Options("3dtg", "3d models compiler from various formats to tile format.");

    bool hasDefaultAlgoritm() {
      return !this->_defaultAlgorithm.empty();
    };

    bool hasDefaultFormat() {
      return !this->_defaultFormat.empty();
    };

    void showAvailableAlgorithms() {
      std::cout << "Available algoritms: ";

      this->printList(this->_availableAlgoritms);
    };

    void showAvailableFormats() {
      std::cout << "Available formats: ";

      this->printList(this->_availableFormats);
    };

    void printList(OptionList &list) {
      uint32_t index = 0;
      for (const auto& [key, value] : list) {
        std::cout << key.c_str();// << " has value " << value << std::endl;
        index++;

        if (index != list.size()) {
          std::cout << ", ";
        } else {
          std::cout << std::endl;
        }
      }
    };

    Options() = default;

    // Delete copy/move so extra instances can't be created/moved.
    Options(const Options&) = delete;
    Options& operator=(const Options&) = delete;
    Options(Options&&) = delete;
    Options& operator=(Options&&) = delete;
};

#endif // __OPTIONS_H__`