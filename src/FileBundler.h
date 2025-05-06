/**
 * @file FileBundler.h
 * @date 25/03/26
 * @brief ファイルの説明
 * @details ファイルの詳細
 * @author saku shirakura (saku@sakushira.com)
 */

#ifndef FILEBUNDLER_H
#define FILEBUNDLER_H
#include <string>


class FileBundler {
public:
    struct Options {
        enum Option {
            HEADER_ONLY = 0x0001,
            DECLARE_ONLY = 0x0010,
            ALL_YES = 0x0100
        };
    };

    FileBundler(std::string input_dir_, std::string output_dir_, std::string filelist_path_, int option_);

    [[nodiscard]] int bundle() const;

    FileBundler() = delete;
    FileBundler(const FileBundler&) = delete;
    FileBundler(FileBundler&&) = delete;
    FileBundler& operator=(const FileBundler&) = delete;
    FileBundler& operator=(FileBundler&&) = delete;

private:
    std::string _input_dir;
    std::string _output_dir;
    std::string _filelist_path;
    bool _header_only;
    bool _declare_only;
    bool _all_yes;
};


#endif //FILEBUNDLER_H
