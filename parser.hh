#pragma once
#include <cstddef>
#include <memory>
#include <stack>
#include <string>
#include <vector>

#include "lexer.hh"

enum class Type { VOID, INT, STRING };

extern std::unordered_map<Type, std::string> TypeToString;

class Variable {
 public:
  std::string _name;
  std::shared_ptr<class Procedure> _procedure;
  bool _kind;  // 0 for var 1 for param
  Type _type;
  size_t _level;
  int _address;
  bool _is_declared;

  Variable(const std::string& name, const std::shared_ptr<Procedure>& procedure,
           bool kind, Type type, size_t level, int address, bool is_declared)
      : _name(name),
        _procedure(procedure),
        _kind(kind),
        _type(type),
        _level(level),
        _address(address),
        _is_declared(is_declared) {}
};

class Procedure {
 public:
  std::string _name;
  Type _type;
  size_t _level;
  int _first_var_address;
  int _last_val_address;

  Procedure(const std::string& name, const Type type, const size_t level)
      : _name(std::move(name)), _type(type), _level(level) {
    _first_var_address = -1;
    _last_val_address = -1;
  }
};

class Parser {
 public:
  Parser(const std::vector<Token>& tokens);
  auto formatPrint(std::ofstream& dysFile, std::ofstream& varFile, std::ofstream& proFile) const -> void;
  auto good() const -> const bool { return _flag; }

 private:
  bool _flag;
  int _line;
  int _idx;
  int _current_address;
  std::vector<Token> _tokens;
  std::vector<Token>::iterator _cursor;
  std::vector<Token> _results;
  std::vector<std::shared_ptr<Variable>> _variables;
  std::vector<std::shared_ptr<Procedure>> _procedures;
  std::stack<std::shared_ptr<Procedure>> _callStack;

  auto AddError(const std::string& msg)->void;
  auto SkipEndOfLine() -> void;
  auto Match(const TokenType& type,
             const std::string& err_message = "") -> void;
  auto Program() -> void;
  auto SubProgram() -> void;
  auto Declarations() -> void;
  auto Declarations_() -> void;
  auto Declaration() -> void;
  auto Declaration_() -> void;
  auto VariableDeclaration() -> void;
  auto Variable() -> void;
  auto ProcedureDeclaration() -> void;
  auto ProcedureNameDeclaration() -> void;
  auto ProcedureName() -> void;
  auto ParameterDeclaration() -> void;
  auto ProcedureBody() -> void;
  auto Executions() -> void;
  auto Executions_() -> void;
  auto Execution() -> void;
  auto Read() -> void;
  auto Write() -> void;
  auto Assign() -> void;
  auto ArithmeticExpression() -> void;
  auto ArithmeticExpression_() -> void;
  auto Term() -> void;
  auto Term_() -> void;
  auto Factor() -> void;
  auto ProcedureCall() -> void;
  auto Condition() -> void;
  auto ConditionExpression() -> void;
  auto Operator() -> void;

  auto registerVariable(const std::string& name) -> void;
  auto findDuplicateVariable(const std::string& name) -> bool;
  auto findVariable(const std::string& name) -> std::shared_ptr<class Variable>;

  auto registerParameter(const std::string& name) -> void;
  auto findDuplicateParameter(const std::string& name) -> bool;
  auto findParameter(const std::string& name)
      -> std::shared_ptr<class Variable>;

  auto registerProcedure(const std::string& name) -> void;
  auto findDuplicateProcedure(const std::string& name) -> bool;
  auto findProcedure(const std::string& name)
      -> std::shared_ptr<class Procedure>;

  auto updateProcedureVariableAddresses() -> void;
};