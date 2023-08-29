#include "cli_parser.h"

#include <sstream>
#include <memory>
#include <utility>
#include <iomanip>

namespace cli
{
    namespace {
        std::string_view rtrim_copy(std::string_view str, std::string_view pattern) {
            if (const auto pos = str.rfind(pattern.data()); pos != std::string_view::npos)
                return std::string_view{str.data(), pos};

            return str;
        }

        std::vector<std::string> split(const std::string& str, std::string_view delimeter)
        {
            std::string::size_type cur_pos{};
            std::string::size_type next_pos{};

            std::vector<std::string> items;

            while ((next_pos = str.find_first_of(delimeter, cur_pos)) != std::string::npos) {
                if ((next_pos - cur_pos) > 0)
                    items.emplace_back(std::string{str.data() + cur_pos, next_pos - cur_pos});
                cur_pos = next_pos + 1;
            }

            if (cur_pos < str.size())
                items.emplace_back(std::string{str.data() + cur_pos, str.size() - cur_pos});

            return items;
        }

        void ltrim(std::string& s, char sym = ' ')
        {
            s.erase(s.begin(), std::find_if(s.begin(), s.end(), [sym](int ch)
            {
                return !(ch == sym);
            }));
        }

        void rtrim(std::string& s, char sym = ' ')
        {
            s.erase(std::find_if(s.rbegin(), s.rend(), [sym](int ch)
            {
                return !(ch == sym);
            }).base(), s.end());
        }

        void trim(std::string& s, char sym = ' ')
        {
            ltrim(s, sym);
            rtrim(s, sym);
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

                args.emplace_back(std::string{argv[i]});
            }

            return args;
        }
    }

    Param& Param::required()
    {
        is_required_ = true;
        return *this;
    }

    Param& Param::flag()
    {
        is_flag_ = true;
        return *this;
    }

    Param::Param(std::string name)
    {
        auto names = split(name, ",");

        for (auto& it : names)
        {
            trim(it);
            ltrim(it, '-');
        }

        if (names.empty())
            throw std::runtime_error("Command line parameter must have name");

        if (names.size() > 1 && names[0].size() == 1 && names[1].size() == 1)
            throw std::runtime_error("Command line parameter must have only one short name");

        if (names.size() > 1 && names[0].size() > 1 && names[1].size() > 1)
            throw std::runtime_error("Command line parameter must have only one long name");

        if (names.size() == 1)
        {
            if (names[0].size() > 1)
                long_name_ = names[0];
            else
                short_name_ = names[0];
        }

        if (names.size() > 1)
        {
            short_name_ = names[0];
            long_name_ = names[1];

            if (names[0].size() > names[1].size())
                std::swap(short_name_, long_name_);
        }
    }

    Param& Param::short_name(std::string short_name)
    {
        short_name_ = short_name;
        ltrim(short_name_, '-');
        return *this;
    }

    Param& Param::long_name(std::string long_name)
    {
        long_name_ = long_name;
        ltrim(long_name_, '-');
        return *this;
    }

    Param& Param::default_value(std::string default_value)
    {
        default_value_ = default_value;
        return *this;
    }

    Param& Param::description(std::string description)
    {
        description_ = description;
        return *this;
    }

    Param& Param::value(std::string value)
    {
        value_ = value;
        return *this;
    }

    ParamParser& ParamParser::add_parameter(Param parm)
    {
        const auto ptr{std::make_shared<Param>(parm)};

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

    std::optional<std::string> ParamParser::parse(int argc, char* argv[])
    {
        return parse(strings_from_raw_args(argc, argv));
    }

    std::optional<std::string> ParamParser::parse(const std::vector<std::string>& args)
    {
        std::size_t parm_count = args.size() - 1;

        if (parm_count < required_args_count())
            return {"Not all required arguments are specified"};

        for (std::size_t i = 1; i < args.size(); ++i) {
            std::string parm{args[i]};
            ltrim(parm, '-');
            std::string parm_name, parm_value;

            // check for short parm_name case
            if (parm.size() != 1) {
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

                // The case when the Param parm_name is given in a short form 
                // and requires its parm_value, but the parm_value is not provided
                if (parm_value.empty()) {
                    if (i == parm_count)
                        return {"Expected value for the key: " + parm_name};

                    parm_value = args[i + 1];
                    ++i;
                }

                parg->value(parm_value);
                parg->set_parsed(true);
            } else {
                return {"An unknown parameter key is specified: " + parm};
            }
        }

        return check_required_args();
    }

    void ParamParser::add_usage_string(std::string usage_string)
    {
        usage_examples_.push_back(usage_string);
    }

    void ParamParser::print_help()
    {
        static const std::string tab(4, ' ');

        const auto params = all_params();

        for (const auto& parm : params)
            adust_fmt_max_field_lengths(parm);

        std::cout << "Usage: \n";
        print_usage_examples();

        for (const auto& parm : params) {
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

    const Param& ParamParser::arg(const std::string& arg_name) const
    {
        if (const auto& arg_it = params_map_.find(arg_name); arg_it != params_map_.end())
            return *arg_it->second;

        return empty_arg_;
    }

    void ParamParser::reset()
    {
        params_map_.clear();
        usage_examples_.clear();

        max_long_param_name_length_ = 0;
        max_default_param_value_length_ = 0;
    }

    const std::vector<ParamParser::ParamPtr> ParamParser::all_params() const
    {
        std::vector<ParamPtr> params;
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

    void ParamParser::print_usage_examples() const
    {
        static const std::string tab(4, ' ');
        for (const auto& str : usage_examples_)
            std::cout << tab << str << std::endl;
        std::cout << std::endl;
    }

    void ParamParser::adust_fmt_max_field_lengths(const ParamPtr& p)
    {
        if (p) {
            if (p->long_name().size() > max_long_param_name_length_)
                max_long_param_name_length_ = p->long_name().size();
            if (p->default_value().size() > max_default_param_value_length_)
                max_default_param_value_length_ = p->default_value().size();
        }
    }

    std::optional<std::string> ParamParser::check_required_args() const
    {
        const auto params = all_params();
        for (const auto& parm : params)
            if (parm->is_required() && parm->value().empty())
                return {"Expected required parameter value: " + parm->short_name() + " [" + parm->long_name() + "]"};

        return {};
    }
}
