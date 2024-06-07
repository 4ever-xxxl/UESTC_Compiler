#include "parser.hh"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <utility>

#include "lexer.hh"

Parser::Parser(const std::vector<Token>& tokens)
    : _flag(true),
      _line(1),
      _idx(0),
      _current_address(0),
      _tokens(std::move(tokens)),
      _cursor(_tokens.begin()) {
  try {
    Program();
  } catch (const std::exception& e) {
  }
}

auto Parser::AddError(const std::string& msg) -> void {
  _flag = false;
  std::cerr << "Error at line " << _line << ", index " << _idx << ": " << msg
            << std::endl;
}

auto Parser::SkipEndOfLine() -> void {
  while (_cursor != _tokens.end() &&
         _cursor->getType() == TokenType::END_OF_LINE) {
    _results.emplace_back(*_cursor);
    _cursor++;
    _idx = 0;
    _line++;
  }
}

auto Parser::Match(const TokenType& type,
                   const std::string& err_message) -> void {
  SkipEndOfLine();
  if (_cursor == _tokens.end()) {
    AddError(err_message.empty() ? "Missing symbol" + TokenTypeToString[type]
                                 : err_message);
    return;
  }
  if (_cursor->getType() != type) {
    AddError(err_message.empty() ? "Expected " + TokenTypeToString[type] +
                                       ", but got " + _cursor->getText()
                                 : err_message);
    return;
  }
  _results.emplace_back(*_cursor);
  _cursor++;
  _idx++;
  SkipEndOfLine();
}

auto Parser::Program() -> void {
  SubProgram();
  Match(TokenType::END_OF_FILE);
}

auto Parser::SubProgram() -> void {
  registerProcedure("main");
  Match(TokenType::BEGIN);
  Declarations();
  Executions();
  Match(TokenType::END);
  _callStack.pop();
}

auto Parser::Declarations() -> void {
  Declaration();
  Declarations_();
}

auto Parser::Declarations_() -> void {
  if (_cursor->getType() == TokenType::INTEGER) {
    Declaration();
    Declarations_();
  }
}

auto Parser::Declaration() -> void {
  Match(TokenType::INTEGER);
  Declaration_();
  Match(TokenType::SEMICOLON);
}

auto Parser::Declaration_() -> void {
  switch (_cursor->getType()) {
    case TokenType::IDENT: {
      VariableDeclaration();
      break;
    }
    case TokenType::FUNCTION: {
      ProcedureDeclaration();
      break;
    }
    default: {
      AddError("Invalid variable name " + _cursor->getText());
      break;
    }
  }
}

auto Parser::VariableDeclaration() -> void {
  Match(TokenType::IDENT);
  registerVariable(_results.back().getText());
}

auto Parser::Variable() -> void {
  Match(TokenType::IDENT);
  if (!findVariable(_results.back().getText())) {
    registerVariable(_results.back().getText());
  }
}

auto Parser::ProcedureDeclaration() -> void {
  Match(TokenType::FUNCTION);
  ProcedureNameDeclaration();
  Match(TokenType::L_PAREN);
  ParameterDeclaration();
  Match(TokenType::R_PAREN, "Unmatched '(', expected ')'");
  Match(TokenType::SEMICOLON);
  ProcedureBody();
}

auto Parser::ProcedureNameDeclaration() -> void {
  Match(TokenType::IDENT);
  registerProcedure(_results.back().getText());
}

auto Parser::ProcedureName() -> void {
  Match(TokenType::IDENT);
  if (!findProcedure(_results.back().getText())) {
    AddError("Undefined procedure '" + _results.back().getText() + "'");
    throw std::runtime_error("Undefined procedure '" +
                             _results.back().getText() + "'");
  }
}

auto Parser::ParameterDeclaration() -> void {
  Match(TokenType::IDENT);
  registerParameter(_results.back().getText());
}

auto Parser::ProcedureBody() -> void {
  Match(TokenType::BEGIN);
  Declarations();
  Executions();
  Match(TokenType::END);
  _callStack.pop();
}

auto Parser::Executions() -> void {
  Execution();
  Executions_();
}

auto Parser::Executions_() -> void {
  if (_cursor->getType() == TokenType::SEMICOLON) {
    Match(TokenType::SEMICOLON);
    Execution();
    Executions_();
  }
}

