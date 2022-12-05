#ifndef cli_parser_h__

#include <iostream>
#include <unordered_map>
#include <algorithm>
#include <optional>
#include <iomanip>
#include <set>
#include <vector>
#include <exception>
#include <sstream>

namespace cli {
    namespace {
        std::string_view rtrim_copy(std::string_view str, std::string_view pattern)
        {
            if (const auto pos = str.rfind(pattern.data()); pos != std::string_view::npos)
                return std::string_view{str.data(), pos};

            return str;
        }

        // parse parameters like --listen-port=1010
        bool parse_key_arg(std::string_view in_str, std::string& key, std::string& value) {
            key.clear();
            value.clear();

            if (const auto equal_pos = in_str.find('='); equal_pos != std::string::npos) {
                key = rtrim_copy(in_str.substr(0, equal_pos), " ");
                value = rtrim_copy(in_str.substr(equal_pos + 1, in_str.size() - equal_pos), " ");
            } else {
                key = in_str;
            }

            return !key.empty() || !value.empty();
        }
    }

    class param {
    public:
        param& set_short_name(std::string short_name) {
            short_name_ = short_name;
            return *this;
        }

        param& set_long_name(std::string long_name) {
            long_name_ = long_name;
            return *this;
        }

        param& set_default_value(std::string default_value) {
            default_value_ = default_value;
            return *this;
        }

        param& set_description(std::string description) {
            description_ = description;
            return *this;
        }

        param& set_value(std::string value) {
            value_ = value;
            return *this;
        }

        param& set_required() {
            is_required_ = true;
            return *this;
        }

        param& set_flag() {
            is_flag_ = true;
            return *this;
        }

        void set_parsed(bool is_parsed) {
            is_parsed_ = true;
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

        std::string get_value_as_str() const {
            return value_;
        }

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

    class param_parser {
        using param_ptr = std::shared_ptr<cli::param>;
    public:
        param_parser& add_parameter(cli::param parm) {
            const auto ptr{std::make_shared<cli::param>(parm)};
            params_.insert(ptr);
            if (!ptr->short_name().empty())
                params_map_.insert({ptr->short_name(), ptr});
            if (!ptr->long_name().empty())
                params_map_.insert({ptr->long_name(), ptr});
            if (!ptr->default_value().empty() && ptr->value().empty())
                ptr->set_value(ptr->default_value());
            return *this;
        }

        std::optional<std::string> parse(int argc, char* argv[]) {
            std::size_t parm_count = static_cast<std::size_t>(argc - 1);

            if (parm_count < required_args_count())
                return {"Not all required arguments are specified"};

            for (int i = 1; i < argc; ++i) {
                std::string_view parm{argv[i]};
                std::string parm_name, parm_value;

                // check for short parm_name case
                if (parm.size() != 2) {
                    if (!parse_key_arg(argv[i], parm_name, parm_value))
                        return {"Parameter format parse error: " + std::string{argv[i]}};
                } else {
                    parm_name = argv[i];
                }

                const auto it_arg = params_map_.find(parm_name);
                if (it_arg != params_map_.end()) {
                    auto parg{it_arg->second};
                    if (parg->is_flag()) {
                        parg->set_parsed(true);
                        continue;
                    }

                    // The case when the param parm_name is given in a short form 
                    // and requires its parm_value, but the parm_value is not provided
                    if (parm_value.empty()) {
                        if (i == (argc - 1))
                            return {"Expected value for the key: " + parm_name};
                        else {
                            parm_value = argv[i + 1];
                            ++i;
                        }
                    }

                    if (parm_value[0] == '-')
                        return {"Expected value for the key: " + parm_name};

                    parg->set_value(parm_value);
                    parg->set_parsed(true);
                } else {
                    return {"An unknown parameter key is specified: " + std::string{argv[i]}};
                }
            }

            return check_required_args();
        }

        void add_usage_string(std::string usage_string) {
            usage_examples_.push_back(usage_string);
        }

        void print_help() {
            static const std::string tab(4, ' ');

            for (const auto& param : params_)
                adust_fmt_max_field_lengths(param);

            std::cout << "Usage: \n";
            print_usage_examples();

            for (const auto& param : params_) {
                std::cout << tab << param->short_name() << " ";
                if (!param->long_name().empty())
                    std::cout << std::left << " [ " << std::setw(max_long_param_name_length_) << param->long_name() << " ] ";

                if (!param->default_value().empty())
                    std::cout << "(=" << std::setw(max_default_param_value_length_) << param->default_value() << ") ";
                else
                    std::cout << std::string(max_default_param_value_length_ + tab.size(), ' ');

                if (!param->description().empty())
                    std::cout << param->description();

                std::cout << "\n";
            }
        }

        cli::param& arg(const std::string& arg_name) const {
            if (const auto& arg_it = params_map_.find(arg_name); arg_it != params_map_.end())
                return *arg_it->second;

            throw(std::invalid_argument(""));
        }

    private:
        void print_usage_examples() {
            static const std::string tab(4, ' ');
            for (const auto& str : usage_examples_)
                std::cout << tab << str << std::endl;
            std::cout << std::endl;
        }

        void adust_fmt_max_field_lengths(const param_ptr& p) {
            if (p) {
                if (p->long_name().size() > max_long_param_name_length_)
                    max_long_param_name_length_ = p->long_name().size();
                if (p->default_value().size() > max_default_param_value_length_)
                    max_default_param_value_length_ = p->default_value().size();
            }
        }

        std::size_t required_args_count() const {
            return std::count_if(
                std::cbegin(params_),
                std::cend(params_),
                [](const auto& ptr) { return ptr->is_required(); }
            );
        }

        std::optional<std::string> check_required_args() {
            for (const auto& it : params_)
                if (it->is_required() && it->value().empty())
                    return {"Expected required parameter value: " + it->short_name() + " [" + it->long_name() + "]"};

            return {};
        }

        std::set<param_ptr> params_;
        std::unordered_map<std::string, param_ptr> params_map_;
        std::vector<std::string> usage_examples_;

        std::size_t max_long_param_name_length_{};
        std::size_t max_default_param_value_length_{};
    };
}

#endif // cli_parser_h__