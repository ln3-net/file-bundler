/**
 * @file FileBundler.cpp
 * @date 25/03/26
 * @brief ファイルの説明
 * @details ファイルの詳細
 * @author saku shirakura (saku@sakushira.com)
 */

#include "FileBundler.h"

#include <algorithm>
#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>
#include <iterator>
#include <regex>
#include <unordered_map>

#include <net_ln3/cpp_lib/PrintHelper.h>

#include "constants.h"

const std::regex sign_replace_pattern("[^A-Z0-9_]");
const std::regex space_replace_pattern(" ");
const std::regex line_separator_pattern(R"((\r\n?)|\n)");

namespace fs = std::filesystem;
using ph = net_ln3::cpp_lib::PrintHelper;

bool confirmPrompt(const std::string& message_, const std::string& yes_message_, const std::string& no_message_,
                   const bool yes_)
{
    if (yes_)
        return true;
    while (true) {
        std::cout << message_ << std::flush;
        std::string user_input;
        std::cin >> user_input;
        std::ranges::transform(user_input, user_input.begin(), tolower);
        if (user_input == "y" || user_input == "yes") {
            std::cout << yes_message_ << std::endl;
            return true;
        }
        if (user_input == "n" || user_input == "no") {
            std::cout << no_message_ << std::endl;
            return false;
        }
    }
}

std::string convertFilePathToConstantName(const fs::path& file_path)
{
    std::string filename = file_path.stem().generic_string();
    std::string extension = file_path.extension().generic_string();
    std::string result = std::format("{}_{}", filename, extension);
    std::ranges::transform(result, result.begin(), toupper);
    result = std::regex_replace(result, space_replace_pattern, "_");
    result = std::regex_replace(result, sign_replace_pattern, "");
    return result;
}

std::string stripLn(const std::string& str) { return std::regex_replace(str, line_separator_pattern, ""); }

FileBundler::FileBundler(std::string input_dir_, std::string output_dir_, std::string filelist_path_,
                         const int option_)
    : _input_dir(std::move(input_dir_)), _output_dir(std::move(output_dir_)),
      _filelist_path(std::move(filelist_path_)), _header_only(option_ & Options::HEADER_ONLY),
      _declare_only(option_ & Options::DECLARE_ONLY),
      _all_yes(option_ & Options::ALL_YES)
{
}

