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


class FileBundler
{
public:
    FileBundler(std::string input_dir_, std::string output_dir_, std::string filelist_path_, bool header_only_, bool declare_only_);

    [[nodiscard]] int bundle(bool all_yes_) const;

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
};


#endif //FILEBUNDLER_H
