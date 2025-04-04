#include <iostream>
#include <net_ln3/cpp_lib/ArgumentParser.h>
#include <net_ln3/cpp_lib/multi_platform_util.h>
#include <net_ln3/cpp_lib/PrintHelper.h>
#include <filesystem>

#include "constants.h"
#include "FileBundler.h"
#include "resource.h"

using ph = net_ln3::cpp_lib::PrintHelper;
namespace fs = std::filesystem;

const std::string app_name = "file-bundler";
const std::string app_id = "net.ln3.file-bundler";
const std::string version = "v0.0.1-alpha";
const std::string copyright = "Copyright (c) 2025 Saku Shirakura <saku@sakushira.com>";
const std::string license = "MIT";

void printHelp()
{
    std::cout << "file-bundler\n"
        "\tファイルをC言語で使える形にバンドルします。\n"
        "\t生成された定数はF_{FILE_NAME}_{EXTENSION}の形で命名されます。\n"
        "\tただし、半角英数字およびアンダーバー以外の文字は取り除かれます。\n"
        "\t半角スペースについては、アンダーバーに置換されます。\n"
        "\t配列のサイズの定数はSIZE_{FILE_NAME}_{EXTENSION}の形で命名されます。\n"
        "\tここで生成される定数名が競合する場合は、警告とともに無視されます。\n"
        "\tこのソフトウェアは、C言語標準機能となるembedディレクティブまでの繋ぎです。\n"
        "\n\t--output-dir, -o" << ph::Color("*(必須)", ERROR_COLOR) << ":\n"
        "\t\t出力先ディレクトリを指定します。\n"
        "\t\t指定したディレクトリには、resources.h及びresources.cが生成されます。\n"
        "\n\t--input-dir, -i" << ph::Color("*(1つ必須, 併用可能)", "#00a381") << ":\n"
        "\t\t入力ディレクトリを指定します。\n"
        "\t\t指定したディレクトリに存在するファイルが全てバンドルされます。\n"
        "\t\t== 制約 ==\n"
        "\t\t・サブディレクトリ内のファイルは処理されません。\n"
        "\t\t・output-dirと同じディレクトリは指定できません。\n"
        "\t\t・target-filelist引数により指定されているファイルと\n"
        "\t\t\t同一の名称(拡張子を含む)を持つファイルについては処理されません。\n"
        "\n\t--target-filelist, -t" << ph::Color("*(1つ必須, 併用可能)", "#00a381") << ":\n"
        "\t\tファイルリストを使用します。\n"
        "\t\tこの引数により指定するファイルリストファイルには１行につき１ファイルのパスを記述します。\n"
        "\t\tこの引数は、input-dirの代わりに指定することができ、両方を指定した場合はそれぞれをバンドルします。\n"
        "\n\t--header-only:\n"
        "\t\tヘッダファイルに直接バンドルします。\n"
        "\n\t--declare-only:\n"
        "\t\t定数宣言のみのヘッダファイルを生成します。\n"
        "\t\t""header-onlyと同時に指定された場合は、header-onlyが優先されます。\n"
        "\n\t--help, -?:\n"
        "\t\tヘルプテキストを表示します。\n"
        "\n\t--version, -v:\n"
        "\t\tバージョン情報を表示します。\n"
        "\n\t--show-license:\n"
        "\t\tライセンス情報を表示します。"
        "\n\t--yes, -y:\n"
        "\t\t上書き保存などをスキップしyesを渡します。" << std::endl;
}

void printVersion()
{
    std::cout << std::format("{} {}\n{}\n--- APP INFO ---\n""app_id: {}\n""license: {}",
                             app_name, version, copyright, app_id, license);
}

void printLicense()
{
    const std::string license_body(F_LICENSE_, SIZE_LICENSE_);
    std::cout << license_body << std::endl;
}

