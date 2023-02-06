#ifndef cli_parser_h__

#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <algorithm>
#include <optional>
#include <iomanip>
#include <vector>
#include <functional>
#include <exception>
#include <sstream>
#include <memory>
#include <utility>

namespace cli {
    namespace {
        std::string_view rtrim_copy(std::string_view str, std::string_view pattern) {
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

        std::vector<std::string> strings_from_raw_args(int argc, char* argv[]) {
            if (argc < 1 || argv == nullptr)
                return {};

            std::vector<std::string> args;
            args.reserve(static_cast<std::size_t>(argc));

            for (int i = 0; i < argc; ++i) {
                if (argv[i] == nullptr)
                    return {};

                args.push_back(argv[i]);
            }

            return args;
        }
    }

    class param {
    public:
        param& short_name(std::string short_name) {
            short_name_ = short_name;
            return *this;
        }

        param& long_name(std::string long_name) {
            long_name_ = long_name;
            return *this;
        }

        param& default_value(std::string default_value) {
            default_value_ = default_value;
            return *this;
        }

        param& description(std::string description) {
            description_ = description;
            return *this;
        }

        param& value(std::string value) {
            value_ = value;
            return *this;
        }

        param& required() {
            is_required_ = true;
            return *this;
        }

        param& flag() {
            is_flag_ = true;
            return *this;
        }

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

    class param_parser {
        using param_ptr = std::shared_ptr<cli::param>;
    public:
        param_parser& add_parameter(cli::param parm) {
            const auto ptr{std::make_shared<cli::param>(parm)};

            if (!ptr->short_name().empty()) {
                if (auto p = params_map_.find(ptr->short_name()); p != params_map_.end()) {
                    if (ptr->long_name() != p->second->long_name()) {
                        params_map_.erase(p->second->long_name());
                        p->second = ptr;
                    }
                } else {
                    params_map_.insert({ptr->short_name(), ptr});
                }
            }

            if (!ptr->long_name().empty()) {
                if (const auto p = params_map_.find(ptr->long_name()); p != params_map_.end()) {
                    if (ptr->short_name() != p->second->short_name()) {
                        params_map_.erase(p->second->short_name());
                        p->second = ptr;
                    }
                } else {
                    params_map_.insert({ptr->long_name(), ptr});
                }
            }

            if (!ptr->default_value().empty() && ptr->value().empty())
                ptr->value(ptr->default_value());

            return *this;
        }

        std::optional<std::string> parse(const std::vector<std::string>& args) {
            std::size_t parm_count = args.size() - 1;

            if (parm_count < required_args_count())
                return {"Not all required arguments are specified"};

            for (std::size_t i = 1; i < args.size(); ++i) {
                const std::string& parm{args[i]};
                std::string parm_name, parm_value;

                // check for short parm_name case
                if (parm.size() != 2) {
                    if (!parse_key_arg(parm, parm_name, parm_value))
                        return {"Parameter format parse error: " + std::string{parm}};
                } else {
                    parm_name = parm;
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
                        if (i == parm_count)
                            return {"Expected value for the key: " + parm_name};
                        else {
                            parm_value = args[i + 1];
                            ++i;
                        }
                    }

                    if (parm_value[0] == '-')
                        return {"Expected value for the key: " + parm_name};

                    parg->value(parm_value);
                    parg->set_parsed(true);
                } else {
                    return {"An unknown parameter key is specified: " + parm};
                }
            }

            return check_required_args();
        }

        std::optional<std::string> parse(int argc, char* argv[]) {
            return parse(strings_from_raw_args(argc, argv));
        }

        void add_usage_string(std::string usage_string) {
            usage_examples_.push_back(usage_string);
        }

        void print_help() {
            static const std::string tab(4, ' ');

            const auto params = all_params();

            for (const auto& parm : params)
                adust_fmt_max_field_lengths(parm);

            std::cout << "Usage: \n";
            print_usage_examples();

            for (const auto& parm: params) {
                std::cout << tab << parm->short_name() << " ";
                if (!parm->long_name().empty())
                    std::cout << std::left << " [ " << std::setw(static_cast<int>(max_long_param_name_length_)) << parm->long_name() << " ] ";

                if (!parm->default_value().empty())
                    std::cout << "(=" << std::setw(static_cast<int>(max_default_param_value_length_)) << parm->default_value() << ") ";
                else
                    std::cout << std::string(max_default_param_value_length_ + tab.size(), ' ');

                if (!parm->description().empty())
                    std::cout << parm->description();

                std::cout << "\n";
            }
        }

        cli::param& arg(const std::string& arg_name) const {
            if (const auto& arg_it = params_map_.find(arg_name); arg_it != params_map_.end())
                return *arg_it->second;

            throw(std::invalid_argument(""));
        }

        std::size_t parameters_count() const { return all_params().size(); }

        void reset() {
            params_map_.clear();
            usage_examples_.clear();

            max_long_param_name_length_ = 0;
            max_default_param_value_length_ = 0;
        }

        const std::vector<param_ptr> all_params() const {
            std::vector<param_ptr> params;
            for (const auto& [key, value] : params_map_) {
                if (value->long_name().empty())
                    params.push_back(value);
                else {
                    const auto& p = params_map_.find(value->long_name());
                    if (p != params_map_.end())
                        params.push_back(value);
                }
            }

            std::sort(std::begin(params), std::end(params));
            const auto last = std::unique(std::begin(params), std::end(params));
            params.erase(last, params.end());

            return params;
        }

    private:
        void print_usage_examples() const {
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
            const auto params = all_params();
            return static_cast<std::size_t>(std::count_if(
                std::cbegin(params),
                std::cend(params),
                [](const auto& ptr) mutable { return ptr->is_required(); }
            ));
        }

        std::optional<std::string> check_required_args() const {
            const auto params = all_params();
            for (const auto& parm: params)
                if (parm->is_required() && parm->value().empty())
                    return {"Expected required parameter value: " + parm->short_name() + " [" + parm->long_name() + "]"};

            return {};
        }

        std::unordered_map<std::string, param_ptr> params_map_;
        std::vector<std::string> usage_examples_;

        std::size_t max_long_param_name_length_{};
        std::size_t max_default_param_value_length_{};
    };
}

#endif // cli_parser_h__