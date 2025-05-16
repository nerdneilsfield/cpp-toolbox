#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>

#include <cpp-toolbox/cpp-toolbox_export.hpp>

namespace toolbox::utils
{

class CPP_TOOLBOX_EXPORT parameter_t
{
protected:
  std::string name_;
  std::string description_;
  bool required_;
  bool is_set_;

public:
  parameter_t(const std::string& name,
              const std::string& description,
              bool required = false);
  virtual ~parameter_t() = default;

  [[nodiscard]] std::string get_name() const;
  [[nodiscard]] std::string get_description() const;
  [[nodiscard]] bool is_required() const;
  [[nodiscard]] bool is_set() const;

  virtual bool parse(const std::string& value) = 0;
  virtual bool accepts_missing_value() const;
  virtual bool is_option() const { return false; }
  virtual bool is_argument() const { return false; }
  virtual bool is_flag() const { return false; }
  virtual std::string get_short_name() const;
};

template<typename T>
class CPP_TOOLBOX_EXPORT option_t : public parameter_t
{
private:
  T value_;
  T default_value_;
  std::string short_name_;
  std::function<bool(const std::string&, T&)> parser_;

public:
  option_t(const std::string& name,
           const std::string& short_name,
           const std::string& description,
           bool required = false);

  option_t& set_default(const T& default_value);
  option_t& set_parser(std::function<bool(const std::string&, T&)> parser);
  [[nodiscard]] std::string get_short_name() const override;
  T get() const;
  bool parse(const std::string& value) override;
  bool accepts_missing_value() const override;
  bool is_option() const override;

private:
  void set_default_parser();  // Type-specific default parsers
};

class CPP_TOOLBOX_EXPORT flag_t : public option_t<bool>
{
public:
  flag_t(const std::string& name,
         const std::string& short_name,
         const std::string& description);

  bool parse(const std::string& value) override;
  bool is_option() const override { return true; }
  bool is_flag() const override { return true; }
  std::string get_short_name() const override;
  bool get() const;
};

template<typename T>
class CPP_TOOLBOX_EXPORT argument_t : public parameter_t
{
private:
  T value_;
  T default_value_;
  std::function<bool(const std::string&, T&)> parser_;

public:
  argument_t(const std::string& name,
             const std::string& description,
             bool required = true);

  argument_t& set_default(const T& default_value);
  T get() const;
  bool parse(const std::string& value) override;
  bool accepts_missing_value() const override;
  bool is_argument() const override;
  std::string get_short_name() const override;

private:
  void set_default_parser();
};

class CPP_TOOLBOX_EXPORT command_t
{
private:
  std::string name_;
  std::string description_;
  std::vector<std::unique_ptr<parameter_t>> parameters_;
  std::vector<std::unique_ptr<command_t>> subcommands_;
  std::function<int()> callback_;
  bool help_flag_added_ = false;
  bool help_requested_ = false;

public:
  command_t(const std::string& name, const std::string& description);

  // 显式删除拷贝操作，允许移动操作
  command_t(const command_t&) = delete;
  command_t& operator=(const command_t&) = delete;
  command_t(command_t&&) = default;
  command_t& operator=(command_t&&) = default;

  std::string get_name() const;
  std::string get_description() const;

  template<typename T>
  option_t<T>& add_option(const std::string& name,
                          const std::string& short_name,
                          const std::string& description,
                          bool required = false);

  flag_t& add_flag(const std::string& name,
                   const std::string& short_name,
                   const std::string& description);

  template<typename T>
  argument_t<T>& add_argument(const std::string& name,
                              const std::string& description,
                              bool required = true);

  command_t& add_command(const std::string& name,
                         const std::string& description);
  command_t& set_callback(std::function<int()> callback);

  int parse_and_execute(const std::vector<std::string>& args);
  std::string format_help() const;

private:
  void add_help();
  [[nodiscard]] bool handle_help_flag() const;
  [[nodiscard]] std::string format_command_list() const;
};

class CPP_TOOLBOX_EXPORT CommandLineApp : public command_t
{
public:
  CommandLineApp(const std::string& name, const std::string& description);

  int run(int argc, char** argv);

private:
  void handle_exceptions(const std::exception& e) const;
};

class CPP_TOOLBOX_EXPORT ClickException : public std::runtime_error
{
public:
  ClickException(const std::string& message);
  virtual void print() const;
};

class CPP_TOOLBOX_EXPORT ParameterError : public ClickException
{
public:
  ParameterError(const std::string& message);
};

class CPP_TOOLBOX_EXPORT UsageError : public ClickException
{
public:
  UsageError(const std::string& message);
};

}  // namespace toolbox::utils

#include <cpp-toolbox/utils/impl/click_impl.hpp>
