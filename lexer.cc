#include "lexer.hh"

#include <cctype>
#include <cstdio>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

std::unordered_map<TokenType, std::string> TokenTypeToString = {
    {TokenType::UNKNOWN, "UNKNOWN"},
    {TokenType::BEGIN, "BEGIN"},
    {TokenType::END, "END"},
    {TokenType::INTEGER, "INTEGER"},
    {TokenType::IF, "IF"},
    {TokenType::THEN, "THEN"},
    {TokenType::ELSE, "ELSE"},
    {TokenType::FUNCTION, "FUNCTION"},
    {TokenType::READ, "READ"},
    {TokenType::WRITE, "WRITE"},
    {TokenType::IDENT, "IDENT"},
    {TokenType::NUMBER, "NUMBER"},
    {TokenType::EQ, "EQ"},
    {TokenType::NEQ, "NEQ"},
    {TokenType::LE, "LE"},
    {TokenType::LT, "LT"},
    {TokenType::GE, "GE"},
    {TokenType::GT, "GT"},
    {TokenType::MINUS, "MINUS"},
    {TokenType::MUL, "MUL"},
    {TokenType::ASSIGN, "ASSIGN"},
    {TokenType::L_PAREN, "L_PAREN"},
    {TokenType::R_PAREN, "R_PAREN"},
    {TokenType::SEMICOLON, "SEMICOLON"},
    {TokenType::END_OF_LINE, "END_OF_LINE"},
    {TokenType::END_OF_FILE, "END_OF_FILE"}};

Lexer::Lexer(const std::vector<std::string>& words) {
  _line = 1;
  _flag = true;
  for (const auto& word : words) {
    int cursor = 0;
    int word_size = word.size();
    while (cursor < word_size) {
      if (std::isalpha(word[cursor])) {
        int right_bound = cursor + 1;
        while (right_bound < word_size && std::isalnum(word[right_bound])) {
          right_bound++;
        }
        std::string identifier = word.substr(cursor, right_bound - cursor);
        const auto it = _table.find(identifier);  // 保留字匹配
        if (it != _table.end()) {
          _tokens.emplace_back(it->second, it->first);
        } else {
          _tokens.emplace_back(TokenType::IDENT, std::move(identifier));
          if (right_bound - cursor > 16) {
            _flag = false;
            std::cerr << "Line: " << _line << ", Ident out of length: '"
                      << identifier << "'\n";
          }
        }
        cursor = right_bound;
      } else if (std::isdigit(word[cursor])) {
        int right_bound = cursor + 1;
        while (right_bound < word_size && std::isdigit(word[right_bound])) {
          right_bound++;
        }
        std::string identifier = word.substr(cursor, right_bound - cursor);
        _tokens.emplace_back(TokenType::NUMBER, std::move(identifier));
        cursor = right_bound;
      } else {
        switch (word[cursor]) {
          case '\n': {
            _line++;
            _tokens.emplace_back(TokenType::END_OF_LINE, "EOLN");
            break;
          }
          case '=': {
            _tokens.emplace_back(TokenType::EQ, "=");
            break;
          }
          case '-': {
            _tokens.emplace_back(TokenType::MINUS, "-");
            break;
          }
          case '*': {
            _tokens.emplace_back(TokenType::MUL, "*");
            break;
          }
          case '(': {
            _tokens.emplace_back(TokenType::L_PAREN, "(");
            break;
          }
          case ')': {
            _tokens.emplace_back(TokenType::R_PAREN, ")");
            break;
          }
          case '<': {
            int right_bound = cursor + 1;
            if (right_bound < word_size) {
              if (word[right_bound] == '=') {
                _tokens.emplace_back(TokenType::LE, word.substr(cursor, 2));
                cursor++;
                break;
              }
              if (word[right_bound] == '>') {
                _tokens.emplace_back(TokenType::NEQ, word.substr(cursor, 2));
                cursor++;
                break;
              }
            }
            _tokens.emplace_back(TokenType::LT, std::string(1, word[cursor]));
            break;
          }
          case '>': {
            int right_bound = cursor + 1;
            if (right_bound < word_size && word[right_bound] == '=') {
              _tokens.emplace_back(TokenType::GE, word.substr(cursor, 2));
              cursor++;
              break;
            }
            _tokens.emplace_back(TokenType::GT, std::string(1, word[cursor]));
            break;
          }
          case ':': {
            int right_bound = cursor + 1;
            if (right_bound < word_size && word[right_bound] == '=') {
              _tokens.emplace_back(TokenType::ASSIGN, word.substr(cursor, 2));
              cursor++;
            } else {
              _flag = false;
              std::cerr << "Line: " << _line << ", Expected '=' after ':'\n";
              _tokens.emplace_back(TokenType::UNKNOWN,
                                   std::string(1, word[cursor]));
            }
            break;
          }
          case ';': {
            _tokens.emplace_back(TokenType::SEMICOLON, ";");
            break;
          }
          default: {
            _flag = false;
            _tokens.emplace_back(TokenType::UNKNOWN,
                                 std::string(1, word[cursor]));
            std::cerr << "Line: " << _line << ", Invalid character: '"
                      << word[cursor] << "'\n";
            break;
          }
        }
        cursor++;
      }
    }
  }
  _tokens.emplace_back(TokenType::END_OF_FILE, "EOF");
}

auto Lexer::formatPrint(std::ofstream& outputFile) const -> void {
  for (const auto& node : _tokens) {
    outputFile << std::setw(16) << node.getText() << "  " << std::setw(2)
               << int(node.getType()) << " "
               << TokenTypeToString[node.getType()] << "\n";
  }
}