int FileBundler::bundle() const
{
    // バンドル対象ファイルの指定モード
    int bundle_target_mode = 0;
    // 有効なパスかを確認し、それが有効なパスであればモードに追加する。
    if (fs::is_regular_file(_filelist_path))
        bundle_target_mode |= 0b01; // filelist id: 1
    if (fs::is_directory(_input_dir))
        bundle_target_mode |= 0b10; // input_dir id: 2
    if (bundle_target_mode == 0) {
        std::cout << ph::Color("エラー", ERROR_COLOR) << ": バンドル対象が指定されていません。" << std::endl;
        return 1;
    }
    // ディレクトリが作成できず、ディレクトリが存在しない場合
    if (!fs::create_directories(_output_dir) && !fs::is_directory(_output_dir)) {
        std::cout << ph::Color("エラー", ERROR_COLOR) << ": 指定されたパスはディレクトリでないか作成に失敗しました。" << std::endl;
        return 2;
    }

    //
    // バンドル対象のファイルを登録する
    //

    // ファイルから登録
    std::unordered_map<std::string, fs::path> files;
    if (bundle_target_mode & 0b01) {
        if (std::ifstream ifs(_filelist_path); ifs) {
            while (!ifs.eof()) {
                std::string line;
                std::getline(ifs, line);
                line = stripLn(line);
                const fs::path file_path(line);
                if (line.empty()) continue;
                if (!is_regular_file(file_path)) {
                    std::cout << ph::Color("警告", WARN_COLOR) << ": \"" <<
                        file_path.generic_string() <<
                        "\"はファイルではないか存在しないため無視されます。" << std::endl;
                    continue;
                }
                std::string filename = convertFilePathToConstantName(file_path);
                if (files.contains(filename)) {
                    std::cout << ph::Color("警告", WARN_COLOR) << ": \"" <<
                        file_path.generic_string() << "\"は同じファイル名のファイルがすでに存在しているため無視されます。" << std::endl;
                    continue;
                }
                files.try_emplace(filename, file_path);
            }
        }
        else { std::cout << ph::Color("エラー", ERROR_COLOR) << ": ファイルリストの読み込みに失敗しました。" << std::endl; }
    }
    // ディレクトリから登録
    if (bundle_target_mode & 0b10) {
        for (const fs::directory_iterator it(_input_dir); const auto& i : it) {
            if (!i.is_regular_file()) continue;
            std::string filename = convertFilePathToConstantName(i.path());
            if (files.contains(filename)) {
                std::cout << ph::Color("警告", WARN_COLOR) << ": \"" <<
                    i.path().generic_string() << "\"は同じファイル名のファイルがすでに存在しているため無視されます。" << std::endl;
                continue;
            }
            files.try_emplace(filename, i.path());
        }
    }

    //
    // ヘッダファイル・ソースファイルに書き込む。
    //

    fs::path header_path(_output_dir);
    header_path /= "resource.h";
    fs::path source_path(_output_dir);
    source_path /= "resource.c";
    // ファイルの存在確認・上書き確認を行う。
    if (exists(header_path)) {
        if (is_regular_file(header_path)) {
            if (!confirmPrompt("resource.hは既に存在しています。上書きしますか？(Y/N)",
                               "ファイルを上書きします。",
                               "コマンドをキャンセルしました。", _all_yes))
                return 0;
        }
        else {
            std::cout << ph::Color("エラー", ERROR_COLOR) << ": resource.hはすでに存在していますがファイルではありません。" << std::endl;
            return 3;
        }
    }
    if (!(_header_only || _declare_only) && exists(source_path)) {
        if (is_regular_file(header_path)) {
            if (!confirmPrompt("resource.cは既に存在しています。上書きしますか？(Y/N)",
                               "ファイルを上書きします。",
                               "コマンドをキャンセルしました。", _all_yes))
                return 0;
        }
        else {
            std::cout << ph::Color("エラー", ERROR_COLOR) << ": resource.cはすでに存在していますがファイルではありません。" << std::endl;
            return 4;
        }
    }

    //
    // 書き込み対象ファイルのストリームを開き、コメント・includeディレクティブなどを書き込む。
    //

    std::ofstream header, source;
    if (!(_header_only || _declare_only)) {
        source.open(source_path);
        if (!source) {
            std::cout << ph::Color("エラー", ERROR_COLOR) << ": 出力先ファイルが開けませんでした。" << std::endl;
            return 5;
        }
        source << "// This file is auto generated. by net.ln3.file-bundler\n\n\n"
            << "#include \"resource.h\"\n\n";
    }
    header.open(header_path);
    if (!header) {
        std::cout << ph::Color("エラー", ERROR_COLOR) << ": 出力先ファイルが開けませんでした。" << std::endl;
        return 6;
    }
    header << "// This file is auto generated. by net.ln3.file-bundler\n\n\n";
    header << "#ifndef RESOURCE_H\n#define RESOURCE_H\n\n\n" << std::flush;

    //
    // モードに合わせてヘッダファイル・ソースファイルにファイルの内容や定数宣言を書き込む。
    //

    for (const auto& [filename, path] : files) {
        //
        // ヘッダファイルの書き込み
        //

        header << std::format("// {}\n", path.filename().generic_string());
        if (!_header_only) {
            header
                << std::format("extern const unsigned long long SIZE_{};\n", filename)
                << std::format("extern const char F_{}[];\n\n\n", filename);
        }
        if (_declare_only) continue;

        //
        // ファイルサイズを取得
        //

        size_t input_size;
        try { input_size = fs::file_size(path); }
        catch (const std::filesystem::filesystem_error& e) {
            std::cout << ph::Color("エラー", ERROR_COLOR) << ": ファイルサイズが取得できませんでした。" << std::endl;
            std::cerr << e.what() << std::endl;
            // TODO: 失敗時の処理を関数として切り出す。（ソースファイルとヘッダの書き込み時）
            // ERROR HANDLER - Remove files.
            header.close();
            fs::remove(header_path);
            if (!_header_only) {
                source.close();
                fs::remove(source_path);
            }
            // END ERROR HANDLER
            return 7;
        }

        //
        // ソースファイルまたはヘッダファイルに定義を書き込む。
        //

        std::string size_declare = std::format("const unsigned long long SIZE_{} = {};\n", filename, input_size);
        std::string file_declare = std::format("const char F_{}[] = {{", filename);
        if (_header_only) { header << size_declare << file_declare; }
        else { source << size_declare << file_declare; }

        header.flush();
        if (!_header_only) { source.flush(); }

        //
        // バンドル対象のファイルからデータをソースファイルまたはヘッダファイルに書き込む。
        //

        // ファイルを開く
        std::ifstream input(path);
        if (!input) {
            std::cout << ph::Color("エラー", ERROR_COLOR) << ": ファイルを開けませんでした。" << std::endl;
            // TODO: 失敗時の処理を関数として切り出す。（ソースファイルとヘッダの書き込み時）
            // ERROR HANDLER - Remove files.
            header.close();
            fs::remove(header_path);
            if (!_header_only) {
                source.close();
                fs::remove(source_path);
            }
            // END ERROR HANDLER
            return 8;
        }

        std::ofstream& output = _header_only ? header : source;

        do {
            constexpr size_t input_buff_size = 10000;
            char buff[input_buff_size]{};
            // 読み込み文字数を正規化
            const size_t read_char_count = input_size > input_buff_size ? input_buff_size : input_size;
            input.read(buff, input_buff_size);
            input_size -= input_buff_size;
            std::vector buff_vec(buff, buff + read_char_count);
            // データを書き込む。
            if (input.eof()) {
                std::copy(buff_vec.begin(), buff_vec.end() - 1, std::ostream_iterator<int>(output, ", "));
                output << static_cast<int>(buff_vec.back()) << "};\n\n\n";
            }
            else {
                std::ranges::copy(buff_vec, std::ostream_iterator<int>(output, ", "));
            }
            output.flush();
        } while (!input.eof());
    }
    header << "#endif // RESOURCE_H\n";
    return 0;
}
