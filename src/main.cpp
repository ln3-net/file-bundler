#include <iostream>
#include <net_ln3/cpp_lib/ArgumentParser.h>
#include <net_ln3/cpp_lib/multi_platform_util.h>

int main(const int argc_, char* argv_[])
{
    net_ln3::cpp_lib::multi_platform::CodePageGuard cp;
    using ap = net_ln3::cpp_lib::ArgumentParser;
    ap argument_parser{
        ap::OptionNames({
            {"output-dir", ap::OptionType::STRING},
            {"input-dir", ap::OptionType::STRING},
            {"help", ap::OptionType::BOOLEAN}
        })
    };
    argument_parser.parse(argc_, argv_);
    if (argument_parser.getOption("help")) {
        std::cout << "file-bundler\n"
                     "\t--output-dir:\n"
                     "\t\t出力先ディレクトリを指定します。\n"
                     "\t\t指定したディレクトリには、resources.h及びresources.cが生成されます。\n"
                     "\t--input-dir:\n"
                     "\t\t入力ディレクトリを指定します。\n"
                     "\t\t指定したディレクトリに存在するファイルが全てバンドルされます。\n"
                     "\t\toutput-dirと同じディレクトリは指定できません。" << std::endl;
    }
    return 0;
}
