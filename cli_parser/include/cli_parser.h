#ifndef cli_parser_h__
#define cli_parser_h__

#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <vector>
#include <functional>
#include <sstream>
#include <memory>

namespace cliap {
    class Arg {
    public:
        Arg() = default;
        Arg(std::string name);
        Arg& short_name(std::string short_name);
        Arg& long_name(std::string long_name);
        Arg& default_value(std::string default_value);
        Arg& description(std::string description);
        Arg& value(std::string value);
        Arg& required();
        Arg& flag();

        void set_parsed(bool is_parsed) {
            is_parsed_ = is_parsed;
        }

        std::string short_name() const { return short_name_; }
        std::string long_name() const { return long_name_; }
        std::string default_value() const { return default_value_; }
        std::string description() const { return description_; }
        std::string value() const { return value_; }
        bool is_required() const { return is_required_; }
        bool is_flag() const { return is_flag_; }
        bool is_parsed() const { return is_parsed_; }

        template<typename T>
        T get_value_as() const {
            if (value_.empty())
                return {};

            T result{};
            std::stringstream ss{value_};
            ss >> result;
            return result;
        }

        std::string get_value_as_str() const { return value_; }

    private:
        std::string short_name_;
        std::string long_name_;
        std::string default_value_;
        std::string description_;
        std::string value_;
        bool is_required_{false};
        bool is_flag_{false};
        bool is_parsed_{false};
    };

    class ArgParser {
        using ArgPtr = std::shared_ptr<cliap::Arg>;
    public:
        ArgParser& add_parameter(cliap::Arg parm);

        std::optional<std::string> parse(const std::vector<std::string>& args);

        std::optional<std::string> parse(int argc, char* argv[]);

        void add_usage_string(std::string usage_string);

        void print_help();

        const cliap::Arg& arg(const std::string& arg_name) const;

        std::size_t parameters_count() const { return all_params().size(); }

        void reset();

        const std::vector<ArgPtr> all_params() const;

    private:
        void print_usage_examples() const;

        void adjust_fmt_max_field_lengths(const ArgPtr& p);

        std::size_t required_args_count() const {
            const auto params = all_params();
            return static_cast<std::size_t>(std::count_if(
                std::cbegin(params),
                std::cend(params),
                [](const auto& ptr) mutable { return ptr->is_required(); }
            ));
        }

        std::optional<std::string> check_required_args() const;

        std::unordered_map<std::string, ArgPtr> params_map_;
        std::vector<std::string> usage_examples_;

        std::streamsize max_long_param_name_length_{};
        std::streamsize max_default_param_value_length_{};

        cliap::Arg empty_arg_{};
    };
}

#endif // cli_parser_h__