int main(const int argc_, char* argv_[])
{
    net_ln3::cpp_lib::multi_platform::CodePageGuard cp;
    net_ln3::cpp_lib::multi_platform::EnableAnsiEscapeSequence::enable();
    using ap = net_ln3::cpp_lib::ArgumentParser;
    ap argument_parser{
        ap::OptionNames({
            {"output-dir", ap::OptionType::STRING},
            {"input-dir", ap::OptionType::STRING},
            {"target-filelist", ap::OptionType::STRING},
            {"header-only", ap::OptionType::BOOLEAN},
            {"declare-only", ap::OptionType::BOOLEAN},
            {"help", ap::OptionType::BOOLEAN},
            {"version", ap::OptionType::BOOLEAN},
            {"show-license", ap::OptionType::BOOLEAN},
            {"yes", ap::OptionType::BOOLEAN}
        }),
        ap::OptionAlias({
            {"?", "help"},
            {"v", "version"},
            {"i", "input-dir"},
            {"o", "output-dir"},
            {"t", "target-filelist"},
            {"y", "yes"}
        })
    };
    argument_parser.parse(argc_, argv_);
    if (argument_parser.getOption("help"))
        printHelp(); // NOLINT(*-branch-clone)
    else if (argument_parser.getOption("version")) { printVersion(); }
    else if (argument_parser.getOption("show-license")) { printLicense(); }
    // 処理に必要なファイル・ディレクトリのパスを指定する引数が存在しないなら、ヘルプを表示する。
    else if (!(
            argument_parser.isExistOption("input-dir") ||
            argument_parser.isExistOption("target-filelist") ||
            argument_parser.isExistOption("output-dir"))
    ) { printHelp(); }
    else {
        int invalid_args{};
        int missing_args{};
        // id: 1
        if (!(argument_parser.isExistOption("input-dir") || argument_parser.isExistOption("target-filelist")))
            missing_args |= 0b0001;
        else if (argument_parser.isExistOption("input-dir") && !fs::is_directory(
            argument_parser.getOption("input-dir").getString()))
            invalid_args |= 0b0001;
        // id: 2
        if (!argument_parser.isExistOption("output-dir"))
            missing_args |= 0b0010;
        else if (
            !fs::create_directories(argument_parser.getOption("output-dir").getString()) &&
            !fs::is_directory(argument_parser.getOption("output-dir").getString())
        )
            invalid_args |= 0b0010;
        // id: 4
        if (argument_parser.isExistOption("target-filelist") &&
            !fs::is_regular_file(argument_parser.getOption("target-filelist").getString())
        ) { invalid_args |= 0b100; }
        // 同一ディレクトリ制約のエラーを表示する。
        if (missing_args == 0 && argument_parser.getOption("input-dir").getString() == argument_parser.
            getOption("output-dir").getString()) {
            std::cout << ph::Color("エラー", ERROR_COLOR) << ": 入力ディレクトリと出力先ディレクトリは異なるディレクトリを指定してください。" << std::endl;
        }
        // 引数エラーを表示する。
        if (missing_args > 0) {
            std::cout << ph::Color("エラー", ERROR_COLOR);
            std::cout << ": 必須の引数が指定されていません。\n不足している引数:\n";
            if (missing_args & 0b0001)
                std::cout << "・input-dir または target-filelist\n";
            if (missing_args & 0b0010)
                std::cout << "・output-dir\n";
            std::cout << "\n詳細な用法はヘルプを参照してください。 file-bundler --help" << std::endl;
            return 1;
        }
        if (invalid_args > 0) {
            std::cout << ph::Color("エラー", ERROR_COLOR);
            std::cout << ": 引数に指定された値が不正です。\n"
                "指定されたパスは有効なパスではありません。\n無効な引数:\n";
            if (invalid_args & 0b0001)
                std::cout << "・input-dir\n";
            if (invalid_args & 0b0010)
                std::cout << "・output-dir (ディレクトリを作成できませんでした。)\n";
            if (invalid_args & 0b0100)
                std::cout << "・target-filelist\n";
            std::cout << "\n詳細な用法はヘルプを参照してください。 file-bundler --help" << std::endl;
            return 1;
        }
        if (argument_parser.getOption("declare-only") && argument_parser.getOption("header-only")) {
            std::cout << ph::Color("警告", WARN_COLOR) <<
                ": 競合するフラグ「header-only」「declare-only」の両方が有効になっています。\n"
                "\t「declare-only」フラグを無視します。" << std::endl;
        }
        const FileBundler bundler{
            argument_parser.getOption("input-dir").getString(),
            argument_parser.getOption("output-dir").getString(),
            argument_parser.getOption("target-filelist").getString(),
            argument_parser.getOption("header-only").getBoolean(),
            argument_parser.getOption("declare-only").getBoolean()
        };
        return bundler.bundle(argument_parser.getOption("yes").getBoolean());
    }
    return 0;
}
