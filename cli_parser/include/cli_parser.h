#ifndef cli_parser_h__

#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <vector>
#include <functional>
#include <sstream>
#include <memory>

namespace cli {
    class Param {
    public:
        Param() = default;
        Param(std::string name);
        Param& short_name(std::string short_name);
        Param& long_name(std::string long_name);
        Param& default_value(std::string default_value);
        Param& description(std::string description);
        Param& value(std::string value);
        Param& required();
        Param& flag();

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

    class ParamParser {
        using ParamPtr = std::shared_ptr<cli::Param>;
    public:
        ParamParser& add_parameter(cli::Param parm);

        std::optional<std::string> parse(const std::vector<std::string>& args);

        std::optional<std::string> parse(int argc, char* argv[]);

        void add_usage_string(std::string usage_string);

        void print_help();

        const cli::Param& arg(const std::string& arg_name) const;

        std::size_t parameters_count() const { return all_params().size(); }

        void reset();

        const std::vector<ParamPtr> all_params() const;

    private:
        void print_usage_examples() const;

        void adust_fmt_max_field_lengths(const ParamPtr& p);

        std::size_t required_args_count() const {
            const auto params = all_params();
            return static_cast<std::size_t>(std::count_if(
                std::cbegin(params),
                std::cend(params),
                [](const auto& ptr) mutable { return ptr->is_required(); }
            ));
        }

        std::optional<std::string> check_required_args() const;

        std::unordered_map<std::string, ParamPtr> params_map_;
        std::vector<std::string> usage_examples_;

        std::size_t max_long_param_name_length_{};
        std::size_t max_default_param_value_length_{};

        cli::Param empty_arg_{};
    };
}

#endif // cli_parser_h__