auto Parser::Execution() -> void {
  switch (_cursor->getType()) {
    case TokenType::READ: {
      Read();
      break;
    }
    case TokenType::WRITE: {
      Write();
      break;
    }
    case TokenType::IDENT: {
      Assign();
      break;
    }
    case TokenType::IF: {
      Condition();
      break;
    }
    default: {
      AddError("Exection cannot begin with " + _cursor->getText());
      throw std::runtime_error("Exection cannot begin with " +
                               _cursor->getText());
    }
  }
}

auto Parser::Read() -> void {
  Match(TokenType::READ);
  Match(TokenType::L_PAREN);
  Variable();
  Match(TokenType::R_PAREN, "Unmatched '(', expected ')'");
}

auto Parser::Write() -> void {
  Match(TokenType::WRITE);
  Match(TokenType::L_PAREN);
  Variable();
  Match(TokenType::R_PAREN, "Unmatched '(', expected ')'");
}

auto Parser::Assign() -> void {
  if (findVariable(_cursor->getText())) {
    Variable();
  } else if (findProcedure(_cursor->getText())) {
    ProcedureName();
  } else {
    AddError("Undefined variable or procedure" + _cursor->getText());
  }
  Match(TokenType::ASSIGN);
  ArithmeticExpression();
}

auto Parser::ArithmeticExpression() -> void {
  Term();
  ArithmeticExpression_();
}

auto Parser::ArithmeticExpression_() -> void {
  if (_cursor->getType() == TokenType::MINUS) {
    Match(TokenType::MINUS);
    Term();
    ArithmeticExpression_();
  }
}

auto Parser::Term() -> void {
  Factor();
  Term_();
}

auto Parser::Term_() -> void {
  if (_cursor->getType() == TokenType::MUL) {
    Match(TokenType::MUL);
    Factor();
    Term_();
  }
}

auto Parser::Factor() -> void {
  switch (_cursor->getType()) {
    case TokenType::NUMBER: {
      Match(TokenType::NUMBER);
      break;
    }
    case TokenType::IDENT: {
      if (findVariable(_cursor->getText())) {
        Variable();
        return;
      }
      if (findProcedure(_cursor->getText())) {
        ProcedureCall();
        return;
      }
      AddError("Undefined variable or procedure " + _cursor->getText());
      throw std::runtime_error("Undefined variable or procedure " +
                               _cursor->getText());
      break;
    }
    default: {
      AddError("Expect variable, procedure or constant, but got " +
               _cursor->getText());
      throw std::runtime_error(
          "Expect variable, procedure or constant, but got " +
          _cursor->getText());
      break;
    }
  }
}

auto Parser::ProcedureCall() -> void {
  ProcedureName();
  Match(TokenType::L_PAREN);
  ArithmeticExpression();
  Match(TokenType::R_PAREN, "Unmatched '(', expected ')'");
}

auto Parser::Condition() -> void {
  Match(TokenType::IF);
  ConditionExpression();
  Match(TokenType::THEN);
  Execution();
  Match(TokenType::ELSE);
  Execution();
}

auto Parser::ConditionExpression() -> void {
  ArithmeticExpression();
  Operator();
  ArithmeticExpression();
}

auto Parser::Operator() -> void {
  switch (_cursor->getType()) {
    case TokenType::EQ: {
      Match(TokenType::EQ);
      break;
    }
    case TokenType::NEQ: {
      Match(TokenType::NEQ);
      break;
    }
    case TokenType::LT: {
      Match(TokenType::LT);
      break;
    }
    case TokenType::LE: {
      Match(TokenType::LE);
      break;
    }
    case TokenType::GT: {
      Match(TokenType::GT);
      break;
    }
    case TokenType::GE: {
      Match(TokenType::GE);
      break;
    }
    default: {
      AddError("'" + _cursor->getText() + "' is not an operator");
      break;
    }
  }
}

auto Parser::registerVariable(const std::string& name) -> void {
  auto parameter = findParameter(name);
  if (parameter) {
    parameter->_is_declared = true;
    return;
  };

  if (findDuplicateVariable(name)) {
    AddError("Parameter '" + name + "' has already been declared");
  }
  auto ptr = std::make_shared<class Variable>(name, _callStack.top(), 0,
                                              Type::INT, _callStack.size(),
                                              ++_current_address, true);
  _variables.emplace_back(ptr);
  updateProcedureVariableAddresses();
}

