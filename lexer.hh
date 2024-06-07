#pragma once
#include <cstdio>
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

enum class TokenType {
  UNKNOWN,
  BEGIN,
  END,
  INTEGER,
  IF,
  THEN,
  ELSE,
  FUNCTION,
  READ,
  WRITE,
  IDENT,
  NUMBER,
  EQ,
  NEQ,
  LE,
  LT,
  GE,
  GT,
  MINUS,
  MUL,
  ASSIGN,
  L_PAREN,
  R_PAREN,
  SEMICOLON,
  END_OF_LINE,
  END_OF_FILE
};

extern std::unordered_map<TokenType, std::string> TokenTypeToString;

class Token {
 public:
  Token(const TokenType& token_type, const std::string& text)
      : _type(token_type), _text(std::move(text)) {}
  auto getType() const -> const TokenType& { return _type; }
  auto getText() const -> const std::string& { return _text; }

 private:
  TokenType _type;
  std::string _text;
};

class Lexer {
 public:
  Lexer(const std::vector<std::string>& words);
  auto good() const -> const bool { return _flag; }
  auto formatPrint(std::ofstream& outputFile) const -> void;
  auto getTokens() const -> const std::vector<Token>& { return _tokens; }

 private:
  int _line;
  bool _flag;
  std::vector<Token> _tokens;
  /* 用来匹配需要完全匹配的保留字 */
  std::unordered_map<std::string, TokenType> _table = {
      {"begin", TokenType::BEGIN},       {"end", TokenType::END},
      {"integer", TokenType::INTEGER},   {"if", TokenType::IF},
      {"then", TokenType::THEN},         {"else", TokenType::ELSE},
      {"function", TokenType::FUNCTION}, {"read", TokenType::READ},
      {"write", TokenType::WRITE}};
};