auto Parser::findDuplicateVariable(const std::string& name) -> bool {
  auto it =
      std::find_if(_variables.begin(), _variables.end(), [&](const auto& v) {
        return v->_name == name && v->_procedure == _callStack.top();
      });
  return it != _variables.end();
}

auto Parser::findVariable(const std::string& name)
    -> std::shared_ptr<class Variable> {
  auto it =
      std::find_if(_variables.begin(), _variables.end(), [&](const auto& v) {
        return v->_name == name && v->_level <= _callStack.size();
      });
  if (it != _variables.end()) {
    if ((*it)->_is_declared) {
      return *it;
    }
    AddError("Variable '" + name + "' has not been declared");
  }
  return nullptr;
}

auto Parser::registerParameter(const std::string& name) -> void {
  if (findDuplicateParameter(name)) {
    AddError("Parameter '" + name + "' has already been declared");
  }
  auto ptr = std::make_shared<class Variable>(name, _callStack.top(), 1,
                                              Type::INT, _callStack.size(),
                                              ++_current_address, false);
  _variables.emplace_back(ptr);
  updateProcedureVariableAddresses();
}

auto Parser::findDuplicateParameter(const std::string& name) -> bool {
  auto it =
      std::find_if(_variables.begin(), _variables.end(), [&](const auto& p) {
        return p->_name == name && p->_kind == 1 &&
               p->_procedure == _callStack.top();
      });
  return it != _variables.end();
}

auto Parser::findParameter(const std::string& name)
    -> std::shared_ptr<class Variable> {
  auto it =
      std::find_if(_variables.begin(), _variables.end(), [&](const auto& p) {
        return p->_name == name && p->_kind == 1 &&
               p->_level <= _callStack.size();
      });
  if (it != _variables.end()) {
    return *it;
  }
  return nullptr;
}

auto Parser::registerProcedure(const std::string& name) -> void {
  if (findDuplicateProcedure(name)) {
    AddError("Procedure '" + name + "' has already been declared");
  }
  auto ptr =
      std::make_shared<class Procedure>(name, Type::INT, _callStack.size());
  _procedures.emplace_back(ptr);
  _callStack.push(ptr);
}

auto Parser::findDuplicateProcedure(const std::string& name) -> bool {
  auto it =
      std::find_if(_procedures.begin(), _procedures.end(), [&](const auto& p) {
        return p->_name == name && p->_level == _callStack.size();
      });
  return it != _procedures.end();
}

auto Parser::findProcedure(const std::string& name)
    -> std::shared_ptr<class Procedure> {
  auto it =
      std::find_if(_procedures.begin(), _procedures.end(), [&](const auto& p) {
        return p->_name == name && p->_level <= _callStack.size();
      });
  if (it != _procedures.end()) {
    return *it;
  }
  return nullptr;
}

auto Parser::updateProcedureVariableAddresses() -> void {
  auto p = _callStack.top();
  if (p->_first_var_address == -1) {
    p->_first_var_address = _current_address;
  }
  p->_last_val_address = _current_address;
}

std::unordered_map<Type, std::string> TypeToString = {
    {Type::VOID, "VOID"}, {Type::INT, "INTEGER"}, {Type::STRING, "STRING"}};

auto Parser::formatPrint(std::ofstream& dysFile, std::ofstream& varFile,
                         std::ofstream& proFile) const -> void {
  for (const auto& node : _results) {
    dysFile << std::setw(16) << node.getText() << "  " << std::setw(2)
            << int(node.getType()) << " " << TokenTypeToString[node.getType()]
            << "\n";
  }

  varFile << "         VarName      ProduceName  Level  Type  Address Kind\n";
  for (const auto& var : _variables) {
    varFile << std::setw(16) << var->_name << " " << std::setw(16)
            << var->_procedure->_name << "  " << std::setw(5) << var->_level
            << " " << std::setw(4) << TypeToString[var->_type] << " "
            << std::setw(5) << var->_address << " " << std::setw(5)
            << var->_kind << "\n";
  }

  proFile
      << "     ProduceName     Type  Level  FirstVarAddress  LastVarAddress\n";
  for (const auto& pro : _procedures) {
    proFile << std::setw(16) << pro->_name << "  " << std::setw(7)
            << TypeToString[pro->_type] << "  " << std::setw(5) << pro->_level
            << "  " << std::setw(15) << pro->_first_var_address << "  "
            << std::setw(14) << pro->_last_val_address << "\n";
  }
}
